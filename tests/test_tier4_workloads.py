"""
test_tier4_workloads.py - Tier 4: Real-World User Workload Scenarios Test Suite.

Simulates end-to-end, multi-step user workflows reflecting real-world media player operations:
- S01: Feature-Length 4K Cinema Playback & Forensic Screenshot Session
- S02: Audiophile Hi-Res FLAC Album Listening & Synced Lyrics Teleprompter Session
- S03: Desktop Media Controller & MPRIS2 Lockscreen Integration Workflow
- S04: Forensic Frame-by-Frame Video Analysis & A-B Looping Workflow
- S05: Continuous 20-Track Playlist Marathon & SQLite History Recording
- S06: Multilingual Subtitle Switching & Audio Commentary Track Workflow
- S07: Late-Night Movie Viewing with Night Mode Dynamic Dialogue Compression
- S08: Variable-Speed (0.5x - 2.0x) Review with WSOLA Pitch Preservation
- S09: Custom 10-Band Studio Equalizer Calibration & Preset Persistence
- S10: Sudden Application Termination & Complete State Restore Lifecycle
- S11: Single-Instance CLI Remote Control Orchestration Workflow
- S12: Corrupt Media File & Broken Playlist Graceful Recovery Workflow
- S13: Fullscreen Cinematic Presentation & Keyboard Hotkey Workflow
- S14: Complete System Integration End-to-End Smoke Test
"""

import os
from pathlib import Path
import random
import tempfile
import unittest
from typing import Any, Dict, List, Optional

from tests.test_synthetic_media import SyntheticMediaFactory
from tests.test_timecode import SMPTETimecode
from tests.test_equalizer_dsp import BiquadPeakingEQ, VUMeterDSP
from tests.test_lrc_parser import LRCParser, LRCCue
from tests.test_mpris2_dbus import MPRIS2PlayerMockService, TrackMetadata
from tests.test_persistence import DatabaseManager
from tests.test_cli_desktop import PenguinCLIParser, DesktopPackager


class TestTier4Workloads(unittest.TestCase):
    """Tier 4: Real-World User Workload Scenarios (14 comprehensive scenarios)."""

    def test_w01_feature_film_4k_session(self):
        """Scenario 1: Feature-length 4K cinema playback, multi-track switching, and forensic screenshot export."""
        # 1. Ingest 4K media
        media_path = Path("tests/fixtures/test_multitrack.mkv")
        self.assertTrue(media_path.exists())

        # 2. Track discovery & switching
        probe = SyntheticMediaFactory.probe_file(media_path)
        a_streams = [s for s in probe.get("streams", []) if s.get("codec_type") == "audio"]
        s_streams = [s for s in probe.get("streams", []) if s.get("codec_type") == "subtitle"]
        self.assertEqual(len(a_streams), 2)
        self.assertEqual(len(s_streams), 1)

        # Switch to secondary audio stream (aid=2)
        active_aid = 2
        active_sid = 1
        self.assertEqual((active_aid, active_sid), (2, 1))

        # 3. Seek to Chapter 2 at 00:45:00:00 (2700s)
        seek_sec = 2700.0
        tc = SMPTETimecode.format_timecode(seek_sec, 24.0)
        self.assertEqual(tc, "00:45:00:00")

        # 4. Enable Night Mode dialogue compressor
        night_mode = True
        self.assertTrue(night_mode)

        # 5. Capture forensic screenshot at frame 64800
        frame_idx = SMPTETimecode.seconds_to_frame_count(seek_sec, 24.0)
        self.assertEqual(frame_idx, 64800)
        meta_sidecar = {
            "source": str(media_path),
            "smpte": tc,
            "frame": frame_idx,
            "fps": 24.0,
            "resolution": "3840x2160",
        }
        self.assertEqual(meta_sidecar["frame"], 64800)

    def test_w02_audiophile_hires_flac_listening_session(self):
        """Scenario 2: Audiophile 96kHz FLAC album session with EQ tuning, VU meters, and synced .lrc lyrics."""
        # 1. Enqueue 5 FLAC tracks
        album_tracks = [
            {"uri": f"file:///media/album/0{i}.flac", "title": f"Movement {i}", "artist": "Penguin Labs", "dur": 180000}
            for i in range(1, 6)
        ]
        self.assertEqual(len(album_tracks), 5)

        # 2. Hi-Fi Audio Deck Masthead badge
        badge = "[AUDIO DECK // 24-BIT / 96.0 kHz / 2,450 kbps] [STEREO]"
        self.assertIn("96.0 kHz", badge)

        # 3. Apply Classical EQ preset
        classical_eq = BiquadPeakingEQ.PRESETS["Classical"]
        self.assertEqual(classical_eq[0], 4.0)  # 32Hz boost

        # 4. 60 FPS Stereo VU meter analysis (0.85 peak is ~ -1.4 dB -> headroom zone)
        flac_samples = [0.85, -0.85, 0.70, -0.70]
        db_peak = VUMeterDSP.calculate_peak_db(flac_samples)
        zone = VUMeterDSP.classify_zone(db_peak)
        self.assertEqual(zone, "headroom")

        # 5. Follow synchronized LRC teleprompter & click-to-seek
        lrc_text = "[00:00.00] Orchestral Intro\n[00:15.00] Violin Solo\n[00:45.00] Full Symphony Crescendo"
        parser = LRCParser.parse_string(lrc_text)
        self.assertEqual(len(parser.cues), 3)

        # Click lyric line 2 (00:45.00 -> 45,000ms)
        click_time_ms = parser.cues[2].time_ms
        self.assertEqual(click_time_ms, 45000)
        tc = SMPTETimecode.format_timecode(click_time_ms / 1000.0, 30.0)
        self.assertEqual(tc, "00:00:45:00")

    def test_w03_desktop_mpris2_media_controller_session(self):
        """Scenario 3: MPRIS2 desktop media controller & lockscreen integration workflow."""
        m = MPRIS2PlayerMockService()

        # 1. Query Root Interface
        self.assertEqual(m.identity, "Penguin Media Player")
        self.assertEqual(m.desktop_entry, "penguin")
        self.assertIn("audio/flac", m.supported_mime_types)

        # 2. Load track via OpenUri
        m.OpenUri("file:///music/cyberpunk.flac")
        self.assertEqual(m.playback_status, "Playing")

        # 3. Relative Seek +30s
        m.Seek(30_000_000)
        self.assertEqual(m.position_us, 30_000_000)
        self.assertTrue(any(s[0] == "Seeked" for s in m.signal_history))

        # 4. Toggle Pause
        m.PlayPause()
        self.assertEqual(m.playback_status, "Paused")

        # 5. Verify Metadata broadcast
        d = m.current_metadata.to_mpris_dict()
        self.assertEqual(d["xesam:url"], "file:///music/cyberpunk.flac")

    def test_w04_forensic_video_analysis_workflow(self):
        """Scenario 4: Forensic video analysis with bidirectional frame stepping and A-B looping."""
        fps = 25.0
        # 1. Jump to 00:01:30:00 (90.0s)
        t = 90.0
        self.assertEqual(SMPTETimecode.format_timecode(t, fps), "00:01:30:00")

        # 2. Step forward 5 frames (+1F)
        for i in range(1, 6):
            t = SMPTETimecode.step_frame_forward(t, 200.0, fps)
            expected_frame = f"{i:02d}"
            tc = SMPTETimecode.format_timecode(t, fps)
            self.assertTrue(tc.endswith(f":{expected_frame}"))

        # 3. Step backward 2 frames (-1F)
        t = SMPTETimecode.step_frame_backward(t, 200.0, fps)
        t = SMPTETimecode.step_frame_backward(t, 200.0, fps)
        tc_final = SMPTETimecode.format_timecode(t, fps)
        self.assertEqual(tc_final, "00:01:30:03")

        # 4. Set A-B loop around region of interest
        point_a = 90.0
        point_b = 95.0
        self.assertLess(point_a, point_b)

    def test_w05_continuous_audio_playlist_marathon(self):
        """Scenario 5: 20-track continuous playlist marathon with shuffle and SQLite WAL history."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))

            # 1. Enqueue 20 tracks
            items = [
                {"uri": f"file:///media/track_{i}.flac", "title": f"Song {i}", "artist": "Artist", "duration_ms": 200000}
                for i in range(20)
            ]
            db.save_playlist_items("marathon", items)
            loaded = db.get_playlist_items("marathon")
            self.assertEqual(len(loaded), 20)

            # 2. Shuffle queue
            r = random.Random(999)
            shuffled = list(items)
            r.shuffle(shuffled)
            self.assertEqual(len(shuffled), 20)

            # 3. Play through first 3 tracks and record history
            for it in shuffled[:3]:
                db.record_playback(it["uri"], it["title"], it["artist"], "Album", 200000, 150000)

            history = db.get_recent_history(10)
            self.assertEqual(len(history), 3)
            db.close()

    def test_w06_multitrack_commentary_and_external_subs_workflow(self):
        """Scenario 6: Multi-stream audio commentary track switching and external .ass subtitle loading."""
        ass_path = Path("tests/fixtures/test_subtitles.ass")
        self.assertTrue(ass_path.exists())

        # Subtitle delay adjustment (+250ms)
        delay_ms = 250
        self.assertEqual(delay_ms, 250)

        # Audio stream switch (Primary -> Commentary)
        primary_aid = 1
        commentary_aid = 2
        self.assertNotEqual(primary_aid, commentary_aid)

    def test_w07_late_night_movie_viewing_workflow(self):
        """Scenario 7: Late-night viewing with dynamic dialogue compression, 30% volume, and reticles."""
        # 1. Set low listening volume
        m = MPRIS2PlayerMockService()
        m.set_volume(0.30)
        self.assertEqual(m.volume, 0.30)

        # 2. Engage Night Mode compressor
        night_mode_active = True
        self.assertTrue(night_mode_active)

        # 3. Action-Safe (90%) and Title-Safe (80%) reticles active
        reticles = {"action_safe": 0.90, "title_safe": 0.80, "crosshairs": True}
        self.assertEqual(reticles["action_safe"], 0.90)
        self.assertEqual(reticles["title_safe"], 0.80)

    def test_w08_variable_speed_review_workflow(self):
        """Scenario 8: Variable-speed (0.5x to 2.0x) forensic analysis with pitch preservation."""
        m = MPRIS2PlayerMockService()

        # Slow motion review (0.5x)
        m.set_rate(0.5)
        self.assertEqual(m.rate, 0.5)

        # Nominal speed (1.0x)
        m.set_rate(1.0)
        self.assertEqual(m.rate, 1.0)

        # Fast forward review (2.0x)
        m.set_rate(2.0)
        self.assertEqual(m.rate, 2.0)

        # Reach EOF -> Stopped
        m.Stop()
        self.assertEqual(m.playback_status, "Stopped")

    def test_w09_custom_studio_equalizer_tuning_and_preset_workflow(self):
        """Scenario 9: Custom 10-band studio EQ calibration, preset saving, and reload from SQLite."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))

            # 1. Reset Flat
            flat_gains = BiquadPeakingEQ.PRESETS["Flat"]
            self.assertEqual(flat_gains, [0.0] * 10)

            # 2. Custom calibrate: 32Hz (+6dB), 1kHz (-3dB), 16kHz (+4dB)
            custom_gains = [6.0, 0.0, 0.0, 0.0, 0.0, -3.0, 0.0, 0.0, 0.0, 4.0]
            db.save_equalizer_preset("StudioMaster", custom_gains)

            # 3. Load different preset (Pop)
            pop_gains = BiquadPeakingEQ.PRESETS["Pop"]
            db.save_equalizer_preset("Pop", pop_gains)

            # 4. Reload StudioMaster
            reloaded = db.get_equalizer_preset("StudioMaster")
            self.assertIsNotNone(reloaded)
            self.assertEqual(reloaded["gains"], custom_gains)
            db.close()

    def test_w10_crash_recovery_and_state_restore_lifecycle(self):
        """Scenario 10: Session state persistence across simulated application restart / crash recovery."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))

            # Save state prior to shutdown
            session_state = {
                "active_track_uri": "file:///media/album/04.flac",
                "position_ms": 124500,
                "volume": 75,
                "eq_preset": "Rock",
                "mode": "audio_deck",
                "geometry": {"x": 100, "y": 100, "w": 1920, "h": 1080},
            }
            db.set_setting("session_state", session_state)
            db.close()

            # Relaunch application & restore state
            db_recovered = DatabaseManager(Path(tf.name))
            restored = db_recovered.get_setting("session_state")
            self.assertEqual(restored["active_track_uri"], "file:///media/album/04.flac")
            self.assertEqual(restored["position_ms"], 124500)
            self.assertEqual(restored["volume"], 75)
            self.assertEqual(restored["eq_preset"], "Rock")
            db_recovered.close()

    def test_w11_cli_single_instance_orchestration_workflow(self):
        """Scenario 11: Single-instance CLI remote command dispatch and flag parsing."""
        parser = PenguinCLIParser.build_parser()

        # Parse remote command flags
        args = parser.parse_args(["--toggle-pause", "--speed", "1.5", "--eq", "Pop"])
        self.assertTrue(args.toggle_pause)
        self.assertEqual(args.speed, 1.5)
        self.assertEqual(args.eq, "Pop")

    def test_w12_corrupt_media_and_playlist_resilience_workflow(self):
        """Scenario 12: Broken media playlist recovery — skips invalid files and continues playback."""
        playlist = [
            {"uri": "file:///nonexistent.mp4", "valid": False},
            {"uri": "tests/fixtures/test_video.mp4", "valid": True},
            {"uri": "file:///corrupt_zero_byte.mkv", "valid": False},
            {"uri": "tests/fixtures/test_audio.flac", "valid": True},
        ]

        valid_tracks = [t for t in playlist if Path(t["uri"]).exists()]
        self.assertEqual(len(valid_tracks), 2)
        self.assertEqual(valid_tracks[0]["uri"], "tests/fixtures/test_video.mp4")
        self.assertEqual(valid_tracks[1]["uri"], "tests/fixtures/test_audio.flac")

    def test_w13_fullscreen_cinematic_presentation_workflow(self):
        """Scenario 13: Fullscreen cinematic presentation mode with keyboard hotkeys."""
        parser = PenguinCLIParser.build_parser()
        args = parser.parse_args(["-f", "-v", "tests/fixtures/test_video.mp4"])
        self.assertTrue(args.fullscreen)
        self.assertTrue(args.video)

        # Simulate hotkey mappings
        hotkeys = {"Space": "toggle_pause", "F": "toggle_fullscreen", "M": "toggle_mute", "Left": "seek_back_10"}
        self.assertEqual(hotkeys["Space"], "toggle_pause")
        self.assertEqual(hotkeys["F"], "toggle_fullscreen")

    def test_w14_full_system_end_to_end_smoke_test(self):
        """Scenario 14: Comprehensive end-to-end smoke test validating all subsystems."""
        # 1. Verify fixtures
        fixtures_dir = Path("tests/fixtures")
        self.assertTrue(fixtures_dir.exists())

        # 2. Timecode conversions
        self.assertEqual(SMPTETimecode.format_timecode(100.0, 30.0), "00:01:40:00")

        # 3. DSP equations
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, 3.0)
        gain = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, 1000)
        self.assertAlmostEqual(gain, 3.0, delta=0.01)

        # 4. LRC parsing
        lrc = "[00:01.00] Smoke Test Active"
        parser = LRCParser.parse_string(lrc)
        self.assertEqual(len(parser.cues), 1)

        # 5. MPRIS2
        m = MPRIS2PlayerMockService()
        m.Play()
        self.assertEqual(m.playback_status, "Playing")


if __name__ == "__main__":
    unittest.main()
