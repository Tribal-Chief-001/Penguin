"""
test_e2e_runner.py - Master Headless Verification Harness for Penguin Media Player.

Executes all unit, integration, and multi-tier E2E test suites with offscreen QPA,
verifies synthetic test media fixtures, collects structured pass/fail metrics,
emits a high-contrast Tactile Brutalist summary report, and exits with code 0 on full pass.
"""

import io
import json
import os
from pathlib import Path
import sys
import time
import unittest

# Ensure project root is in sys.path
PROJECT_ROOT = Path(__file__).resolve().parent.parent
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from tests.test_synthetic_media import SyntheticMediaFactory, TestSyntheticMediaGenerator
from tests.test_timecode import TestSMPTETimecode
from tests.test_equalizer_dsp import TestEqualizerDSP
from tests.test_lrc_parser import TestLRCParser
from tests.test_mpris2_dbus import TestMPRIS2Compliance
from tests.test_persistence import TestPersistence
from tests.test_cli_desktop import TestCLIDesktop
from tests.test_tiers import (
    TestTier1CategoryPartition,
    TestTier2BoundaryAnalysis,
    TestTier3PairwiseCombinations,
    TestTier4RealWorldScenarios,
)
from tests.test_tier5_adversarial import TestTier5Adversarial


class BrutalistTestRunner:
    """Tactile Brutalist Test Runner & Structured Report Generator."""

    TEST_SUITES = [
        ("Synthetic Media Engine", [TestSyntheticMediaGenerator]),
        ("SMPTE Timecode Engine", [TestSMPTETimecode]),
        ("10-Band EQ & VU Meter DSP", [TestEqualizerDSP]),
        ("Synchronized LRC Parser", [TestLRCParser]),
        ("MPRIS2 D-Bus Compliance", [TestMPRIS2Compliance]),
        ("SQLite WAL Persistence", [TestPersistence]),
        ("CLI & Desktop Packaging", [TestCLIDesktop]),
        ("Tier 1: Category Partition", [TestTier1CategoryPartition]),
        ("Tier 2: Boundary Analysis", [TestTier2BoundaryAnalysis]),
        ("Tier 3: Pairwise Matrix", [TestTier3PairwiseCombinations]),
        ("Tier 4: Real-World Workloads", [TestTier4RealWorldScenarios]),
        ("Tier 5: Adversarial Hardening", [TestTier5Adversarial]),
    ]

    @classmethod
    def ensure_fixtures(cls, fixtures_dir: Path):
        required_files = [
            "test_video.mp4", "test_multitrack.mkv", "test_video.webm", "test_video.avi",
            "test_audio.mp3", "test_audio.flac", "test_audio.wav", "test_audio.aac", "test_audio.opus",
            "test_subtitles.srt", "test_subtitles.vtt", "test_subtitles.ass", "test_lyrics.lrc"
        ]
        missing = [f for f in required_files if not (fixtures_dir / f).exists()]
        if missing:
            print(f"[FIXTURES] Generating missing test media fixtures: {missing}...")
            SyntheticMediaFactory.create_all_fixtures(fixtures_dir)
            print("[FIXTURES] Synthetic test media generation complete.")

    @classmethod
    def run_all(cls) -> int:
        start_time = time.time()
        fixtures_dir = Path("tests/fixtures")
        cls.ensure_fixtures(fixtures_dir)

        # Set headless environment
        os.environ["QT_QPA_PLATFORM"] = "offscreen"

        total_ran = 0
        total_errors = 0
        total_failures = 0
        suite_results = []

        print("=" * 80)
        print("                PENGUIN HEADLESS E2E VERIFICATION HARNESS               ")
        print("               [TACTILE DIGITAL BRUTALISM // STUDIO PRECISION]          ")
        print("=" * 80)

        for suite_name, test_classes in cls.TEST_SUITES:
            suite = unittest.TestSuite()
            loader = unittest.TestLoader()
            for tc in test_classes:
                suite.addTests(loader.loadTestsFromTestCase(tc))

            count = suite.countTestCases()
            stream = io.StringIO()
            runner = unittest.TextTestRunner(stream=stream, verbosity=1)
            res = runner.run(suite)

            n_errors = len(res.errors)
            n_failures = len(res.failures)
            status = "PASS" if (n_errors == 0 and n_failures == 0) else "FAIL"

            total_ran += res.testsRun
            total_errors += n_errors
            total_failures += n_failures

            suite_results.append({
                "name": suite_name,
                "count": res.testsRun,
                "errors": n_errors,
                "failures": n_failures,
                "status": status,
            })

            status_color = "[PASS]" if status == "PASS" else "[FAIL]"
            dots = "." * (55 - len(suite_name) - len(str(res.testsRun)))
            print(f" {suite_name} ({res.testsRun} tests) {dots} {status_color}")

            if status == "FAIL":
                for failure in res.failures:
                    print(f"   [FAIL] {failure[0]}: {failure[1]}")
                for error in res.errors:
                    print(f"   [ERROR] {error[0]}: {error[1]}")

        elapsed_sec = time.time() - start_time
        print("=" * 80)

        overall_status = "PASS" if (total_errors == 0 and total_failures == 0) else "FAIL"
        print(f" TOTAL TEST CASES EXECUTED : {total_ran}")
        print(f" TOTAL FAILURES / ERRORS   : {total_failures} / {total_errors}")
        print(f" TOTAL ELAPSED TIME        : {elapsed_sec:.3f}s")
        print(f" VERIFICATION RESULT       : {overall_status} (EXIT {0 if overall_status == 'PASS' else 1})")
        print("=" * 80)

        # Emit structured JSON report
        report_data = {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "status": overall_status,
            "total_tests": total_ran,
            "total_failures": total_failures,
            "total_errors": total_errors,
            "elapsed_seconds": round(elapsed_sec, 3),
            "suites": suite_results,
        }
        report_path = Path("tests/test_report.json")
        report_path.write_text(json.dumps(report_data, indent=2), encoding="utf-8")
        print(f"[REPORT] Emitted structured test report to {report_path}")

        return 0 if overall_status == "PASS" else 1


if __name__ == "__main__":
    exit_code = BrutalistTestRunner.run_all()
    sys.exit(exit_code)
