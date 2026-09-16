#!/usr/bin/env python3

import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class ConfigControlsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._temporary_directory = tempfile.TemporaryDirectory(prefix="nsc-config-tests-")
        cls.temp_dir = pathlib.Path(cls._temporary_directory.name)
        cls.probe = cls.temp_dir / "config_probe"
        subprocess.run(
            [
                "c++",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                f"-I{ROOT / 'src'}",
                str(ROOT / "tests" / "config_probe.cpp"),
                str(ROOT / "src" / "InputParameter.cpp"),
                "-o",
                str(cls.probe),
            ],
            check=True,
            cwd=ROOT,
        )
        # The integration case verifies that ForceMatSize filters a real search.
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

    def run_config(self, contents, print_resolved=False):
        config = self.temp_dir / f"case-{len(list(self.temp_dir.glob('case-*')))}.cfg"
        config.write_text(contents, encoding="utf-8")
        command = [str(self.probe), str(config)]
        if print_resolved:
            command.append("--print")
        return subprocess.run(command, cwd=ROOT, text=True, capture_output=True)

    def assert_invalid(self, directive):
        result = self.run_config(directive + "\n")
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("Invalid", result.stdout)

    def test_compatibility_defaults(self):
        result = self.run_config("")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("relax=1", result.stdout)
        self.assertIn("aspect=3", result.stdout)
        self.assertIn("force=0", result.stdout)
        self.assertIn("dram_residual=0.10000000000000001", result.stdout)
        self.assertIn("m3d=0", result.stdout)
        self.assertIn("max_tiers=4", result.stdout)

    def test_boolean_spellings_are_case_insensitive(self):
        for value in ("yes", "YES", "True", "1"):
            with self.subTest(value=value):
                result = self.run_config(
                    f"-RelaxSRAMCell: {value}\n-M3DMemory: {value}\n"
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertIn("relax=1", result.stdout)
                self.assertIn("m3d=1", result.stdout)
        for value in ("no", "NO", "False", "0"):
            with self.subTest(value=value):
                result = self.run_config(
                    f"-RelaxSRAMCell: {value}\n-M3DMemory: {value}\n"
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertIn("relax=0", result.stdout)
                self.assertIn("m3d=0", result.stdout)

    def test_numeric_and_mat_controls(self):
        result = self.run_config(
            "-BankAspectRatioLimit: 1000\n"
            "-ForceMatSize (Rows x Columns): 256 X 512\n"
            "-DRAMTargetResidualRatio: 0.25\n"
            "-M3DMemory: true\n"
            "-LimitMonolithicTier (N): 3\n"
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("aspect=1000", result.stdout)
        self.assertIn("force=1", result.stdout)
        self.assertIn("force_rows=256", result.stdout)
        self.assertIn("force_columns=512", result.stdout)
        self.assertIn("dram_residual=0.25", result.stdout)
        self.assertIn("max_tiers=3", result.stdout)

    def test_zero_disables_aspect_limit(self):
        result = self.run_config("-BankAspectRatioLimit: 0\n")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("aspect=0", result.stdout)

    def test_force_mat_accepts_full_uint64_range(self):
        result = self.run_config(
            "-ForceMatSize (Rows x Columns): "
            "18446744073709551615 x 18446744073709551615\n"
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("force_rows=18446744073709551615", result.stdout)
        self.assertIn("force_columns=18446744073709551615", result.stdout)

    def test_force_mat_size_filters_instead_of_overwriting(self):
        fixture = ROOT / "tests" / "fixtures" / "force_mat_sram.cfg"
        matching = subprocess.run(
            [str(ROOT / "nsc"), str(fixture)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=30,
        )
        self.assertEqual(matching.returncode, 0, matching.stdout + matching.stderr)
        self.assertIn("numSolutions = 1 / numDesigns = 1", matching.stdout)
        self.assertIn("Mat Size    : 256 Rows x 256 Columns", matching.stdout)

        mismatched_config = self.temp_dir / "force-mismatch.cfg"
        mismatched_config.write_text(
            fixture.read_text(encoding="utf-8").replace(
                "256 x 256", "128 x 256", 1
            ),
            encoding="utf-8",
        )
        mismatched = subprocess.run(
            [str(ROOT / "nsc"), str(mismatched_config)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            timeout=30,
        )
        self.assertIn("numSolutions = 0 / numDesigns = 1", mismatched.stdout)
        self.assertIn("No valid solutions.", mismatched.stdout)
        self.assertNotIn("Mat Size    : 128 Rows x 256 Columns", mismatched.stdout)

    def test_resolved_values_are_printed(self):
        result = self.run_config(
            "-DesignTarget: cache\n"
            "-Capacity (B): 8192\n"
            "-WordWidth (bit): 256\n"
            "-Associativity (for cache only): 4\n"
            "-RelaxSRAMCell: false\n"
            "-BankAspectRatioLimit: 0\n"
            "-ForceMatSize (Rows x Columns): 256 x 256\n"
            "-DRAMTargetResidualRatio: 0.2\n"
            "-M3DMemory: true\n"
            "-LimitMonolithicTier (N): 3\n",
            print_resolved=True,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("Relax SRAM Cell: Disabled", result.stdout)
        self.assertIn("Bank Aspect Ratio Limit: Disabled", result.stdout)
        self.assertIn("Forced Data MAT Size: 256 Rows x 256 Columns", result.stdout)
        self.assertIn("DRAM Target Residual Ratio: 0.2", result.stdout)
        self.assertIn("Monolithic 3D MAT: Enabled", result.stdout)
        self.assertIn("Monolithic MAT Tier Limit: 3", result.stdout)

    def test_invalid_booleans(self):
        for directive in (
            "-RelaxSRAMCell: maybe",
            "-RelaxSRAMCell: true trailing",
            "-M3DMemory: enabled",
            "-M3DMemory:",
        ):
            with self.subTest(directive=directive):
                self.assert_invalid(directive)

    def test_invalid_aspect_limits(self):
        for value in ("-1", "0.5", "nan", "inf", "3 trailing", ""):
            with self.subTest(value=value):
                self.assert_invalid(f"-BankAspectRatioLimit: {value}")

    def test_invalid_mat_sizes(self):
        for value in (
            "0 x 256",
            "256 x 0",
            "-1 x 256",
            "256",
            "256 by 256",
            "256 x 256 trailing",
            "18446744073709551616 x 1",
        ):
            with self.subTest(value=value):
                self.assert_invalid(f"-ForceMatSize (Rows x Columns): {value}")

    def test_invalid_dram_residual_ratios(self):
        for value in ("0", "1", "-0.1", "nan", "inf", "0.1 trailing", ""):
            with self.subTest(value=value):
                self.assert_invalid(f"-DRAMTargetResidualRatio: {value}")

    def test_invalid_tier_limits(self):
        for value in ("0", "-1", "1.5", "3 trailing", "2147483648", ""):
            with self.subTest(value=value):
                self.assert_invalid(f"-LimitMonolithicTier (N): {value}")

    def test_abbreviated_or_prefixed_control_names_are_rejected(self):
        for directive in (
            "-ForceMatSize: 256 x 256",
            "-LimitMonolithicTier: 3",
            "-RelaxSRAMCellExtra: true",
            "-M3DMemoryExtra: true",
            "-BankAspectRatioLimitExtra: 3",
            "-DRAMTargetResidualRatioExtra: 0.1",
        ):
            with self.subTest(directive=directive):
                self.assert_invalid(directive)


if __name__ == "__main__":
    unittest.main()
