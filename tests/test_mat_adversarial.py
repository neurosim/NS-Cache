#!/usr/bin/env python3
"""Adversarial identities for DRAM settling, refresh, and M3D rejection."""

import pathlib
import re
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests" / "fixtures"
NUMBER = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


class MatAdversarialTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._temporary_directory = tempfile.TemporaryDirectory(prefix="nsc-mat-adversarial-")
        cls.temp_dir = pathlib.Path(cls._temporary_directory.name)
        subprocess.run(
            ["make", "-C", str(ROOT / "src"), "-j2"],
            check=True,
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        objects = [
            item
            for item in sorted((ROOT / "src" / "obj").glob("*.o"))
            if item.name != "main.o"
        ]
        cls.probe = cls.temp_dir / "mat_adversarial_probe"
        subprocess.run(
            [
                "c++",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                f"-I{ROOT / 'src'}",
                str(ROOT / "tests" / "cpp" / "mat_adversarial_probe.cpp"),
                *(str(item) for item in objects),
                "-o",
                str(cls.probe),
            ],
            check=True,
            cwd=ROOT,
        )

    @classmethod
    def tearDownClass(cls):
        cls._temporary_directory.cleanup()

    def write_variant(self, fixture, settings, stem):
        path = self.temp_dir / f"{stem}.cfg"
        contents = (FIXTURES / fixture).read_text(encoding="utf-8")
        path.write_text(contents + "\n" + "\n".join(settings) + "\n", encoding="utf-8")
        return path

    def run_config(self, path):
        return subprocess.run(
            [str(ROOT / "nsc"), str(path)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=30,
        )

    def test_exact_settling_refresh_identity_and_m3d_load_propagation(self):
        sram_m3d = self.write_variant(
            "fixed_sram_512x128.cfg",
            ["-M3DMemory: true", "-LimitMonolithicTier (N): 4"],
            "sram-m3d-load",
        )
        gcdram_m3d = self.write_variant(
            "fixed_gcdram_512x128.cfg",
            ["-M3DMemory: true", "-LimitMonolithicTier (N): 4"],
            "gcdram-m3d-load",
        )
        subprocess.run(
            [
                str(self.probe),
                str(FIXTURES / "fixed_edram_512x128.cfg"),
                str(FIXTURES / "fixed_gcdram_512x128.cfg"),
                str(sram_m3d),
                str(gcdram_m3d),
            ],
            check=True,
            cwd=ROOT,
        )

    def test_high_redundancy_stops_before_a_worse_fold(self):
        samples = {}
        for limit in (1, 2, 4):
            config = self.write_variant(
                "fixed_sram_512x128.cfg",
                [
                    "-TSVRedundancy: 100",
                    "-M3DMemory: true",
                    f"-LimitMonolithicTier (N): {limit}",
                ],
                f"high-redundancy-{limit}",
            )
            result = self.run_config(config)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("numSolutions = 1 / numDesigns = 1", result.stdout)
            tiers = int(re.search(r"Mat Memory Tiers = (\d+)", result.stdout).group(1))
            projected = float(
                re.search(rf"Projected MAT Area = ({NUMBER})um\^2", result.stdout).group(1)
            )
            samples[limit] = (tiers, projected)

        self.assertEqual(samples[1][0], 1)
        self.assertEqual(samples[2][0], 2)
        self.assertEqual(samples[4][0], 2)
        self.assertLess(samples[2][1], samples[1][1])
        self.assertAlmostEqual(samples[4][1], samples[2][1], places=6)

    def test_m3d_count_overflow_rejects_candidate(self):
        config = self.write_variant(
            "fixed_sram_512x128.cfg",
            [
                "-TSVRedundancy: 1e20",
                "-M3DMemory: true",
                "-LimitMonolithicTier (N): 4",
            ],
            "m3d-overflow",
        )
        result = self.run_config(config)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("[Mat] Error: M3D MIV count or tier limit is outside the supported range.", result.stdout)
        self.assertIn("numSolutions = 0 / numDesigns = 1", result.stdout)
        self.assertIn("No valid solutions.", result.stdout)
        self.assertNotIn("numSolutions = 1 / numDesigns = 1", result.stdout)


if __name__ == "__main__":
    unittest.main()
