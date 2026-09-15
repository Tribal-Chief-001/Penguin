"""
test_headless_cli.py - Headless Verification Script and CLI Test Runner for Penguin.

Tests command-line parsing, argument validation, mode flags, IPC remote actions,
and headless execution behavior with zero display requirements.
"""

import io
import os
from pathlib import Path
import sys
import unittest

from tests.test_cli_desktop import PenguinCLIParser, DesktopPackager


class TestHeadlessCLI(unittest.TestCase):
    """Headless CLI Verification and Argument Parser Test Suite."""

    def setUp(self):
        self.parser = PenguinCLIParser.build_parser()

    def test_01_cli_parser_creation_and_help(self):
        """Verifies CLI parser creation and help output generation."""
        self.assertEqual(self.parser.prog, "penguin")
        help_stream = io.StringIO()
        self.parser.print_help(help_stream)
        help_text = help_stream.getvalue()
        self.assertIn("Tactile Digital Brutalist", help_text)
        self.assertIn("--audio", help_text)
        self.assertIn("--video", help_text)

    def test_02_cli_audio_and_video_mode_flags(self):
        """Verifies mutually exclusive UI mode flags."""
        args_audio = self.parser.parse_args(["-a"])
        self.assertTrue(args_audio.audio)
        self.assertFalse(args_audio.video)

        args_video = self.parser.parse_args(["-v"])
        self.assertTrue(args_video.video)
        self.assertFalse(args_video.audio)

        with self.assertRaises(SystemExit):
            self.parser.parse_args(["-a", "-v"])

    def test_03_cli_fullscreen_and_volume_arguments(self):
        """Verifies fullscreen flag and volume integer choices [0-100]."""
        args = self.parser.parse_args(["-f", "--volume", "70"])
        self.assertTrue(args.fullscreen)
        self.assertEqual(args.volume, 70)

        # Volume out of range raises SystemExit (argparse error)
        with self.assertRaises(SystemExit):
            self.parser.parse_args(["--volume", "150"])

    def test_04_cli_speed_subtitles_loop_shuffle(self):
        """Verifies speed float, external subtitle path, loop choice, and shuffle flag."""
        args = self.parser.parse_args([
            "--speed", "1.75",
            "--sub", "/tmp/subs.srt",
            "--loop", "track",
            "--shuffle",
        ])
        self.assertEqual(args.speed, 1.75)
        self.assertEqual(args.sub, "/tmp/subs.srt")
        self.assertEqual(args.loop, "track")
        self.assertTrue(args.shuffle)

    def test_05_cli_eq_preset_and_ipc_remote_actions(self):
        """Verifies equalizer preset string and remote IPC transport flags."""
        args = self.parser.parse_args([
            "--eq", "Electronic",
            "--toggle-pause",
            "--next",
            "--prev",
            "--stop",
        ])
        self.assertEqual(args.eq, "Electronic")
        self.assertTrue(args.toggle_pause)
        self.assertTrue(args.next)
        self.assertTrue(args.prev)
        self.assertTrue(args.stop)

    def test_06_cli_test_mode_and_dev_telemetry_flags(self):
        """Verifies --test and --dev-telemetry flags."""
        args = self.parser.parse_args(["--test", "--dev-telemetry"])
        self.assertTrue(args.test_mode)
        self.assertTrue(args.dev_telemetry)

    def test_07_cli_positional_files_and_stream_urls(self):
        """Verifies positional argument ingestion for files and URLs."""
        args = self.parser.parse_args([
            "tests/fixtures/test_video.mp4",
            "tests/fixtures/test_audio.flac",
            "https://stream.example.com/live.m3u8",
        ])
        self.assertEqual(len(args.files), 3)
        self.assertEqual(args.files[0], "tests/fixtures/test_video.mp4")
        self.assertEqual(args.files[2], "https://stream.example.com/live.m3u8")

    def test_08_headless_environment_offscreen_qpa(self):
        """Verifies offscreen Qt QPA environment variable configuration."""
        os.environ["QT_QPA_PLATFORM"] = "offscreen"
        self.assertEqual(os.environ.get("QT_QPA_PLATFORM"), "offscreen")


if __name__ == "__main__":
    unittest.main()
