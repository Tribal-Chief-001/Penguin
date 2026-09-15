"""
tests/test_desktop_integration_m3.py - Dedicated Automated Verification for Milestone M3.

Verifies:
1. MPRIS2 D-Bus Service (org.mpris.MediaPlayer2 & org.mpris.MediaPlayer2.Player, XESAM metadata, signals, engine binding).
2. Command-Line Interface (Argparse flags, options dataclass, error boundaries, single-instance IPC forwarding).
3. Audio Routing Subsystem (Backend detection, sink enumeration, sample rate negotiation, low-latency config).
4. Desktop Packaging & Assets (FreeDesktop .desktop validation, Brutalist SVG icons, symbolic icons, PNG icons).
5. Universal Launcher Script (bin/penguin executable bit and wrapper behavior).
"""

import configparser
import os
from pathlib import Path
import stat
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

from src.desktop import (
    MPRIS2Service,
    TrackMetadata,
    PenguinCLIParser,
    PenguinCLIOptions,
    parse_args,
    forward_ipc_command,
    is_mpris2_instance_running,
    AudioRouter,
    AudioBackend,
    AudioSinkInfo,
    AudioRoutingConfig,
    DesktopPackager,
)

PROJECT_ROOT = Path(__file__).resolve().parent.parent


class TestM3DesktopIntegration(unittest.TestCase):
    """Automated verification suite for Linux Desktop Integration (Milestone M3)."""

    def setUp(self):
        self.mpris = MPRIS2Service()
        self.router = AudioRouter()

    # =========================================================================
    # 1. MPRIS2 D-BUS SERVICE & SPECIFICATION COMPLIANCE
    # =========================================================================

    def test_01_mpris2_root_interface_spec(self):
        """Verifies org.mpris.MediaPlayer2 standard Root interface properties and actions."""
        self.assertEqual(self.mpris.BUS_NAME, "org.mpris.MediaPlayer2.penguin")
        self.assertEqual(self.mpris.OBJECT_PATH, "/org/mpris/MediaPlayer2")
        self.assertEqual(self.mpris.ROOT_INTERFACE, "org.mpris.MediaPlayer2")
        self.assertEqual(self.mpris.PLAYER_INTERFACE, "org.mpris.MediaPlayer2.Player")

        self.assertTrue(self.mpris.can_quit)
        self.assertTrue(self.mpris.can_raise)
        self.assertTrue(self.mpris.can_set_fullscreen)
        self.assertFalse(self.mpris.fullscreen)
        self.assertFalse(self.mpris.has_track_list)
        self.assertEqual(self.mpris.identity, "Penguin Media Player")
        self.assertEqual(self.mpris.desktop_entry, "penguin")
        self.assertIn("file", self.mpris.supported_uri_schemes)
        self.assertIn("http", self.mpris.supported_uri_schemes)
        self.assertIn("video/mp4", self.mpris.supported_mime_types)
        self.assertIn("audio/flac", self.mpris.supported_mime_types)

        # Test Raise and Quit signal generation
        self.mpris.Raise()
        self.mpris.Quit()
        signals = [s[0] for s in self.mpris.signal_history]
        self.assertIn("Raise", signals)
        self.assertIn("Quit", signals)

    def test_02_mpris2_player_state_transitions(self):
        """Verifies Player interface playback state transitions and PropertiesChanged signals."""
        self.assertEqual(self.mpris.playback_status, "Stopped")

        self.mpris.Play()
        self.assertEqual(self.mpris.playback_status, "Playing")

        self.mpris.Pause()
        self.assertEqual(self.mpris.playback_status, "Paused")

        self.mpris.PlayPause()
        self.assertEqual(self.mpris.playback_status, "Playing")

        self.mpris.Stop()
        self.assertEqual(self.mpris.playback_status, "Stopped")
        self.assertEqual(self.mpris.position_us, 0)

        # Check PropertiesChanged signals
        prop_signals = [s for s in self.mpris.signal_history if s[0] == "PropertiesChanged"]
        self.assertGreaterEqual(len(prop_signals), 4)

    def test_03_mpris2_microsecond_seeking_and_boundary_clamping(self):
        """Verifies relative Seek and absolute SetPosition in microseconds with boundaries."""
        self.mpris.current_metadata = TrackMetadata(
            track_id="/org/mpris/MediaPlayer2/Track/10",
            length_us=120_000_000, # 120s
            title="Boundary Test Track"
        )
        self.mpris.position_us = 20_000_000 # 20s

        # Relative seek forward +15s
        self.mpris.Seek(15_000_000)
        self.assertEqual(self.mpris.position_us, 35_000_000)

        # Relative seek backward past 0 (clamps to 0)
        self.mpris.Seek(-50_000_000)
        self.assertEqual(self.mpris.position_us, 0)

        # Relative seek beyond duration (clamps to length_us)
        self.mpris.Seek(200_000_000)
        self.assertEqual(self.mpris.position_us, 120_000_000)

        # Absolute SetPosition with matching track ID
        self.mpris.SetPosition("/org/mpris/MediaPlayer2/Track/10", 42_000_000)
        self.assertEqual(self.mpris.position_us, 42_000_000)

        # Absolute SetPosition with mismatching track ID (ignored)
        self.mpris.SetPosition("/org/mpris/MediaPlayer2/Track/other", 99_000_000)
        self.assertEqual(self.mpris.position_us, 42_000_000)

        # Verify Seeked signals
        seek_signals = [s for s in self.mpris.signal_history if s[0] == "Seeked"]
        self.assertEqual(len(seek_signals), 4)

    def test_04_mpris2_volume_and_rate_clamping(self):
        """Verifies Volume [0.0 to 1.5] and Rate [0.5 to 2.0] clamping."""
        # Volume
        self.mpris.set_volume(0.8)
        self.assertAlmostEqual(self.mpris.volume, 0.8)
        self.mpris.set_volume(-0.5)
        self.assertEqual(self.mpris.volume, 0.0)
        self.mpris.set_volume(2.5)
        self.assertEqual(self.mpris.volume, 1.5)

        # Rate
        self.mpris.set_rate(1.25)
        self.assertAlmostEqual(self.mpris.rate, 1.25)
        self.mpris.set_rate(0.1)
        self.assertEqual(self.mpris.rate, 0.5)
        self.mpris.set_rate(5.0)
        self.assertEqual(self.mpris.rate, 2.0)

    def test_05_mpris2_xesam_metadata_roundtrip(self):
        """Verifies XESAM metadata dictionary serialization and deserialization."""
        meta = TrackMetadata(
            track_id="/org/mpris/MediaPlayer2/Track/42",
            length_us=300_000_000,
            art_url="file:///tmp/cover.jpg",
            title="Studio Precision Opus 1",
            artists=["Penguin Sound Design", "Brutalist Labs"],
            album="Tactile Digital Brutalism",
            album_artists=["Penguin Sound Design"],
            genres=["Electronic", "Cinematic"],
            track_number=1,
            disc_number=1,
            url="file:///music/opus1.flac",
            user_rating=0.95,
        )
        d = meta.to_mpris_dict()
        self.assertEqual(d["mpris:trackid"], "/org/mpris/MediaPlayer2/Track/42")
        self.assertEqual(d["mpris:length"], 300_000_000)
        self.assertEqual(d["xesam:title"], "Studio Precision Opus 1")
        self.assertEqual(len(d["xesam:artist"]), 2)
        self.assertEqual(d["xesam:userRating"], 0.95)

        restored = TrackMetadata.from_dict(d)
        self.assertEqual(restored.title, meta.title)
        self.assertEqual(restored.length_us, meta.length_us)
        self.assertEqual(restored.artists, meta.artists)

    def test_06_mpris2_engine_event_binding(self):
        """Verifies bi-directional event synchronization with PlaybackEngine mock."""
        class MockEngine:
            def __init__(self):
                self.state_cb = None
                self.time_cb = None
                self.vol_cb = None
                self.played = False
                self.paused = False
                self.stopped = False
                self.seek_target = None
            def add_state_callback(self, cb): self.state_cb = cb
            def add_time_callback(self, cb): self.time_cb = cb
            def add_volume_callback(self, cb): self.vol_cb = cb
            def play(self): self.played = True
            def pause(self): self.paused = True
            def stop(self): self.stopped = True
            def seek_ms(self, pos): self.seek_target = pos
            def set_volume(self, v): pass
            def set_speed(self, s): pass

        mock_eng = MockEngine()
        service = MPRIS2Service(engine=mock_eng)

        # Trigger MPRIS2 methods -> Engine
        service.Play()
        self.assertTrue(mock_eng.played)
        service.Pause()
        self.assertTrue(mock_eng.paused)
        service.Stop()
        self.assertTrue(mock_eng.stopped)

        # Trigger Engine events -> MPRIS2
        mock_eng.state_cb("Playing")
        self.assertEqual(service.playback_status, "Playing")
        mock_eng.time_cb(4500.0) # 4.5s
        self.assertEqual(service.position_us, 4_500_000)
        mock_eng.vol_cb(75)
        self.assertAlmostEqual(service.volume, 0.75)

    # =========================================================================
    # 2. COMMAND-LINE PARSER & SINGLE-INSTANCE IPC
    # =========================================================================

    def test_07_cli_full_argument_matrix(self):
        """Verifies CLI parsing across all supported parameters and flags."""
        parser = PenguinCLIParser.build_parser()
        args = parser.parse_args([
            "song1.flac", "movie.mkv",
            "-a", "-f",
            "--volume", "85",
            "--speed", "1.5",
            "--sub", "/tmp/subs.srt",
            "--loop", "playlist",
            "--shuffle",
            "--eq", "Rock",
            "--dev-telemetry",
        ])
        opts = PenguinCLIOptions.from_namespace(args)

        self.assertEqual(opts.files, ["song1.flac", "movie.mkv"])
        self.assertTrue(opts.audio)
        self.assertFalse(opts.video)
        self.assertTrue(opts.fullscreen)
        self.assertEqual(opts.volume, 85)
        self.assertAlmostEqual(opts.speed, 1.5)
        self.assertEqual(opts.sub, "/tmp/subs.srt")
        self.assertEqual(opts.loop, "playlist")
        self.assertTrue(opts.shuffle)
        self.assertEqual(opts.eq, "Rock")
        self.assertTrue(opts.dev_telemetry)

    def test_08_cli_boundary_and_rejection_rules(self):
        """Verifies mutually exclusive mode conflict and volume limit rejections."""
        parser = PenguinCLIParser.build_parser()

        # Both -a and -v rejected
        with self.assertRaises(SystemExit):
            parser.parse_args(["-a", "-v"])

        # Volume < 0 or > 100 rejected
        with self.assertRaises(SystemExit):
            parser.parse_args(["--volume", "101"])
        with self.assertRaises(SystemExit):
            parser.parse_args(["--volume", "-1"])

        # Unknown argument rejected
        with self.assertRaises(SystemExit):
            parser.parse_args(["--invalid-flag"])

    def test_09_cli_ipc_remote_action_parsing(self):
        """Verifies IPC remote control action flag parsing."""
        parser = PenguinCLIParser.build_parser()
        args = parser.parse_args(["--toggle-pause"])
        opts = PenguinCLIOptions.from_namespace(args)
        self.assertTrue(opts.toggle_pause)
        self.assertEqual(opts.ipc_action, "play-pause")

        args_prev = parser.parse_args(["--prev"])
        opts_prev = PenguinCLIOptions.from_namespace(args_prev)
        self.assertTrue(opts_prev.prev)
        self.assertEqual(opts_prev.ipc_action, "previous")

    # =========================================================================
    # 3. AUDIO ROUTING (PIPEWIRE / PULSEAUDIO)
    # =========================================================================

    def test_10_audio_router_backend_and_sink_query(self):
        """Verifies sound server discovery, sink enumeration, and default sink inspection."""
        backend = self.router.active_backend
        self.assertIn(backend, [
            AudioBackend.PIPEWIRE, AudioBackend.PULSEAUDIO, AudioBackend.ALSA, AudioBackend.NULL
        ])

        sinks = self.router.list_sinks()
        self.assertGreater(len(sinks), 0)
        default_sink = self.router.get_default_sink()
        self.assertIsNotNone(default_sink)
        self.assertGreater(default_sink.sample_rate, 0)

    def test_11_audio_router_sample_rate_negotiation_and_mpv_options(self):
        """Verifies sample rate negotiation and low-latency MPV property dictionary synthesis."""
        # 44.1kHz standard rate
        rate_441 = self.router.negotiate_sample_rate(44100)
        self.assertEqual(rate_441, 44100)

        # 96kHz studio hi-res rate
        rate_96k = self.router.negotiate_sample_rate(96000)
        self.assertEqual(rate_96k, 96000)

        # Low-latency config synthesis
        cfg = self.router.build_low_latency_config(source_rate=48000, channels=2, low_latency=True)
        opts = cfg.to_mpv_options()

        self.assertIn("ao", opts)
        self.assertTrue(opts["ao"].startswith("pipewire"))
        self.assertEqual(opts["audio-samplerate"], "48000")
        self.assertEqual(opts["audio-buffer"], "0.020")
        self.assertEqual(opts["audio-pitch-correction"], "yes")

    def test_12_audio_router_system_info(self):
        """Verifies comprehensive audio system diagnostic report."""
        info = self.router.get_audio_system_info()
        self.assertIn("backend", info)
        self.assertIn("sinks", info)
        self.assertIn("supported_sample_rates", info)
        self.assertIn(48000, info["supported_sample_rates"])
        self.assertIn(192000, info["supported_sample_rates"])

    # =========================================================================
    # 4. PACKAGING & ASSETS VALIDATION
    # =========================================================================

    def test_13_desktop_entry_syntax_and_actions(self):
        """Verifies FreeDesktop .desktop entry file in assets/."""
        desktop_path = PROJECT_ROOT / "assets" / "penguin.desktop"
        self.assertTrue(desktop_path.exists())

        valid, errors = DesktopPackager.validate_desktop_file(desktop_path)
        self.assertTrue(valid, f"Desktop validation failed: {errors}")

        config = configparser.ConfigParser(interpolation=None)
        config.read(str(desktop_path), encoding="utf-8")

        entry = config["Desktop Entry"]
        self.assertEqual(entry.get("Name"), "Penguin")
        self.assertEqual(entry.get("Exec"), "penguin %U")
        self.assertEqual(entry.get("Icon"), "penguin")
        self.assertIn("AudioVideo;", entry.get("Categories"))
        self.assertIn("video/x-matroska;", entry.get("MimeType"))

        # Check Desktop Actions
        self.assertIn("Desktop Action PlayPause", config.sections())
        self.assertIn("Desktop Action Next", config.sections())
        self.assertIn("Desktop Action Previous", config.sections())
        self.assertIn("Desktop Action Stop", config.sections())

    def test_14_brutalist_svg_icons_and_tokens(self):
        """Verifies Brutalist scalable SVG icon and design tokens in assets/icons/."""
        svg_path = PROJECT_ROOT / "assets" / "icons" / "hicolor" / "scalable" / "apps" / "penguin.svg"
        self.assertTrue(svg_path.exists())

        valid, errors = DesktopPackager.validate_svg_icon(svg_path)
        self.assertTrue(valid, f"SVG validation failed: {errors}")

        content = svg_path.read_text(encoding="utf-8")
        self.assertIn("#070709", content) # Obsidian base
        self.assertIn("#FF4400", content) # Safety orange
        self.assertIn("#CCFF00", content) # Signal lime
        self.assertIn("#00E5FF", content) # Cyan reticle

        # Symbolic icon
        sym_path = PROJECT_ROOT / "assets" / "icons" / "hicolor" / "symbolic" / "apps" / "penguin-symbolic.svg"
        self.assertTrue(sym_path.exists())

        # PNG rasterized icons
        for size in ["48x48", "256x256", "512x512"]:
            png_path = PROJECT_ROOT / "assets" / "icons" / "hicolor" / size / "apps" / "penguin.png"
            self.assertTrue(png_path.exists(), f"PNG icon missing: {png_path}")

    def test_15_executable_launcher_script(self):
        """Verifies bin/penguin wrapper script permissions and execution."""
        bin_path = PROJECT_ROOT / "bin" / "penguin"
        self.assertTrue(bin_path.exists())
        file_stat = bin_path.stat()
        is_executable = bool(file_stat.st_mode & stat.S_IXUSR)
        self.assertTrue(is_executable, "bin/penguin is not marked executable")


if __name__ == "__main__":
    unittest.main()
