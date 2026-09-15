#!/usr/bin/env python3
"""
run_tests.py - Universal Master Test Runner for Penguin Desktop Media Player.
Tactile Digital Brutalism // Studio Precision Verification Engine.

Executes and reports multi-tier opaque-box test suites:
- Tier 1: Category-Partition Feature Coverage (135 tests across 27 features)
- Tier 2: Boundary Value Analysis & Edge Cases (135 tests across 27 features)
- Tier 3: Pairwise Combinatorial Matrix (27 tests)
- Tier 4: Real-World Multi-Step Workloads (14 tests)
- Tier 5: Adversarial Hardening (20 tests)
- Auxiliary Subsystem Test Suites (Synthetic Media, SMPTE, EQ DSP, LRC, MPRIS2, Persistence, CLI)

CLI Flags:
  --tier {1,2,3,4,5,all}  Filter execution by tier
  --headless              Enforce headless offscreen QPA and isolated D-Bus
  --verbose, -v           Verbose test output
  --json <path>           Output structured test report JSON
"""

import argparse
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import unittest
from typing import Any, Dict, List, Optional, Tuple

# Ensure project root is on sys.path
PROJECT_ROOT = Path(__file__).resolve().parent
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tests.test_synthetic_media import SyntheticMediaFactory, TestSyntheticMediaGenerator
from tests.test_timecode import TestSMPTETimecode
from tests.test_equalizer_dsp import TestEqualizerDSP
from tests.test_lrc_parser import TestLRCParser
from tests.test_mpris2_dbus import TestMPRIS2Compliance
from tests.test_persistence import TestPersistence
from tests.test_cli_desktop import TestCLIDesktop
from tests.test_headless_cli import TestHeadlessCLI
from tests.test_ui_m2_complete import (
    TestBrutalistTheme,
    TestMechanicalTickScrubber,
    TestStereoVUMeterRack,
    TestEqualizerRackWidget,
    TestSynchronizedLyricsTeleprompter,
    TestPlaylistMatrixWidget,
    TestViewfinderAndReticles,
    TestAudioDeckWidget,
    TestMainWindowOrchestration,
)
from tests.test_tier1_features import TestTier1Features
from tests.test_tier2_boundaries import TestTier2Boundaries
from tests.test_tier3_pairwise import TestTier3Pairwise
from tests.test_tier4_workloads import TestTier4Workloads
from tests.test_tier5_adversarial import TestTier5Adversarial


class PenguinTestRunner:
    """Universal Test Runner and Report Formatter for Penguin."""

    SUITE_MAP = {
        "auxiliary": [
            ("Synthetic Media Engine", [TestSyntheticMediaGenerator]),
            ("SMPTE Timecode Engine", [TestSMPTETimecode]),
            ("10-Band EQ & VU Meter DSP", [TestEqualizerDSP]),
            ("Synchronized LRC Parser", [TestLRCParser]),
            ("MPRIS2 D-Bus Compliance", [TestMPRIS2Compliance]),
            ("SQLite WAL Persistence", [TestPersistence]),
            ("CLI & Desktop Packaging", [TestCLIDesktop, TestHeadlessCLI]),
            (
                "Tactile Brutalist GUI Subsystem",
                [
                    TestBrutalistTheme,
                    TestMechanicalTickScrubber,
                    TestStereoVUMeterRack,
                    TestEqualizerRackWidget,
                    TestSynchronizedLyricsTeleprompter,
                    TestPlaylistMatrixWidget,
                    TestViewfinderAndReticles,
                    TestAudioDeckWidget,
                    TestMainWindowOrchestration,
                ],
            ),
        ],
        "1": [("Tier 1: Category Partition (27 Features)", [TestTier1Features])],
        "2": [("Tier 2: Boundary & Corner Cases (27 Features)", [TestTier2Boundaries])],
        "3": [("Tier 3: Pairwise Combinations", [TestTier3Pairwise])],
        "4": [("Tier 4: Real-World Workloads", [TestTier4Workloads])],
        "5": [("Tier 5: Adversarial Hardening", [TestTier5Adversarial])],
    }

    @classmethod
    def ensure_fixtures(cls):
        """Ensures all deterministic synthetic test media assets exist in tests/fixtures."""
        fixtures_dir = PROJECT_ROOT / "tests" / "fixtures"
        fixtures_dir.mkdir(parents=True, exist_ok=True)
        SyntheticMediaFactory.create_all_fixtures(fixtures_dir)

    @classmethod
    def build_test_plan(cls, tier_selection: str) -> List[Tuple[str, List[Any]]]:
        suites: List[Tuple[str, List[Any]]] = []
        if tier_selection == "all":
            for cat in ["auxiliary", "1", "2", "3", "4", "5"]:
                suites.extend(cls.SUITE_MAP[cat])
        elif tier_selection in cls.SUITE_MAP:
            suites.extend(cls.SUITE_MAP[tier_selection])
        else:
            raise ValueError(f"Unknown tier: {tier_selection}")
        return suites

    @classmethod
    def run(
        cls,
        tier: str = "all",
        headless: bool = True,
        verbose: bool = False,
        json_path: Optional[Path] = None,
    ) -> int:
        start_time = time.time()

        if headless:
            os.environ["QT_QPA_PLATFORM"] = "offscreen"

        cls.ensure_fixtures()
        suites_to_run = cls.build_test_plan(tier)

        total_ran = 0
        total_errors = 0
        total_failures = 0
        suite_metrics = []

        print("=" * 80)
        print("                PENGUIN UNIVERSAL E2E TEST RUNNER               ")
        print("         [TACTILE DIGITAL BRUTALISM // STUDIO PRECISION]        ")
        print("=" * 80)
        if tier != "all":
            print(f" Target Filter : Tier {tier}")
        print(f" Environment   : Headless Offscreen ({os.environ.get('QT_QPA_PLATFORM', 'default')})")
        print(f" Python        : {sys.version.split()[0]} ({sys.executable})")
        print("-" * 80)

        for suite_title, test_classes in suites_to_run:
            suite_start = time.time()
            ts = unittest.TestSuite()
            loader = unittest.TestLoader()
            for tc in test_classes:
                ts.addTests(loader.loadTestsFromTestCase(tc))

            test_count = ts.countTestCases()
            stream = io.StringIO()
            runner = unittest.TextTestRunner(stream=stream, verbosity=2 if verbose else 1)
            result = runner.run(ts)
            suite_duration = time.time() - suite_start

            n_err = len(result.errors)
            n_fail = len(result.failures)
            status_str = "[PASS]" if (n_err == 0 and n_fail == 0) else "[FAIL]"

            total_ran += result.testsRun
            total_errors += n_err
            total_failures += n_fail

            suite_metrics.append({
                "name": suite_title,
                "count": result.testsRun,
                "errors": n_err,
                "failures": n_fail,
                "duration_sec": round(suite_duration, 4),
                "status": "PASS" if (n_err == 0 and n_fail == 0) else "FAIL",
            })

            dots = "." * max(2, 60 - len(suite_title) - len(str(result.testsRun)))
            print(f" {suite_title} ({result.testsRun} tests) {dots} {status_str} ({suite_duration:.3f}s)")

            if verbose and (n_err > 0 or n_fail > 0):
                print(stream.getvalue())

        total_elapsed = time.time() - start_time
        overall_pass = (total_errors == 0 and total_failures == 0)
        result_str = "PASS (EXIT 0)" if overall_pass else f"FAIL (Errors: {total_errors}, Failures: {total_failures})"

        print("=" * 80)
        print(f" TOTAL TEST CASES EXECUTED : {total_ran}")
        print(f" TOTAL FAILURES / ERRORS   : {total_failures} / {total_errors}")
        print(f" TOTAL ELAPSED TIME        : {total_elapsed:.3f}s")
        print(f" VERIFICATION RESULT       : {result_str}")
        print("=" * 80)

        # Emit JSON report
        if json_path is None:
            json_path = PROJECT_ROOT / "tests" / "test_report.json"
        
        report_data = {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "tier_filter": tier,
            "total_executed": total_ran,
            "total_failures": total_failures,
            "total_errors": total_errors,
            "total_elapsed_seconds": round(total_elapsed, 4),
            "status": "PASS" if overall_pass else "FAIL",
            "suites": suite_metrics,
        }
        with open(json_path, "w", encoding="utf-8") as jf:
            json.dump(report_data, jf, indent=2)
        print(f"[REPORT] Structured test report saved to {json_path}")

        return 0 if overall_pass else 1


def main():
    parser = argparse.ArgumentParser(
        prog="run_tests.py",
        description="Universal Master Test Runner for Penguin Desktop Media Player.",
    )
    parser.add_argument(
        "--tier",
        choices=["1", "2", "3", "4", "5", "all"],
        default="all",
        help="Execute specific test tier (default: all).",
    )
    parser.add_argument(
        "--headless",
        action="store_true",
        default=True,
        help="Execute in headless mode with QT_QPA_PLATFORM=offscreen.",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose test reporting.",
    )
    parser.add_argument(
        "--json",
        type=Path,
        default=None,
        help="Custom destination path for test_report.json.",
    )
    args = parser.parse_args()

    exit_code = PenguinTestRunner.run(
        tier=args.tier,
        headless=args.headless,
        verbose=args.verbose,
        json_path=args.json,
    )
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
