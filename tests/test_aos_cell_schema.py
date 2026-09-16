#!/usr/bin/env python3
"""Exercise strict AOS cell parsing with synthetic, malformed, and unused inputs."""

import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class AOSCellSchemaTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._temporary = tempfile.TemporaryDirectory()
        cls.executable = pathlib.Path(cls._temporary.name) / "aos_cell_parser_probe"
        subprocess.run(
            [
                "c++",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                "-pedantic",
                "-I",
                str(ROOT / "src"),
                str(ROOT / "tests/cpp/aos_cell_parser_probe.cpp"),
                str(ROOT / "src/AOSFETCompactModel.cpp"),
                str(ROOT / "src/MemCell.cpp"),
                str(ROOT / "src/InputParameter.cpp"),
                str(ROOT / "src/Technology.cpp"),
                "-o",
                str(cls.executable),
            ],
            check=True,
        )

    @classmethod
    def tearDownClass(cls):
        cls._temporary.cleanup()

    def run_fixture(self, name):
        return subprocess.run(
            [str(self.executable), str(ROOT / "tests/fixtures" / name)],
            text=True,
            capture_output=True,
        )

    def test_synthetic_edram_and_gcdram(self):
        edram = self.run_fixture("synthetic_aos_edram.cell")
        gcdram = self.run_fixture("synthetic_aos_gcdram.cell")
        self.assertEqual(edram.returncode, 0, edram.stderr + edram.stdout)
        self.assertEqual(gcdram.returncode, 0, gcdram.stderr + gcdram.stdout)
        self.assertIn("edram ", edram.stdout)
        self.assertIn("retention-ratio=1", edram.stdout)
        self.assertIn("gcdram ", gcdram.stdout)

    def test_boolean_spellings_and_cmos_fallback(self):
        valid = (ROOT / "tests/fixtures/synthetic_aos_edram.cell").read_text(encoding="utf-8")
        with tempfile.TemporaryDirectory() as directory:
            for index, spelling in enumerate(("true", "TRUE", "yes", "YES", "1")):
                path = pathlib.Path(directory) / f"enabled-{index}.cell"
                path.write_text(
                    valid.replace("-OxideTransistor: true", f"-OxideTransistor: {spelling}"),
                    encoding="utf-8",
                )
                result = subprocess.run(
                    [str(self.executable), str(path)], text=True, capture_output=True
                )
                self.assertEqual(result.returncode, 0, spelling)

            conventional = (ROOT / "config/Old_Configs/eDRAM.cell").read_text(encoding="utf-8")
            for index, spelling in enumerate(("false", "FALSE", "no", "NO", "0")):
                path = pathlib.Path(directory) / f"disabled-{index}.cell"
                path.write_text(
                    conventional + f"\n-OxideTransistor: {spelling}\n", encoding="utf-8"
                )
                result = subprocess.run(
                    [str(self.executable), str(path)], text=True, capture_output=True
                )
                self.assertEqual(result.returncode, 0, spelling)
                self.assertIn("cmos", result.stdout)

    def test_lower_applied_hold_current_increases_retention(self):
        valid = (ROOT / "tests/fixtures/synthetic_aos_edram.cell").read_text(encoding="utf-8")
        lower_hold = valid.replace(
            "-OxideAccessTransistorOffGateVoltage (V): 0.0",
            "-OxideAccessTransistorOffGateVoltage (V): -0.1",
        )
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "lower_hold.cell"
            path.write_text(lower_hold, encoding="utf-8")
            result = subprocess.run(
                [str(self.executable), str(path)], text=True, capture_output=True
            )
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        ratio = float(result.stdout.rsplit("retention-ratio=", 1)[1].split()[0])
        self.assertGreater(ratio, 1.0)

    def test_gcdram_retention_inference_is_rejected(self):
        valid = (ROOT / "tests/fixtures/synthetic_aos_gcdram.cell").read_text(encoding="utf-8")
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "gcdram_retention.cell"
            path.write_text(
                valid
                + "\n-RetentionModel: AOSOffCurrentRatio"
                + "\n-RetentionReferenceHoldVoltage (V): 0\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [str(self.executable), str(path)], text=True, capture_output=True
            )
        self.assertNotEqual(result.returncode, 0)

    def test_incomplete_and_unused_fields_fail(self):
        valid = (ROOT / "tests/fixtures/synthetic_aos_edram.cell").read_text(encoding="utf-8")
        cases = {
            "incomplete": valid.replace("-OxideAccessTransistorLeakageScale: 1.0\n", ""),
            "disabled": valid.replace("-OxideTransistor: true", "-OxideTransistor: false"),
            "bad_boolean": valid.replace("-OxideTransistor: true", "-OxideTransistor: enabled"),
            "wrong_cell_type": valid.replace("-MemCellType: eDRAM", "-MemCellType: SRAM"),
            "wrong_path": valid + "\n-OxideReadTransistorWidth (m): 1e-7\n",
            "nan": valid.replace("-OxideAccessTransistorWidth (m): 100e-9", "-OxideAccessTransistorWidth (m): NaN"),
            "trailing": valid.replace("-OxideAccessTransistorLength (m): 50e-9", "-OxideAccessTransistorLength (m): 50e-9 junk"),
            "malformed": valid.replace("-OxideAccessTransistorLength (m): 50e-9", "-OxideAccessTransistorLength (m) 50e-9"),
            "bad_bias": valid.replace("-OxideAccessTransistorOnGateVoltage (V): VDD", "-OxideAccessTransistorOnGateVoltage (V): 0"),
            "bad_temperature": valid.replace("-OxideAccessTransistorTemperature (K): 300", "-OxideAccessTransistorTemperature (K): 301"),
			"wrong_width_unit": valid.replace("-OxideAccessTransistorWidth (m): 100e-9", "-OxideAccessTransistorWidth (nm): 100"),
			"malformed_width_unit": valid.replace("-OxideAccessTransistorWidth (m): 100e-9", "-OxideAccessTransistorWidth (m) junk: 100e-9"),
			"missing_voltage_unit": valid.replace("-OxideAccessTransistorOnGateVoltage (V): VDD", "-OxideAccessTransistorOnGateVoltage: VDD"),
        }
        with tempfile.TemporaryDirectory() as directory:
            for name, contents in cases.items():
                path = pathlib.Path(directory) / f"{name}.cell"
                path.write_text(contents, encoding="utf-8")
                result = subprocess.run(
                    [str(self.executable), str(path)], text=True, capture_output=True
                )
                self.assertNotEqual(result.returncode, 0, name)


if __name__ == "__main__":
    unittest.main()
