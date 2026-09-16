#!/usr/bin/env python3
"""Build the compact-model probe and compare its numeric output to golden data."""

import json
import math
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]


class AOSCompactModelTest(unittest.TestCase):
    def test_equations_and_guardrails(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = pathlib.Path(directory) / "aos_compact_probe"
            subprocess.run(
                [
                    "c++",
                    "-std=c++17",
                    "-Wall",
                    "-Wextra",
                    "-pedantic",
                    "-I",
                    str(ROOT / "src"),
                    str(ROOT / "tests/cpp/aos_compact_probe.cpp"),
                    str(ROOT / "src/AOSFETCompactModel.cpp"),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            output = subprocess.run(
                [str(executable)], check=True, text=True, capture_output=True
            ).stdout.splitlines()

        golden = json.loads(
            (ROOT / "tests/fixtures/aos_compact_golden.json").read_text(encoding="utf-8")
        )
        self.assertEqual(len(output), len(golden["points"]))
        tolerance = golden["relative_tolerance"]
        names = ("vgs", "vds", "mobility", "surface_potential", "drain_current", "cgs", "cgd")
        for line, expected in zip(output, golden["points"]):
            actual = dict(zip(names, map(float, line.split())))
            for name in names:
                scale = max(abs(expected[name]), 1e-30)
                self.assertLessEqual(
                    abs(actual[name] - expected[name]) / scale,
                    tolerance,
                    f"{name} mismatch at Vgs={expected['vgs']}, Vds={expected['vds']}",
                )
                self.assertTrue(math.isfinite(actual[name]))


if __name__ == "__main__":
    unittest.main()
