#!/usr/bin/env python3
"""End-to-end regression tests for corrected memory-model integration."""

import json
import math
import pathlib
import re
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests" / "fixtures"
NUMBER = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


def quantity(text, label, scales):
    units = "|".join(re.escape(unit) for unit in sorted(scales, key=len, reverse=True))
    match = re.search(rf"{re.escape(label)}\s*=\s*({NUMBER})({units})", text)
    if not match:
        raise AssertionError(f"missing quantity {label!r}\n{text}")
    return float(match.group(1)) * scales[match.group(2)]


TIME_SCALES = {"ps": 1e-12, "ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}
ENERGY_SCALES = {"pJ": 1e-12, "nJ": 1e-9, "uJ": 1e-6, "mJ": 1e-3, "J": 1.0}
AREA_SCALES = {"nm^2": 1e-18, "um^2": 1e-12, "mm^2": 1e-6, "m^2": 1.0}
POWER_SCALES = {"pW": 1e-12, "nW": 1e-9, "uW": 1e-6, "mW": 1e-3, "W": 1.0}


def operating_point(text, name):
    match = re.search(
        rf"{re.escape(name)} AOS operating point: "
        rf"Ion=({NUMBER})A, Ioff=({NUMBER})A, Ron=({NUMBER})ohm, "
        rf"Roff=({NUMBER})ohm, Cgate=({NUMBER})F, Cdrain=({NUMBER})F",
        text,
    )
    if not match:
        raise AssertionError(f"missing {name} AOS operating point\n{text}")
    names = ("Ion", "Ioff", "Ron", "Roff", "Cgate", "Cdrain")
    return dict(zip(names, map(float, match.groups())))


def gem5_option(text, name):
    match = re.search(rf"--{re.escape(name)}\s+(\d+)", text)
    if not match:
        raise AssertionError(f"missing gem5 option --{name}\n{text}")
    return int(match.group(1))


class IntegratedModelTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._temporary_directory = tempfile.TemporaryDirectory(prefix="nsc-model-tests-")
        cls.temp_dir = pathlib.Path(cls._temporary_directory.name)
        cls.golden = json.loads(
            (FIXTURES / "model_integration_golden.json").read_text(encoding="utf-8")
        )
        subprocess.run(
            ["make", "-C", str(ROOT / "src"), "-j2"],
            check=True,
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        objects = sorted((ROOT / "src" / "obj").glob("*.o"))
        objects = [item for item in objects if item.name != "main.o"]
        cls.copy_probe = cls.temp_dir / "mat_result_copy_probe"
        subprocess.run(
            [
                "c++",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                f"-I{ROOT / 'src'}",
                str(ROOT / "tests" / "cpp" / "mat_result_copy_probe.cpp"),
                *(str(item) for item in objects),
                "-o",
                str(cls.copy_probe),
            ],
            check=True,
            cwd=ROOT,
        )

    @classmethod
    def tearDownClass(cls):
        cls._temporary_directory.cleanup()

    def run_fixture(self, name, extra="", expect_success=True):
        source = FIXTURES / name
        if extra:
            config = self.temp_dir / f"{source.stem}-{len(list(self.temp_dir.glob('*.cfg')))}.cfg"
            config.write_text(source.read_text(encoding="utf-8") + "\n" + extra, encoding="utf-8")
        else:
            config = source
        result = subprocess.run(
            [str(ROOT / "nsc"), str(config)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=30,
        )
        if expect_success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("numSolutions = 1 / numDesigns = 1", result.stdout)
        return result

    def run_config_text(self, contents, stem="generated", expect_success=True):
        config = self.temp_dir / f"{stem}-{len(list(self.temp_dir.glob('*.cfg')))}.cfg"
        config.write_text(contents, encoding="utf-8")
        result = subprocess.run(
            [str(ROOT / "nsc"), str(config)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=30,
        )
        if expect_success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("numSolutions = 1 / numDesigns = 1", result.stdout)
        return result

    def assert_sram_wins_multi_cell_bandwidth(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("[Info] Cell exploration failures: 0 / 2", result.stdout)
        self.assertIn("Optimized for: Read Bandwidth", result.stdout)
        self.assertIn("Memory Cell: SRAM", result.stdout)
        bandwidth = quantity(result.stdout, "Read Bandwidth", {"GB/s": 1e9})
        self.assertGreater(bandwidth, 40e9)

    def write_cell_variant(self, fixture, replacement, stem):
        contents = (FIXTURES / fixture).read_text(encoding="utf-8")
        old, new = replacement
        self.assertIn(old, contents)
        path = self.temp_dir / f"{stem}.cell"
        path.write_text(contents.replace(old, new, 1), encoding="utf-8")
        return path

    def assert_relative_equal(self, first, second, tolerance=3e-6):
        self.assertLessEqual(abs(first - second), max(abs(first), abs(second), 1e-30) * tolerance)

    def test_precharger_technology_matrix_and_mlc_rejection(self):
        cases = {
            "fixed_sram_512x128.cfg": "Memory Cell: SRAM",
            "fixed_dram_512x128.cfg": "Memory Cell: DRAM",
            "fixed_edram_512x128.cfg": "Memory Cell: Embedded DRAM",
            "fixed_gcdram_512x128.cfg": "Memory Cell: Gain Cell DRAM",
            "fixed_pcram_256x256.cfg": "Memory Cell: PCRAM (Phase-Change)",
            "fixed_sttram_256x256.cfg": "Memory Cell: MRAM (Magnetoresistive)",
            "fixed_slc_nand.cfg": "Memory Cell: Single-Level Cell NAND Flash",
        }
        delays = {}
        for fixture, cell_label in cases.items():
            with self.subTest(fixture=fixture):
                output = self.run_fixture(fixture).stdout
                self.assertIn(cell_label, output)
                delay = quantity(output, "Precharge Latency", TIME_SCALES)
                self.assertTrue(math.isfinite(delay))
                self.assertGreater(delay, 0)
                delays[fixture] = delay

        golden = self.golden["fixed_sram_512x128"]
        self.assertAlmostEqual(
            delays["fixed_sram_512x128.cfg"] / 1e-12,
            golden["precharge_delay_ps"],
            delta=golden["absolute_tolerance_ps"],
        )

        rejected = self.run_fixture("rejected_mlc_nand.cfg", expect_success=False)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn("[ERROR] MLC NAND flash model is still under development", rejected.stdout)

    def test_multi_cell_bandwidth_comparison_uses_each_result_cell(self):
        forward = self.run_fixture("multi_cell_read_bandwidth.cfg")
        self.assert_sram_wins_multi_cell_bandwidth(forward)

        contents = (FIXTURES / "multi_cell_read_bandwidth.cfg").read_text(encoding="utf-8")
        forward_order = (
            "-MemoryCellInputFile: config/New_Configs/SRAM_cell_14nm.cell\n"
            "-MemoryCellInputFile: config/Old_Configs/sample_2D_eDRAM.cell"
        )
        reverse_order = (
            "-MemoryCellInputFile: config/Old_Configs/sample_2D_eDRAM.cell\n"
            "-MemoryCellInputFile: config/New_Configs/SRAM_cell_14nm.cell"
        )
        self.assertIn(forward_order, contents)
        reverse = self.run_config_text(
            contents.replace(forward_order, reverse_order, 1),
            "multi-cell-read-bandwidth-reversed",
        )
        self.assert_sram_wins_multi_cell_bandwidth(reverse)
        self.assertAlmostEqual(
            quantity(forward.stdout, "Read Bandwidth", {"GB/s": 1e9}),
            quantity(reverse.stdout, "Read Bandwidth", {"GB/s": 1e9}),
            delta=1e6,
        )

    def test_all_cell_files_are_validated_before_exploration(self):
        malformed_cell = self.temp_dir / "incomplete-second-aos.cell"
        malformed_cell.write_text(
            (FIXTURES / "synthetic_aos_edram.cell")
            .read_text(encoding="utf-8")
            .replace("-OxideAccessTransistorLeakageScale: 1.0\n", "", 1),
            encoding="utf-8",
        )
        config = (FIXTURES / "fixed_sram_512x128.cfg").read_text(encoding="utf-8")
        first_cell = "-MemoryCellInputFile: config/New_Configs/SRAM_cell_14nm.cell"
        self.assertIn(first_cell, config)
        config = config.replace(
            first_cell,
            first_cell + f"\n-MemoryCellInputFile: {malformed_cell}",
            1,
        )

        result = self.run_config_text(config, "multi-cell-preflight", expect_success=False)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("AOS transistor is incomplete", result.stderr + result.stdout)
        self.assertNotIn("Using cell file:", result.stdout)
        self.assertNotIn("numDesigns", result.stdout)

    def test_dram_restore_cycle_and_residual_ratio_monotonicity(self):
        standalone = self.run_fixture("fixed_dram_512x128.cfg").stdout
        standalone_access = quantity(
            standalone, "DRAM Access (Time-to-Data)", TIME_SCALES
        )
        standalone_restore = quantity(standalone, "DRAM Restore Delay", TIME_SCALES)
        standalone_cycle = quantity(standalone, "DRAM Full Read Cycle", TIME_SCALES)
        self.assertAlmostEqual(
            standalone_cycle, standalone_access + standalone_restore, delta=2e-12
        )
        self.assertGreater(quantity(standalone, "Refresh Latency", TIME_SCALES), 0)

        samples = []
        for residual in (0.20, 0.10, 0.05):
            output = self.run_fixture(
                "fixed_edram_512x128.cfg",
                f"-DRAMTargetResidualRatio: {residual}",
            ).stdout
            access = quantity(output, "DRAM Access (Time-to-Data)", TIME_SCALES)
            restore = quantity(output, "DRAM Restore Delay", TIME_SCALES)
            cycle = quantity(output, "DRAM Full Read Cycle", TIME_SCALES)
            write = quantity(output, "DRAM Write-Bitline Settling", TIME_SCALES)
            refresh = quantity(output, "Refresh Latency", TIME_SCALES)
            bandwidth = quantity(output, "Read Bandwidth", {"GB/s": 1e9, "MB/s": 1e6})
            self.assertAlmostEqual(cycle, access + restore, delta=2e-12)
            samples.append((access, restore, cycle, write, refresh, bandwidth))

        self.assertAlmostEqual(samples[0][0], samples[1][0], delta=1e-15)
        self.assertAlmostEqual(samples[1][0], samples[2][0], delta=1e-15)
        for index in (1, 2, 3, 4):
            self.assertLess(samples[0][index], samples[1][index])
            self.assertLess(samples[1][index], samples[2][index])
        self.assertGreater(samples[0][5], samples[1][5])
        self.assertGreater(samples[1][5], samples[2][5])

    def test_gcdram_components_are_nonnegative_and_driver_is_counted_once(self):
        output = self.run_fixture("fixed_gcdram_512x128.cfg").stdout
        write_section = output.split(" - Write Dynamic Energy =", 1)[1].split(
            " - Refresh Dynamic Energy =", 1
        )[0]
        mat_total = quantity(write_section, "Mat Dynamic Energy", ENERGY_SCALES)
        component_labels = (
            "Row Decoder Dynamic Energy",
            "Mux Decoder Dynamic Energy",
            "Mux Dynamic Energy",
            "gcDRAM Write-Bitline/Access Energy",
            "gcDRAM Write-Charge-Driver Energy",
        )
        components = [quantity(write_section, label, ENERGY_SCALES) for label in component_labels]
        read_access = quantity(output, "gcDRAM Read-Bitline/Access Energy", ENERGY_SCALES)
        for value in (*components, read_access, mat_total):
            self.assertTrue(math.isfinite(value))
            self.assertGreaterEqual(value, 0)
        self.assertGreater(components[-1], 0)
        # Printed component precision is 0.001 pJ; this equality distinguishes
        # one write-driver contribution from the historical negative/subtracted
        # result and from accidentally adding the driver twice.
        self.assertAlmostEqual(mat_total, sum(components), delta=0.005e-12)

        refresh_section = output.split(" - Refresh Dynamic Energy =", 1)[1].split(
            " - Leakage Power =", 1
        )[0]
        refresh_mat_total = quantity(refresh_section, "Mat Dynamic Energy", ENERGY_SCALES)
        refresh_labels = (
            "gcDRAM Refresh Read-Bitline/Access Energy Per Row",
            "gcDRAM Refresh Write-Bitline/Access Energy Per Row",
            "gcDRAM Refresh Read-Row-Decoder Energy Per Row",
            "gcDRAM Refresh Write-Row-Decoder Energy Per Row",
            "gcDRAM Refresh Read-Precharger Energy Per Row",
            "gcDRAM Refresh Write-Charger Energy Per Row",
            "gcDRAM Refresh Sense-Amp Energy Per Row",
        )
        refresh_components = [
            quantity(refresh_section, label, ENERGY_SCALES) for label in refresh_labels
        ]
        row_multiplier = int(
            re.search(r"gcDRAM Refresh Row Multiplier = (\d+)", refresh_section).group(1)
        )
        per_row_sum = quantity(
            refresh_section, "gcDRAM Refresh Energy Per-Row Sum", ENERGY_SCALES
        )
        reconstructed = quantity(
            refresh_section, "gcDRAM Reconstructed Mat Refresh Energy", ENERGY_SCALES
        )
        self.assertEqual(row_multiplier, 512 + 2)
        # Each displayed per-row term is rounded to 0.001 pJ.  Allow the
        # corresponding worst-case accumulated display error over 514 rows.
        self.assertAlmostEqual(per_row_sum, sum(refresh_components), delta=0.004e-12)
        self.assertAlmostEqual(
            refresh_mat_total,
            sum(refresh_components) * row_multiplier,
            delta=0.004e-12 * row_multiplier,
        )
        self.assertAlmostEqual(refresh_mat_total, reconstructed, delta=0.002e-9)

        subprocess.run([str(self.copy_probe)], check=True, cwd=ROOT)

    def test_cache_file_report_keeps_summary_and_both_array_details(self):
        report_path = self.temp_dir / "cache-report.txt"
        subprocess.run([str(self.copy_probe), str(report_path)], check=True, cwd=ROOT)
        report = report_path.read_text(encoding="utf-8")
        summary = report.index("CACHE DESIGN -- SUMMARY")
        data_details = report.index("CACHE DATA ARRAY DETAILS")
        tag_details = report.index("CACHE TAG ARRAY DETAILS")
        self.assertLess(summary, data_details)
        self.assertLess(data_details, tag_details)
        self.assertIn("Tag Array Area", report)
        self.assertIn(" - Total Area = 2mm^2", report)
        self.assertIn(" - Cache Total Leakage Power  = 2mW", report)

    def test_gcdram_area_breakdown_reports_only_instantiated_peripherals(self):
        gcdram_output = self.run_fixture("fixed_aos_gcdram_8x1024.cfg").stdout
        read_decoder_area = quantity(
            gcdram_output, "Mat gcDRAM Read Row Decoder Area", AREA_SCALES
        )
        write_charger_area = quantity(
            gcdram_output, "Mat gcDRAM Write Charger Area", AREA_SCALES
        )
        displayed_precharger_area = quantity(
            gcdram_output, "Mat precharger Area", AREA_SCALES
        )
        self.assertGreater(read_decoder_area, 0)
        self.assertGreater(write_charger_area, 0)
        # The existing generic precharger line retains its historical two-sided
        # display convention; gcDRAM instantiates one identically sized write
        # charger, so its actual component area is half that displayed value.
        self.assertAlmostEqual(
            write_charger_area * 2, displayed_precharger_area, delta=0.003e-12
        )
        self.assertNotIn("Mat writeDriver Area", gcdram_output)

        sram_output = self.run_fixture(
            "fixed_sram_512x128.cfg", "-ViewMatStatistics: true"
        ).stdout
        self.assertNotIn("Mat gcDRAM Read Row Decoder Area", sram_output)
        self.assertNotIn("Mat gcDRAM Write Charger Area", sram_output)

    def test_gem5_refresh_export_uses_restore_aware_aggregate_bank_timing(self):
        edram_latencies = []
        for residual in (0.20, 0.10, 0.05):
            output = self.run_fixture(
                "fixed_edram_cache_quantized.cfg",
                f"-DRAMTargetResidualRatio: {residual}",
            ).stdout
            refresh_cycles = gem5_option(output, "l2_refresh_latency")
            refresh_period = gem5_option(output, "l2_refresh_period")
            read_cycles = gem5_option(output, "data_read_latency")
            aggregate = quantity(output, "Cache Refresh Latency", TIME_SCALES)
            self.assertEqual(refresh_period, 40_000_000)
            # Cache Refresh Latency is formatted to 0.001 us, hence 500 cycles
            # of display uncertainty at this fixture's 1 THz clock.
            self.assertAlmostEqual(refresh_cycles, aggregate * 1e12, delta=501)
            self.assertGreater(refresh_cycles, read_cycles)
            edram_latencies.append(refresh_cycles)

        self.assertLess(edram_latencies[0], edram_latencies[1])
        self.assertLess(edram_latencies[1], edram_latencies[2])

        gcdram_output = self.run_fixture("fixed_gcdram_cache_quantized.cfg").stdout
        gcdram_refresh = gem5_option(gcdram_output, "l2_refresh_latency")
        gcdram_period = gem5_option(gcdram_output, "l2_refresh_period")
        gcdram_read = gem5_option(gcdram_output, "data_read_latency")
        gcdram_write = gem5_option(gcdram_output, "data_write_latency")
        gcdram_aggregate = quantity(gcdram_output, "Cache Refresh Latency", TIME_SCALES)
        self.assertEqual(gcdram_period, 40_000_000)
        self.assertAlmostEqual(gcdram_refresh, gcdram_aggregate * 1e12, delta=501)
        # The old proxy was one read response plus one write response.  The
        # modeled refresh is a full row sweep through both split paths.
        self.assertGreater(gcdram_refresh, gcdram_read + gcdram_write)

    def test_m3d_hard_tier_caps_and_accounting_identities(self):
        expected_tiers = {1: 1, 2: 2, 3: 2, 4: 4}
        samples = {}
        for limit, expected in expected_tiers.items():
            output = self.run_fixture(
                "fixed_sram_512x128.cfg",
                "-TSVRedundancy: 1.25\n"
                "-M3DMemory: true\n"
                f"-LimitMonolithicTier (N): {limit}",
            ).stdout
            tiers = int(re.search(r"Mat Memory Tiers = (\d+)", output).group(1))
            per_tier = int(re.search(r"MIVs Per Tier = (\d+)", output).group(1))
            total_count = int(re.search(r"Total MIV Count = (\d+)", output).group(1))
            total_area = quantity(output, "Total MIV Area", AREA_SCALES)
            base_logic = quantity(output, "Base Peripheral Logic Area", AREA_SCALES)
            final_logic = quantity(output, "Final Logic-Layer Area", AREA_SCALES)
            memory = quantity(output, "Per-Tier Memory Area", AREA_SCALES)
            projected = quantity(output, "Projected MAT Area", AREA_SCALES)
            dominant = re.search(r"Dominant Tier = (\w+)", output).group(1)

            self.assertEqual(tiers, expected)
            self.assertLessEqual(tiers, limit)
            self.assertEqual(per_tier, math.ceil(2 * (512 + 2 * 128) * 1.25))
            self.assertEqual(total_count, per_tier * tiers)
            self.assertAlmostEqual(final_logic, base_logic + total_area, delta=0.002e-12)
            self.assertAlmostEqual(projected, max(final_logic, memory), delta=0.002e-12)
            self.assertEqual(dominant, "logic" if final_logic >= memory else "memory")
            samples[limit] = (tiers, total_count, total_area, memory, dominant)

        self.assertEqual(samples[1][4], "memory")
        self.assertEqual(samples[2][0], samples[3][0])
        self.assertEqual(samples[2][1], samples[3][1])
        self.assertAlmostEqual(samples[2][2], samples[3][2], delta=1e-18)
        unit_miv_area = samples[1][2] / samples[1][1]
        for _, total_count, total_area, _, _ in samples.values():
            self.assertAlmostEqual(total_area, unit_miv_area * total_count, delta=1e-18)
        base_memory_area = samples[1][3] * samples[1][0]
        for tiers, _, _, memory, _ in samples.values():
            self.assertAlmostEqual(memory * tiers, base_memory_area, delta=0.002e-12)

    def test_aos_real_mat_leakage_formula_and_monotonicity(self):
        edram_output = self.run_fixture("fixed_aos_edram_512x128.cfg").stdout
        gcdram_output = self.run_fixture("fixed_aos_gcdram_8x1024.cfg").stdout
        self.assertIn("AOS compact-model v1", edram_output)
        self.assertIn("AOS compact-model v1", gcdram_output)

        access = operating_point(edram_output, "Access")
        edram_bound = quantity(edram_output, "AOS full-Vds leakage upper bound", POWER_SCALES)
        # The fixed 45-nm HP fixture has Vdd=1 V.
        self.assert_relative_equal(edram_bound, 1.0 * 512 * 128 * access["Ioff"])

        read = operating_point(gcdram_output, "Read")
        write = operating_point(gcdram_output, "Write")
        gcdram_bound = quantity(gcdram_output, "AOS full-Vds leakage upper bound", POWER_SCALES)
        self.assert_relative_equal(
            gcdram_bound,
            1.0 * (8 + 2) * 1024 * (read["Ioff"] + write["Ioff"]),
        )

        base_config = (FIXTURES / "fixed_aos_edram_512x128.cfg").read_text(encoding="utf-8")
        half_cells = base_config.replace("-Capacity (B): 8192", "-Capacity (B): 4096").replace(
            "512 x 128", "256 x 128"
        )
        half_output = self.run_config_text(half_cells, "aos-edram-half-cells").stdout
        half_bound = quantity(half_output, "AOS full-Vds leakage upper bound", POWER_SCALES)
        self.assertGreater(edram_bound, half_bound)
        self.assert_relative_equal(edram_bound, 2 * half_bound)

        low_vdd = base_config.replace("-ProcessNode: 45", "-ProcessNode: 14").replace(
            "-DeviceRoadmap: HP", "-DeviceRoadmap: LOP"
        )
        low_vdd_output = self.run_config_text(low_vdd, "aos-edram-low-vdd").stdout
        low_vdd_bound = quantity(low_vdd_output, "AOS full-Vds leakage upper bound", POWER_SCALES)
        self.assertLess(low_vdd_bound, edram_bound)

        high_ioff_cell = self.write_cell_variant(
            "synthetic_aos_edram.cell",
            (
                "-OxideAccessTransistorLeakageScale: 1.0",
                "-OxideAccessTransistorLeakageScale: 2.0",
            ),
            "aos-edram-high-ioff",
        )
        high_ioff_config = base_config.replace(
            "tests/fixtures/synthetic_aos_edram.cell", str(high_ioff_cell)
        )
        high_ioff_output = self.run_config_text(high_ioff_config, "aos-edram-high-ioff").stdout
        high_ioff_point = operating_point(high_ioff_output, "Access")
        high_ioff_bound = quantity(
            high_ioff_output, "AOS full-Vds leakage upper bound", POWER_SCALES
        )
        self.assertGreater(high_ioff_point["Ioff"], access["Ioff"])
        self.assertGreater(high_ioff_bound, edram_bound)

    def test_aos_gcdram_read_and_write_device_perturbations_are_isolated(self):
        config = (FIXTURES / "fixed_aos_gcdram_8x1024.cfg").read_text(encoding="utf-8")
        baseline_output = self.run_config_text(config, "aos-gcdram-baseline").stdout
        baseline_read = operating_point(baseline_output, "Read")
        baseline_write = operating_point(baseline_output, "Write")
        baseline_read_delay = quantity(
            baseline_output, "gcDRAM Read-Bitline Delay", TIME_SCALES
        )
        baseline_write_delay = quantity(
            baseline_output, "gcDRAM Write-Bitline Delay", TIME_SCALES
        )

        read_cell = self.write_cell_variant(
            "synthetic_aos_gcdram.cell",
            (
                "-OxideReadTransistorMobilityScale: 1.0",
                "-OxideReadTransistorMobilityScale: 0.5",
            ),
            "aos-gcdram-read-slower",
        )
        read_output = self.run_config_text(
            config.replace("tests/fixtures/synthetic_aos_gcdram.cell", str(read_cell)),
            "aos-gcdram-read-slower",
        ).stdout
        changed_read = operating_point(read_output, "Read")
        unchanged_write = operating_point(read_output, "Write")
        self.assertGreater(changed_read["Ron"], baseline_read["Ron"])
        self.assertGreater(
            quantity(read_output, "gcDRAM Read-Bitline Delay", TIME_SCALES),
            baseline_read_delay,
        )
        for name in baseline_write:
            self.assert_relative_equal(unchanged_write[name], baseline_write[name])
        self.assert_relative_equal(
            quantity(read_output, "gcDRAM Write-Bitline Delay", TIME_SCALES),
            baseline_write_delay,
        )

        write_cell = self.write_cell_variant(
            "synthetic_aos_gcdram.cell",
            (
                "-OxideWriteTransistorMobilityScale: 1.0",
                "-OxideWriteTransistorMobilityScale: 0.5",
            ),
            "aos-gcdram-write-slower",
        )
        write_output = self.run_config_text(
            config.replace("tests/fixtures/synthetic_aos_gcdram.cell", str(write_cell)),
            "aos-gcdram-write-slower",
        ).stdout
        unchanged_read = operating_point(write_output, "Read")
        changed_write = operating_point(write_output, "Write")
        self.assertGreater(changed_write["Ron"], baseline_write["Ron"])
        self.assertGreater(
            quantity(write_output, "gcDRAM Write-Bitline Delay", TIME_SCALES),
            baseline_write_delay,
        )
        for name in baseline_read:
            self.assert_relative_equal(unchanged_read[name], baseline_read[name])
        self.assert_relative_equal(
            quantity(write_output, "gcDRAM Read-Bitline Delay", TIME_SCALES),
            baseline_read_delay,
        )

    def test_conventional_gcdram_golden_is_unchanged(self):
        output = self.run_fixture("fixed_gcdram_512x128.cfg").stdout
        golden = self.golden["fixed_cmos_gcdram_512x128"]
        self.assertAlmostEqual(
            quantity(output, "Precharge Latency", TIME_SCALES) / 1e-12,
            golden["precharge_delay_ps"],
            delta=golden["absolute_tolerance_ps"],
        )
        self.assertAlmostEqual(
            quantity(output, "gcDRAM Read-Bitline Delay", TIME_SCALES) / 1e-6,
            golden["read_bitline_delay_us"],
            delta=golden["absolute_tolerance_us"],
        )
        self.assertAlmostEqual(
            quantity(output, "gcDRAM Write-Bitline Delay", TIME_SCALES) / 1e-12,
            golden["write_bitline_delay_ps"],
            delta=golden["absolute_tolerance_ps"],
        )
        self.assertNotIn("AOS compact-model", output)
        self.assertNotIn("AOS full-Vds leakage upper bound", output)


if __name__ == "__main__":
    unittest.main()
