"""
test_cli_desktop.py - CLI Arguments, Desktop Entry, and Icon Packaging Test Suite.

Tests command-line parsing and validation, FreeDesktop .desktop file syntax and
MIME type registrations, desktop action execution entries, and scalable SVG icon properties.
"""

import argparse
import configparser
import os
from pathlib import Path
import re
import tempfile
import unittest
import xml.etree.ElementTree as ET
from typing import List, Optional


class PenguinCLIParser:
    """Standard CLI argument parser for Penguin Media Player."""

    @classmethod
    def build_parser(cls) -> argparse.ArgumentParser:
        parser = argparse.ArgumentParser(
            prog="penguin",
            description="Tactile Digital Brutalist Audio & Video Player for Linux.",
        )
        parser.add_argument(
            "files",
            nargs="*",
            metavar="FILE_OR_URL",
            help="Media file paths, directories, or stream URLs to open and play.",
        )
        mode_group = parser.add_mutually_exclusive_group()
        mode_group.add_argument("-a", "--audio", action="store_true", help="Force Hi-Fi Audio Deck UI mode.")
        mode_group.add_argument("-v", "--video", action="store_true", help="Force Viewfinder Video UI mode.")

        parser.add_argument("-f", "--fullscreen", action="store_true", help="Launch application in fullscreen mode.")
        parser.add_argument("--volume", type=int, choices=range(0, 101), metavar="[0-100]", help="Set initial volume.")
        parser.add_argument("--speed", type=float, help="Set initial playback speed (0.5 to 2.0).")
        parser.add_argument("--sub", type=str, metavar="SUBTITLE_FILE", help="Attach external subtitle file (.srt/.ass/.vtt).")
        parser.add_argument("--loop", choices=["none", "track", "playlist"], default="none", help="Set playlist loop mode.")
        parser.add_argument("--shuffle", action="store_true", help="Enable random playlist shuffle.")
        parser.add_argument("--eq", type=str, metavar="PRESET", help="Apply equalizer preset by name.")

        # IPC Action flags
        ipc_group = parser.add_argument_group("IPC Remote Actions")
        ipc_group.add_argument("--toggle-pause", action="store_true", help="Send play/pause toggle to active instance.")
        ipc_group.add_argument("--next", action="store_true", help="Advance active instance to next track.")
        ipc_group.add_argument("--prev", action="store_true", help="Reverse active instance to previous track.")
        ipc_group.add_argument("--stop", action="store_true", help="Stop playback on active instance.")

        # Diagnostics & Verification
        parser.add_argument("--test", "--headless-test", dest="test_mode", action="store_true", help="Run automated test suite.")
        parser.add_argument("--dev-telemetry", action="store_true", help="Enable OSD telemetry HUD on launch.")
        parser.add_argument("-V", "--version", action="version", version="Penguin 1.0.0 (Tactile Digital Brutalism)")

        return parser


class DesktopPackager:
    """Helper to generate and validate desktop integration assets."""

    DESKTOP_ENTRY_TEMPLATE = """[Desktop Entry]
Version=1.5
Type=Application
Name=Penguin
GenericName=Media Player
Comment=Tactile Digital Brutalist Audio & Video Player
Exec=penguin %U
Icon=penguin
Terminal=false
StartupNotify=true
StartupWMClass=penguin
Categories=AudioVideo;Audio;Video;Player;Qt;
MimeType=audio/mpeg;audio/flac;audio/ogg;audio/opus;audio/x-wav;audio/aac;video/mp4;video/x-matroska;video/webm;video/quicktime;video/x-msvideo;
Actions=PlayPause;Next;Previous;Stop;

[Desktop Action PlayPause]
Name=Play/Pause
Exec=penguin --toggle-pause

[Desktop Action Next]
Name=Next Track
Exec=penguin --next

[Desktop Action Previous]
Name=Previous Track
Exec=penguin --prev

[Desktop Action Stop]
Name=Stop Playback
Exec=penguin --stop
"""

    SVG_ICON_TEMPLATE = """<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" width="512" height="512">
  <rect width="512" height="512" fill="#070709"/>
  <rect x="32" y="32" width="448" height="448" fill="#0B0B0E" stroke="#1E1E24" stroke-width="4"/>
  <!-- Brutalist Penguin Silhouette with Safety Orange Accent -->
  <polygon points="256,96 160,320 200,416 312,416 352,320" fill="#FFFFFF"/>
  <polygon points="256,120 280,180 232,180" fill="#FF4400"/>
  <circle cx="256" cy="240" r="16" fill="#CCFF00"/>
  <!-- Technical Reticle Overlay -->
  <line x1="128" y1="256" x2="384" y2="256" stroke="#00E5FF" stroke-width="2" stroke-dasharray="8,8"/>
  <line x1="256" y1="128" x2="256" y2="384" stroke="#00E5FF" stroke-width="2" stroke-dasharray="8,8"/>
</svg>"""

    @classmethod
    def write_desktop_file(cls, path: Path):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(cls.DESKTOP_ENTRY_TEMPLATE, encoding="utf-8")

    @classmethod
    def write_svg_icon(cls, path: Path):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(cls.SVG_ICON_TEMPLATE, encoding="utf-8")


class TestCLIDesktop(unittest.TestCase):
    """Test suite verifying CLI options, .desktop spec compliance, and SVG icon packaging."""

    def setUp(self):
        self.parser = PenguinCLIParser.build_parser()
        self.temp_dir = tempfile.TemporaryDirectory(prefix="penguin_pkg_test_")
        self.desktop_file = Path(self.temp_dir.name) / "penguin.desktop"
        self.svg_file = Path(self.temp_dir.name) / "penguin.svg"
        DesktopPackager.write_desktop_file(self.desktop_file)
        DesktopPackager.write_svg_icon(self.svg_file)

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_cli_positional_files(self):
        args = self.parser.parse_args(["movie1.mp4", "movie2.mkv", "song.flac"])
        self.assertEqual(args.files, ["movie1.mp4", "movie2.mkv", "song.flac"])

    def test_cli_ui_modes_and_fullscreen(self):
        args_audio = self.parser.parse_args(["--audio", "-f"])
        self.assertTrue(args_audio.audio)
        self.assertFalse(args_audio.video)
        self.assertTrue(args_audio.fullscreen)

        args_video = self.parser.parse_args(["-v"])
        self.assertTrue(args_video.video)
        self.assertFalse(args_video.audio)

    def test_cli_volume_and_speed(self):
        args = self.parser.parse_args(["--volume", "85", "--speed", "1.5"])
        self.assertEqual(args.volume, 85)
        self.assertAlmostEqual(args.speed, 1.5)

    def test_cli_ipc_flags(self):
        args = self.parser.parse_args(["--toggle-pause"])
        self.assertTrue(args.toggle_pause)

        args_next = self.parser.parse_args(["--next"])
        self.assertTrue(args_next.next)

    def test_desktop_file_syntax_and_sections(self):
        self.assertTrue(self.desktop_file.exists())
        config = configparser.ConfigParser(interpolation=None)
        config.read(str(self.desktop_file), encoding="utf-8")

        self.assertIn("Desktop Entry", config.sections())
        entry = config["Desktop Entry"]
        self.assertEqual(entry.get("Type"), "Application")
        self.assertEqual(entry.get("Name"), "Penguin")
        self.assertEqual(entry.get("Exec"), "penguin %U")
        self.assertEqual(entry.get("Icon"), "penguin")
        self.assertIn("AudioVideo;", entry.get("Categories"))
        self.assertIn("video/mp4;", entry.get("MimeType"))
        self.assertIn("audio/flac;", entry.get("MimeType"))

        # Verify Desktop Actions
        self.assertIn("Desktop Action PlayPause", config.sections())
        self.assertIn("Desktop Action Next", config.sections())
        self.assertIn("Desktop Action Previous", config.sections())
        self.assertIn("Desktop Action Stop", config.sections())

    def test_svg_icon_well_formed_xml_and_branding(self):
        self.assertTrue(self.svg_file.exists())
        content = self.svg_file.read_text(encoding="utf-8")

        # Parse XML
        root = ET.fromstring(content)
        self.assertEqual(root.tag, "{http://www.w3.org/2000/svg}svg")
        self.assertEqual(root.attrib.get("viewBox"), "0 0 512 512")

        # Check Brutalist Brand Colors
        self.assertIn("#070709", content)  # Deep obsidian
        self.assertIn("#FF4400", content)  # Safety orange accent
        self.assertIn("#CCFF00", content)  # Signal lime accent
        self.assertIn("#00E5FF", content)  # Telemetry cyan


if __name__ == "__main__":
    unittest.main()
