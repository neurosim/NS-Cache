#!/usr/bin/env python3
"""Compact cross-product coverage for the integrated DRAM model paths."""

import itertools
import math
import pathlib
import re
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests" / "fixtures"


class IntegrationMatrixTest(unittest.TestCase):
    """Exercise every selected model across each integration boundary."""

    CELL_CASES = (
        ("fixed_edram_512x128.cfg", "Embedded DRAM", False, (512, 128)),
        ("fixed_gcdram_512x128.cfg", "Gain Cell DRAM", False, (512, 128)),
        ("fixed_aos_edram_512x128.cfg", "Embedded DRAM", True, (512, 128)),
        # Keep the M3D refresh cycle inside the synthetic cell's explicit
        # 40-us retention window after MIV loading.  The original 8x1024
        # characterization fixture intentionally exceeds that physical limit.
        ("fixed_aos_gcdram_8x1024.cfg", "Gain Cell DRAM", True, (4, 512)),
    )

    @classmethod
    def setUpClass(cls):
        cls._temporary_directory = tempfile.TemporaryDirectory(prefix="nsc-matrix-tests-")
        cls.temp_dir = pathlib.Path(cls._temporary_directory.name)
        subprocess.run(
            ["make", "-C", str(ROOT / "src"), "-j2"],
            check=True,
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

    @classmethod
    def tearDownClass(cls):
        cls._temporary_directory.cleanup()

    def write_case(self, fixture, routing, m3d_enabled, forced, mat_size):
        source = (FIXTURES / fixture).read_text(encoding="utf-8")
        # Monolithic-via geometry is characterized by this project for its
        # advanced-node technology branch.  Normalize the small matrix cases
        # to that common environment so this test targets integration paths,
        # rather than the absence of MIV data in older technology tables.
        source = re.sub(r"(?m)^-ProcessNode:.*$", "-ProcessNode: 14", source)
        source = re.sub(r"(?m)^-DeviceRoadmap:.*$", "-DeviceRoadmap: LOP", source)
        rows, columns = mat_size
        source = re.sub(
            r"(?m)^-Capacity \(B\):.*$",
            f"-Capacity (B): {rows * columns // 8}",
            source,
        )
        source = re.sub(
            r"(?m)^-WordWidth \(bit\):.*$", f"-WordWidth (bit): {columns}", source
        )
        source = re.sub(
            r"(?m)^-ForceMatSize \(Rows x Columns\):.*$",
            f"-ForceMatSize (Rows x Columns): {rows} x {columns}",
            source,
        )
        source = re.sub(r"(?m)^-Routing:.*$", f"-Routing: {routing}", source)
        source = re.sub(r"(?m)^-M3DMemory:.*\n?", "", source)
        source = re.sub(r"(?m)^-LimitMonolithicTier \(N\):.*\n?", "", source)
        if not forced:
            source = re.sub(
                r"(?m)^-ForceMatSize \(Rows x Columns\):.*\n?", "", source
            )
        source += (
            f"\n-M3DMemory: {'true' if m3d_enabled else 'false'}\n"
            "-LimitMonolithicTier (N): 3\n"
        )
        destination = self.temp_dir / (
            f"{pathlib.Path(fixture).stem}-"
            f"{'htree' if routing == 'H-tree' else 'nonhtree'}-"
            f"{'m3d' if m3d_enabled else '2d'}-"
            f"{'forced' if forced else 'unforced'}.cfg"
        )
        destination.write_text(source, encoding="utf-8")
        return destination

    def test_full_selected_model_integration_matrix(self):
        combinations = itertools.product(
            self.CELL_CASES,
            ("H-tree", "Non-H-tree"),
            (False, True),
            (False, True),
        )
        executed = 0
        for (fixture, cell_label, aos, forced_size), routing, m3d, forced in combinations:
            case = self.write_case(fixture, routing, m3d, forced, forced_size)
            with self.subTest(
                cell=fixture, routing=routing, m3d=m3d, forced_mat=forced
            ):
                result = subprocess.run(
                    [str(ROOT / "nsc"), str(case)],
                    cwd=ROOT,
                    text=True,
                    capture_output=True,
                    timeout=30,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                output = result.stdout
                self.assertIn("numSolutions = 1 / numDesigns = 1", output)
                self.assertIn(f"Memory Cell: {cell_label}", output)
                self.assertNotRegex(output.lower(), r"\b(?:nan|inf)\b")

                mat_size = re.search(
                    r"Mat Size\s+: (\d+) Rows x (\d+) Columns", output
                )
                self.assertIsNotNone(mat_size, output)
                rows, columns = map(int, mat_size.groups())
                self.assertGreater(rows, 0)
                self.assertGreater(columns, 0)
                if forced:
                    self.assertEqual((rows, columns), forced_size)
                    self.assertIn(
                        f"Forced Data MAT Size: {forced_size[0]} Rows x "
                        f"{forced_size[1]} Columns",
                        output,
                    )
                else:
                    self.assertIn("Forced Data MAT Size: Disabled", output)

                routing_label = (
                    "|--- H-Tree Latency ="
                    if routing == "H-tree"
                    else "|--- Non-H-Tree Latency ="
                )
                self.assertIn(routing_label, output)

                if m3d:
                    self.assertIn("Monolithic 3D MAT: Enabled", output)
                    tiers_match = re.search(r"Mat Memory Tiers = (\d+)", output)
                    self.assertIsNotNone(tiers_match, output)
                    tiers = int(tiers_match.group(1))
                    self.assertLessEqual(tiers, 3)
                    self.assertEqual(tiers & (tiers - 1), 0)
                    self.assertIn("MIVs Per Tier =", output)
                    self.assertIn("Projected MAT Area =", output)
                else:
                    self.assertIn("Monolithic 3D MAT: Disabled", output)
                    self.assertNotIn("Mat Memory Tiers =", output)
                    self.assertNotIn("MIVs Per Tier =", output)

                if aos:
                    self.assertIn("AOS operating point:", output)
                    self.assertIn("AOS full-Vds leakage upper bound", output)
                else:
                    self.assertNotIn("AOS operating point:", output)
                    self.assertNotIn("AOS full-Vds leakage upper bound", output)

                if cell_label == "Embedded DRAM":
                    self.assertIn("DRAM timing model: restore-aware v1", output)
                    self.assertIn("DRAM Full Read Cycle", output)
                else:
                    self.assertIn(
                        "gcDRAM electrical/energy model: split-path v1", output
                    )
                    self.assertIn("gcDRAM Read-Bitline/Access Energy", output)
                    self.assertIn("gcDRAM Write-Charge-Driver Energy", output)

                latency = re.search(r"-\s+Read Latency = ([0-9.eE+-]+)", output)
                self.assertIsNotNone(latency, output)
                self.assertTrue(math.isfinite(float(latency.group(1))))
                self.assertGreater(float(latency.group(1)), 0)
                executed += 1

        self.assertEqual(executed, 32)


if __name__ == "__main__":
    unittest.main()
