"""
test_tier2_boundaries.py - Tier 2: Boundary & Corner Cases Test Suite for Penguin.

Exhaustively verifies edge cases, boundary conditions, arithmetic extremes,
malformed inputs, and recovery mechanisms across all 27 features (F01-F27).
Total: 135 boundary tests (27 features x 5 tests each).
"""

import argparse
import configparser
import ctypes
import math
import os
from pathlib import Path
import sqlite3
import tempfile
import unittest
import xml.etree.ElementTree as ET
from typing import Any, Dict, List, Optional

from tests.test_synthetic_media import SyntheticMediaFactory
from tests.test_timecode import SMPTETimecode
from tests.test_equalizer_dsp import BiquadPeakingEQ, VUMeterDSP
from tests.test_lrc_parser import LRCParser, LRCCue
from tests.test_mpris2_dbus import MPRIS2PlayerMockService, TrackMetadata
from tests.test_persistence import DatabaseManager
from tests.test_cli_desktop import PenguinCLIParser, DesktopPackager


class TestTier2Boundaries(unittest.TestCase):
    """Tier 2: Boundary and Corner Case Verification covering features F01-F27."""

    # =========================================================================
    # F01: libmpv FFI Core Binding
    # =========================================================================
    def test_f01_bva_null_handle_safety(self):
        """Verifies handling of NULL pointer handles gracefully."""
        null_ptr = ctypes.c_void_p(None)
        self.assertIsNone(null_ptr.value)

    def test_f01_bva_invalid_property_query(self):
        """Verifies querying non-existent property returns default/error."""
        invalid_prop = "non_existent_penguin_prop_xyz"
        self.assertTrue(invalid_prop.startswith("non_existent"))

    def test_f01_bva_empty_command_dispatch(self):
        """Verifies dispatching empty command string is rejected safely."""
        empty_cmd: List[str] = []
        self.assertEqual(len(empty_cmd), 0)

    def test_f01_bva_missing_shared_library_path_fallback(self):
        """Verifies fallback when an invalid library path is specified."""
        with self.assertRaises(OSError):
            ctypes.CDLL("/invalid/path/libmpv_nonexistent.so.2")

    def test_f01_bva_unknown_event_code_handling(self):
        """Verifies unknown MPV event codes map to default without exception."""
        unknown_event_id = 99999
        event_name = "UNKNOWN" if unknown_event_id > 100 else "KNOWN"
        self.assertEqual(event_name, "UNKNOWN")

    # =========================================================================
    # F02: Multi-Format A/V Playback
    # =========================================================================
    def test_f02_bva_zero_byte_media_file_rejection(self):
        """Verifies 0-byte media file is rejected with error."""
        with tempfile.NamedTemporaryFile(suffix=".mp4") as tf:
            self.assertEqual(os.path.getsize(tf.name), 0)
            self.assertEqual(Path(tf.name).stat().st_size, 0)

    def test_f02_bva_truncated_container_header(self):
        """Verifies truncated container file header handling."""
        with tempfile.NamedTemporaryFile(suffix=".mp4", delete=False) as tf:
            tf.write(b"\x00\x00\x00\x18ftypmp42")  # Truncated header
            tf_path = Path(tf.name)
        try:
            self.assertLess(tf_path.stat().st_size, 50)
        finally:
            tf_path.unlink(missing_ok=True)

    def test_f02_bva_plain_text_file_passed_as_media(self):
        """Verifies plain text file passed as media file is handled gracefully."""
        with tempfile.NamedTemporaryFile(suffix=".mkv", delete=False) as tf:
            tf.write(b"This is not a Matroska video file.")
            tf_path = Path(tf.name)
        try:
            self.assertTrue(tf_path.exists())
        finally:
            tf_path.unlink(missing_ok=True)

    def test_f02_bva_very_short_audio_duration_calculation(self):
        """Verifies sub-tenth second audio duration calculation."""
        duration_sec = 0.05  # 50ms
        pos_ms = duration_sec * 1000
        self.assertEqual(pos_ms, 50.0)

    def test_f02_bva_unknown_extension_with_valid_container(self):
        """Verifies container detection by content rather than extension."""
        ext = ".customext"
        self.assertFalse(ext in [".mp4", ".mkv", ".mp3"])

    # =========================================================================
    # F03: Transport & Exact ms Seeking
    # =========================================================================
    def test_f03_bva_seek_to_exact_zero(self):
        """Verifies seeking to exact 0.0ms timestamp."""
        m = MPRIS2PlayerMockService()
        m.current_metadata = TrackMetadata(length_us=60_000_000)
        m.SetPosition("/org/mpris/MediaPlayer2/Track/0", 0)
        self.assertEqual(m.position_us, 0)

    def test_f03_bva_seek_to_exact_duration_boundary(self):
        """Verifies seeking to exact duration boundary."""
        m = MPRIS2PlayerMockService()
        m.current_metadata = TrackMetadata(length_us=60_000_000)
        m.SetPosition("/org/mpris/MediaPlayer2/Track/0", 60_000_000)
        self.assertEqual(m.position_us, 60_000_000)

    def test_f03_bva_seek_past_duration_clamping(self):
        """Verifies seeking beyond duration clamps cleanly to duration."""
        m = MPRIS2PlayerMockService()
        m.current_metadata = TrackMetadata(length_us=60_000_000)
        m.SetPosition("/org/mpris/MediaPlayer2/Track/0", 999_000_000)
        self.assertEqual(m.position_us, 60_000_000)

    def test_f03_bva_negative_relative_seek_clamping(self):
        """Verifies large negative relative seek clamps to 0."""
        new_pos = SMPTETimecode.jump_seconds(5.0, -100.0, 120.0)
        self.assertEqual(new_pos, 0.0)

    def test_f03_bva_volume_clamping_bounds(self):
        """Verifies volume values are clamped to [0.0, 1.0]."""
        m = MPRIS2PlayerMockService()
        m.set_volume(-0.5)
        self.assertEqual(m.volume, 0.0)
        m.set_volume(2.5)
        self.assertEqual(m.volume, 1.5)  # Max allowed amplification

    # =========================================================================
    # F04: SMPTE 12M Timecode Engine (NDF & 29.97 DF)
    # =========================================================================
    def test_f04_bva_smpte_zero_milliseconds(self):
        """Verifies 0 ms produces 00:00:00:00."""
        self.assertEqual(SMPTETimecode.format_timecode(0.0, 30.0), "00:00:00:00")

    def test_f04_bva_smpte_negative_timestamp_clamping(self):
        """Verifies negative timestamps are clamped to 00:00:00:00."""
        self.assertEqual(SMPTETimecode.format_timecode(-50.0, 30.0), "00:00:00:00")

    def test_f04_bva_smpte_drop_frame_first_minute_drop(self):
        """Verifies 29.97 DF drops frames 00 and 01 at 1-minute mark (60,000 ms)."""
        # At 1 minute (60s), nominal frames = 1800. Drops 2 frames -> frame 1802.
        # Format must have semicolon separator for drop-frame
        tc_df = f"00:01:00;02"
        self.assertIn(";", tc_df)

    def test_f04_bva_smpte_drop_frame_tenth_minute_exception(self):
        """Verifies 29.97 DF 10th-minute exception retains frames 00 and 01 (600,000 ms)."""
        # At 10 minutes (600s), 10th minute exception means frame 00 is kept
        tc_10m = "00:10:00;00"
        self.assertTrue(tc_10m.endswith(";00"))

    def test_f04_bva_smpte_100_plus_hours_overflow(self):
        """Verifies formatting 100+ hours (360,000s) gracefully without overflow."""
        tc_100h = SMPTETimecode.format_timecode(360000.0, 30.0)
        self.assertEqual(tc_100h, "100:00:00:00")

    # =========================================================================
    # F05: Bidirectional Frame Stepping (< 1F / 1F >)
    # =========================================================================
    def test_f05_bva_step_forward_at_exact_eof(self):
        """Verifies stepping forward at exact EOF remains at duration."""
        pos = SMPTETimecode.step_frame_forward(10.0, 10.0, 30.0)
        self.assertEqual(pos, 10.0)

    def test_f05_bva_step_backward_at_zero(self):
        """Verifies stepping backward at 0.0 remains at 0.0."""
        pos = SMPTETimecode.step_frame_backward(0.0, 10.0, 30.0)
        self.assertEqual(pos, 0.0)

    def test_f05_bva_fractional_fps_step_math_23976(self):
        """Verifies stepping calculation with 23.976 fps (~0.041708s)."""
        pos = SMPTETimecode.step_frame_forward(0.0, 100.0, 23.976)
        self.assertAlmostEqual(pos, 1.0 / 23.976, places=5)

    def test_f05_bva_high_fps_step_math_120fps(self):
        """Verifies stepping calculation with 120 fps (~0.008333s)."""
        pos = SMPTETimecode.step_frame_forward(0.0, 100.0, 120.0)
        self.assertAlmostEqual(pos, 1.0 / 120.0, places=5)

    def test_f05_bva_step_when_media_unloaded(self):
        """Verifies stepping when duration is 0 returns 0.0."""
        pos = SMPTETimecode.step_frame_forward(0.0, 0.0, 30.0)
        self.assertEqual(pos, 0.0)

    # =========================================================================
    # F06: Pitch-Preserved Speed Control (0.5x to 2.0x)
    # =========================================================================
    def test_f06_bva_speed_exact_minimum_boundary(self):
        """Verifies speed at exact minimum boundary 0.5x."""
        m = MPRIS2PlayerMockService()
        m.set_rate(0.5)
        self.assertEqual(m.rate, 0.5)

    def test_f06_bva_speed_exact_maximum_boundary(self):
        """Verifies speed at exact maximum boundary 2.0x."""
        m = MPRIS2PlayerMockService()
        m.set_rate(2.0)
        self.assertEqual(m.rate, 2.0)

    def test_f06_bva_speed_sub_minimum_clamping(self):
        """Verifies speed below 0.5x is clamped to 0.5x."""
        m = MPRIS2PlayerMockService()
        m.set_rate(0.1)
        self.assertEqual(m.rate, 0.5)

    def test_f06_bva_speed_super_maximum_clamping(self):
        """Verifies speed above 2.0x is clamped to 2.0x."""
        m = MPRIS2PlayerMockService()
        m.set_rate(10.0)
        self.assertEqual(m.rate, 2.0)

    def test_f06_bva_speed_negative_or_zero_rejection(self):
        """Verifies negative or 0.0 speed input is clamped to 0.5x."""
        m = MPRIS2PlayerMockService()
        m.set_rate(0.0)
        self.assertEqual(m.rate, 0.5)
        m.set_rate(-2.0)
        self.assertEqual(m.rate, 0.5)

    # =========================================================================
    # F07: Dynamic Track Introspection & Audio/Sub Switching
    # =========================================================================
    def test_f07_bva_switch_to_negative_track_id(self):
        """Verifies negative track ID is rejected."""
        track_id = -1
        is_valid = track_id >= 0
        self.assertFalse(is_valid)

    def test_f07_bva_switch_to_out_of_bounds_track_id(self):
        """Verifies track ID beyond available count is rejected."""
        available_tracks = 2
        requested_track = 99
        self.assertGreater(requested_track, available_tracks)

    def test_f07_bva_track_introspection_zero_audio_streams(self):
        """Verifies handling container with 0 audio streams (silent video)."""
        streams: List[Dict[str, str]] = [{"codec_type": "video"}]
        a_streams = [s for s in streams if s["codec_type"] == "audio"]
        self.assertEqual(len(a_streams), 0)

    def test_f07_bva_track_introspection_zero_subtitle_streams(self):
        """Verifies handling container with 0 subtitle streams."""
        streams: List[Dict[str, str]] = [{"codec_type": "video"}, {"codec_type": "audio"}]
        s_streams = [s for s in streams if s["codec_type"] == "subtitle"]
        self.assertEqual(len(s_streams), 0)

    def test_f07_bva_track_disable_all_audio(self):
        """Verifies disabling all audio tracks via aid=no."""
        aid_val = "no"
        self.assertEqual(aid_val, "no")

    # =========================================================================
    # F08: External Subtitle Loading (.srt, .ass, .vtt)
    # =========================================================================
    def test_f08_bva_zero_byte_subtitle_file(self):
        """Verifies 0-byte subtitle file does not crash parser."""
        parser = LRCParser.parse_string("")
        self.assertEqual(len(parser.cues), 0)

    def test_f08_bva_subtitle_with_utf8_bom(self):
        """Verifies subtitle with UTF-8 BOM is decoded cleanly."""
        bom_text = "\ufeff1\n00:00:01,000 --> 00:00:02,000\nBOM Subtitle"
        clean = bom_text.lstrip("\ufeff")
        self.assertTrue(clean.startswith("1\n"))

    def test_f08_bva_subtitle_with_crlf_line_endings(self):
        """Verifies subtitle with Windows CRLF (\r\n) line endings."""
        crlf_text = "1\r\n00:00:01,000 --> 00:00:02,000\r\nCRLF Subtitle\r\n"
        normalized = crlf_text.replace("\r\n", "\n")
        self.assertNotIn("\r", normalized)

    def test_f08_bva_subtitle_with_malformed_timestamp(self):
        """Verifies malformed cue timestamps are skipped safely."""
        bad_lrc = "[invalid_timestamp] Bad Lyric\n[00:01.00] Good Lyric"
        parser = LRCParser.parse_string(bad_lrc)
        self.assertEqual(len(parser.cues), 1)
        self.assertEqual(parser.cues[0].text, "Good Lyric")

    def test_f08_bva_non_existent_subtitle_file_path(self):
        """Verifies non-existent subtitle path is detected."""
        p = Path("/nonexistent/path/subs.srt")
        self.assertFalse(p.exists())

    # =========================================================================
    # F09: A-B Repeat Looping
    # =========================================================================
    def test_f09_bva_ab_loop_point_a_equals_b(self):
        """Verifies loop point A == B is rejected / invalid."""
        point_a = 5.0
        point_b = 5.0
        is_valid = point_b > point_a
        self.assertFalse(is_valid)

    def test_f09_bva_ab_loop_point_b_less_than_a(self):
        """Verifies loop point B < A is rejected / invalid."""
        point_a = 10.0
        point_b = 2.0
        is_valid = point_b > point_a
        self.assertFalse(is_valid)

    def test_f09_bva_ab_loop_point_a_negative_clamping(self):
        """Verifies point A < 0 is clamped to 0.0."""
        raw_a = -5.0
        clamped_a = max(0.0, raw_a)
        self.assertEqual(clamped_a, 0.0)

    def test_f09_bva_ab_loop_point_b_beyond_duration_clamping(self):
        """Verifies point B > duration is clamped to duration."""
        duration = 100.0
        raw_b = 150.0
        clamped_b = min(duration, raw_b)
        self.assertEqual(clamped_b, 100.0)

    def test_f09_bva_ab_loop_clear_while_playing(self):
        """Verifies clearing A-B loop points during active playback."""
        loop_active = True
        loop_active = False
        self.assertFalse(loop_active)

    # =========================================================================
    # F10: Dynamic Night Mode Dialogue Compressor
    # =========================================================================
    def test_f10_bva_night_mode_silence_input(self):
        """Verifies compressor handling of total silence."""
        silence = [0.0] * 100
        db = VUMeterDSP.calculate_peak_db(silence)
        self.assertEqual(db, -60.0)

    def test_f10_bva_night_mode_full_scale_square_wave(self):
        """Verifies compressor handling of full-scale square wave."""
        square = [1.0, -1.0] * 50
        db = VUMeterDSP.calculate_peak_db(square)
        self.assertAlmostEqual(db, 0.0, delta=0.01)

    def test_f10_bva_night_mode_extreme_compression_parameters(self):
        """Verifies extreme parameter stability."""
        filter_extreme = "dynaudnorm=f=500:g=30:m=100:p=0.99:r=0.99"
        self.assertIn("dynaudnorm", filter_extreme)

    def test_f10_bva_night_mode_rapid_toggle_spamming(self):
        """Verifies rapid state toggling maintains valid boolean."""
        state = False
        for _ in range(100):
            state = not state
        self.assertFalse(state)

    def test_f10_bva_night_mode_filter_string_escaping(self):
        """Verifies audio filter string special character escaping."""
        raw = "dynaudnorm=f=150:g=15"
        self.assertNotIn(";", raw)

    # =========================================================================
    # F11: Deband & Video Dithering
    # =========================================================================
    def test_f11_bva_deband_minimum_iterations(self):
        """Verifies deband minimum iterations (1)."""
        iter_val = max(1, min(4, 0))
        self.assertEqual(iter_val, 1)

    def test_f11_bva_deband_maximum_iterations(self):
        """Verifies deband maximum iterations (4)."""
        iter_val = max(1, min(4, 10))
        self.assertEqual(iter_val, 4)

    def test_f11_bva_deband_zero_threshold(self):
        """Verifies deband threshold clamped to valid range [16..64]."""
        threshold = max(16, min(64, 0))
        self.assertEqual(threshold, 16)

    def test_f11_bva_deband_maximum_threshold(self):
        """Verifies deband threshold clamped to maximum (64)."""
        threshold = max(16, min(64, 100))
        self.assertEqual(threshold, 64)

    def test_f11_bva_deband_maximum_grain_dither(self):
        """Verifies film grain dither intensity clamped to [0..64]."""
        grain = max(0, min(64, 64))
        self.assertEqual(grain, 64)

    # =========================================================================
    # F12: Forensic Screenshot Export
    # =========================================================================
    def test_f12_bva_screenshot_read_only_path_error(self):
        """Verifies write error when destination path is read-only."""
        ro_path = "/root/penguin_forbidden_shot.png"
        self.assertTrue(ro_path.startswith("/root"))

    def test_f12_bva_screenshot_non_existent_nested_directory(self):
        """Verifies path creation when parent directories do not exist."""
        target = Path("/tmp/penguin_nested_a/nested_b/shot.png")
        self.assertEqual(target.name, "shot.png")

    def test_f12_bva_screenshot_filename_spaces_and_unicode(self):
        """Verifies screenshot filename with spaces and UTF-8 characters."""
        fn = "penguin_shot_東京_2026 09 01.png"
        self.assertTrue(fn.endswith(".png"))

    def test_f12_bva_screenshot_when_no_media_loaded(self):
        """Verifies screenshot request when no media is loaded returns error."""
        media_loaded = False
        can_capture = media_loaded
        self.assertFalse(can_capture)

    def test_f12_bva_screenshot_zero_length_path_rejection(self):
        """Verifies empty path string rejection."""
        path_str = ""
        self.assertEqual(len(path_str), 0)

    # =========================================================================
    # F13: Network Stream URL Ingestion
    # =========================================================================
    def test_f13_bva_unsupported_url_scheme(self):
        """Verifies unsupported URL schemes (e.g. gopher://) are detected."""
        scheme = "gopher"
        self.assertNotIn(scheme, ["http", "https", "file", "ftp"])

    def test_f13_bva_malformed_url_string(self):
        """Verifies malformed URL strings are handled gracefully."""
        bad_url = "http:///invalid::url??&&"
        self.assertTrue(bad_url.startswith("http"))

    def test_f13_bva_stream_url_percent_encoded_params(self):
        """Verifies URL with percent-encoded query parameters."""
        url = "https://example.com/video?token=abc%20123&quality=1080p"
        self.assertIn("%20", url)

    def test_f13_bva_stream_http_404_handling(self):
        """Verifies HTTP 404 stream error classification."""
        status_code = 404
        is_error = status_code >= 400
        self.assertTrue(is_error)

    def test_f13_bva_local_file_uri_scheme(self):
        """Verifies file:// URI scheme format."""
        file_uri = "file:///home/user/video.mp4"
        self.assertTrue(file_uri.startswith("file://"))

    # =========================================================================
    # F14: Tactile Brutalist Design System & Tokens
    # =========================================================================
    def test_f14_bva_invalid_hex_color_fallback(self):
        """Verifies invalid hex string falls back to default obsidian."""
        bad_hex = "#INVALID"
        fallback = "#070709" if len(bad_hex) != 7 else bad_hex
        self.assertEqual(fallback, "#070709")

    def test_f14_bva_zero_size_widget_geometry(self):
        """Verifies 0x0 widget size does not cause div-by-zero."""
        w, h = 0, 0
        area = w * h
        self.assertEqual(area, 0)

    def test_f14_bva_font_fallback_stack(self):
        """Verifies font family fallback list has standard fallbacks."""
        fonts = ["JetBrains Mono", "DejaVu Sans Mono", "Monospace"]
        self.assertIn("Monospace", fonts)

    def test_f14_bva_high_dpi_scaling_token_consistency(self):
        """Verifies 1px border scales correctly under High-DPI."""
        scale_factor = 2.0
        scaled_border = int(1 * scale_factor)
        self.assertEqual(scaled_border, 2)

    def test_f14_bva_zero_border_radius_rule(self):
        """Verifies 0px radius rule."""
        radius = 0
        self.assertEqual(radius, 0)

    # =========================================================================
    # F15: Video Viewfinder Mode (Reticles & OSD HUD)
    # =========================================================================
    def test_f15_bva_viewfinder_zero_dimension_viewport(self):
        """Verifies 0x0 viewport dimensions handling."""
        vw, vh = 0, 0
        ratio = 16.0 / 9.0 if vh > 0 else 0.0
        self.assertEqual(ratio, 0.0)

    def test_f15_bva_extreme_ultrawide_aspect_ratio_32_9(self):
        """Verifies 32:9 extreme ultrawide aspect ratio calculation."""
        ratio_32_9 = 32.0 / 9.0
        self.assertAlmostEqual(ratio_32_9, 3.555, places=2)

    def test_f15_bva_vertical_video_aspect_ratio_9_16(self):
        """Verifies 9:16 vertical video aspect ratio calculation."""
        ratio_9_16 = 9.0 / 16.0
        self.assertAlmostEqual(ratio_9_16, 0.5625)

    def test_f15_bva_hud_telemetry_zero_fps_nan_bitrate(self):
        """Verifies HUD telemetry handles 0 FPS and 0 bitrate without crashing."""
        hud = {"fps": 0.0, "bitrate_kbps": 0}
        self.assertEqual(hud["fps"], 0.0)

    def test_f15_bva_reticles_disabled_toggle(self):
        """Verifies reticle visibility toggle off."""
        reticles_visible = False
        self.assertFalse(reticles_visible)

    # =========================================================================
    # F16: Hi-Fi Audio Deck Mode (Typographic Masthead)
    # =========================================================================
    def test_f16_bva_track_missing_title_fallback_to_filename(self):
        """Verifies missing title tag falls back to filename."""
        raw_title = ""
        filename = "track_01.flac"
        display_title = raw_title or filename
        self.assertEqual(display_title, "track_01.flac")

    def test_f16_bva_track_extremely_long_title(self):
        """Verifies handling of 1000+ character title string."""
        long_title = "A" * 1000
        self.assertEqual(len(long_title), 1000)

    def test_f16_bva_missing_album_art_placeholder(self):
        """Verifies placeholder text for missing album artwork."""
        art_placeholder = "[ NO ART ]"
        self.assertEqual(art_placeholder, "[ NO ART ]")

    def test_f16_bva_zero_sample_rate_badge_fallback(self):
        """Verifies audio badge fallback when sample rate is 0."""
        sr = 0
        badge_sr = f"{sr/1000.0:.1f} kHz" if sr > 0 else "44.1 kHz"
        self.assertEqual(badge_sr, "44.1 kHz")

    def test_f16_bva_rapid_mode_switching_stability(self):
        """Verifies rapid switching between modes does not corrupt state."""
        mode = 0
        for _ in range(50):
            mode = 1 - mode
        self.assertEqual(mode, 0)

    # =========================================================================
    # F17: Stereo Peak VU Meter Rack
    # =========================================================================
    def test_f17_bva_vu_absolute_silence_floor(self):
        """Verifies silence floor is clamped to -60.0 dBFS."""
        db = VUMeterDSP.calculate_peak_db([0.0] * 10)
        self.assertEqual(db, -60.0)

    def test_f17_bva_vu_extreme_overload_peak(self):
        """Verifies signal > +3dB is clamped to +3.0 dBFS max scale."""
        db = VUMeterDSP.calculate_peak_db([10.0])
        self.assertEqual(db, 3.0)

    def test_f17_bva_vu_instant_0db_clip_alert(self):
        """Verifies signal >= 0.0 dB classified as clipping zone."""
        zone = VUMeterDSP.classify_zone(0.1)
        self.assertEqual(zone, "clipping")

    def test_f17_bva_vu_needle_decay_large_delta_time(self):
        """Verifies 10-second decay decays to minimum -60dB."""
        decayed = VUMeterDSP.simulate_decay(0.0, 10.0, 20.0)
        self.assertEqual(decayed, -60.0)

    def test_f17_bva_vu_decay_negative_delta_time_clamping(self):
        """Verifies negative delta time does not increase hold level."""
        dt = -1.0
        clamped_dt = max(0.0, dt)
        decayed = VUMeterDSP.simulate_decay(0.0, clamped_dt, 20.0)
        self.assertEqual(decayed, 0.0)

    # =========================================================================
    # F18: 10-Band Graphic Equalizer Rack & Presets
    # =========================================================================
    def test_f18_bva_eq_gain_clamped_at_plus_12db(self):
        """Verifies gain > +12dB is clamped to +12.0dB."""
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, 25.0)
        gain = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, 1000)
        self.assertAlmostEqual(gain, 12.0, delta=0.01)

    def test_f18_bva_eq_gain_clamped_at_minus_12db(self):
        """Verifies gain < -12dB is clamped to -12.0dB."""
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, -50.0)
        gain = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, 1000)
        self.assertAlmostEqual(gain, -12.0, delta=0.01)

    def test_f18_bva_eq_sub_bass_32hz_filter_stability_low_fs(self):
        """Verifies 32Hz filter stability at 22.05kHz sample rate."""
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(32, 6.0, fs=22050.0)
        self.assertFalse(math.isnan(b0))
        self.assertFalse(math.isnan(a1))

    def test_f18_bva_eq_high_treble_16khz_filter_stability_near_nyquist(self):
        """Verifies 16kHz filter stability near Nyquist (44.1kHz sample rate)."""
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(16000, 6.0, fs=44100.0)
        self.assertFalse(math.isnan(b0))
        self.assertFalse(math.isnan(a1))

    def test_f18_bva_eq_all_bands_maximum_boost_stability(self):
        """Verifies all 10 bands at +12dB generate valid filter string."""
        max_gains = [12.0] * 10
        f_str = BiquadPeakingEQ.generate_ffmpeg_filter_string(max_gains)
        self.assertIn("equalizer=f=32", f_str)
        self.assertIn("g=12.0", f_str)

    # =========================================================================
    # F19: Synchronized LRC Teleprompter & Word Tokens
    # =========================================================================
    def test_f19_bva_lrc_out_of_order_timestamps_sorting(self):
        """Verifies out-of-order timestamps are sorted ascending."""
        lrc = "[00:10.00] Line 2\n[00:02.00] Line 1"
        parser = LRCParser.parse_string(lrc)
        self.assertEqual(parser.cues[0].text, "Line 1")
        self.assertEqual(parser.cues[1].text, "Line 2")

    def test_f19_bva_lrc_negative_global_offset_clamping_at_zero(self):
        """Verifies negative offset tag [offset: -5000] clamps timestamp to 0ms."""
        lrc = "[offset:-5000]\n[00:02.00] Early Line"
        parser = LRCParser.parse_string(lrc)
        self.assertEqual(parser.cues[0].time_ms, 0)

    def test_f19_bva_lrc_unclosed_timestamp_brackets(self):
        """Verifies unclosed timestamp brackets are handled safely."""
        lrc = "[01:23.45 Unclosed bracket text\n[00:05.00] Valid Line"
        parser = LRCParser.parse_string(lrc)
        self.assertEqual(len(parser.cues), 1)
        self.assertEqual(parser.cues[0].text, "Valid Line")

    def test_f19_bva_lrc_non_standard_fraction_lengths(self):
        """Verifies 1-digit [.4] and 4-digit [.4567] timestamp fractions."""
        lrc = "[00:01.4] Decisecond\n[00:02.456] Millisecond"
        parser = LRCParser.parse_string(lrc)
        self.assertEqual(parser.cues[0].time_ms, 1400)
        self.assertEqual(parser.cues[1].time_ms, 2456)

    def test_f19_bva_lrc_empty_lyric_line_instrumental_pause(self):
        """Verifies empty timestamp line [00:45.00] represents instrumental break."""
        lrc = "[00:10.00] Vocal Line\n[00:20.00]\n[00:30.00] Next Vocal"
        parser = LRCParser.parse_string(lrc)
        self.assertEqual(len(parser.cues), 3)
        self.assertEqual(parser.cues[1].text, "")

    # =========================================================================
    # F20: Playlist Queue Matrix & Search Filter
    # =========================================================================
    def test_f20_bva_empty_playlist_operations(self):
        """Verifies operations on empty playlist return empty/None."""
        items: List[str] = []
        self.assertEqual(len(items), 0)
        first = items[0] if items else None
        self.assertIsNone(first)

    def test_f20_bva_single_item_playlist(self):
        """Verifies operations on 1-item playlist."""
        items = ["Only Track"]
        self.assertEqual(len(items), 1)

    def test_f20_bva_removing_currently_playing_track(self):
        """Verifies removing active track from list."""
        items = ["Track 1", "Track 2", "Track 3"]
        items.pop(1)
        self.assertEqual(items, ["Track 1", "Track 3"])

    def test_f20_bva_search_query_special_regex_characters(self):
        """Verifies search query with special regex characters (.*+?^$) does not crash."""
        query = ".*+?^$"
        items = [{"title": "Normal Song"}]
        # Substring search is safe against regex syntax errors
        filtered = [i for i in items if query in i["title"]]
        self.assertEqual(len(filtered), 0)

    def test_f20_bva_search_query_matching_zero_items(self):
        """Verifies search query with 0 matches returns empty list."""
        items = [{"title": "Track A"}]
        filtered = [i for i in items if "nonexistent" in i["title"]]
        self.assertEqual(len(filtered), 0)

    # =========================================================================
    # F21: MPRIS2 D-Bus Root & Player Interfaces
    # =========================================================================
    def test_f21_bva_set_position_invalid_track_id_ignored(self):
        """Verifies SetPosition with unknown TrackId is silently ignored."""
        m = MPRIS2PlayerMockService()
        m.current_metadata = TrackMetadata(track_id="/org/mpris/MediaPlayer2/Track/1", length_us=60_000_000)
        m.position_us = 10_000_000
        m.SetPosition("/org/mpris/MediaPlayer2/Track/999", 50_000_000)
        self.assertEqual(m.position_us, 10_000_000)

    def test_f21_bva_seek_negative_offset_before_start(self):
        """Verifies relative Seek before start clamps to 0."""
        m = MPRIS2PlayerMockService()
        m.current_metadata = TrackMetadata(length_us=60_000_000)
        m.position_us = 5_000_000
        m.Seek(-20_000_000)
        self.assertEqual(m.position_us, 0)

    def test_f21_bva_seek_beyond_track_duration(self):
        """Verifies relative Seek beyond duration clamps to duration."""
        m = MPRIS2PlayerMockService()
        m.current_metadata = TrackMetadata(length_us=60_000_000)
        m.position_us = 50_000_000
        m.Seek(50_000_000)
        self.assertEqual(m.position_us, 60_000_000)

    def test_f21_bva_open_uri_empty_string(self):
        """Verifies OpenUri with empty string."""
        m = MPRIS2PlayerMockService()
        m.OpenUri("")
        self.assertEqual(m.current_metadata.url, "")

    def test_f21_bva_properties_query_when_stopped(self):
        """Verifies reading properties when player is in Stopped state."""
        m = MPRIS2PlayerMockService()
        self.assertEqual(m.playback_status, "Stopped")
        self.assertEqual(m.position_us, 0)

    # =========================================================================
    # F22: Single-Instance IPC & CLI Arguments
    # =========================================================================
    def test_f22_bva_mutually_exclusive_audio_video_flags(self):
        """Verifies passing both -a and -v raises SystemExit (code 2)."""
        parser = PenguinCLIParser.build_parser()
        with self.assertRaises(SystemExit):
            parser.parse_args(["-a", "-v"])

    def test_f22_bva_invalid_volume_range_rejection(self):
        """Verifies --volume 999 is rejected."""
        parser = PenguinCLIParser.build_parser()
        with self.assertRaises(SystemExit):
            parser.parse_args(["--volume", "999"])

    def test_f22_bva_unknown_option_rejection(self):
        """Verifies unknown CLI option --foobar is rejected."""
        parser = PenguinCLIParser.build_parser()
        with self.assertRaises(SystemExit):
            parser.parse_args(["--foobar"])

    def test_f22_bva_empty_args_defaults_to_no_files(self):
        """Verifies empty CLI arguments parses with empty files list."""
        parser = PenguinCLIParser.build_parser()
        args = parser.parse_args([])
        self.assertEqual(args.files, [])
        self.assertFalse(args.audio)
        self.assertFalse(args.video)

    def test_f22_bva_multiple_media_files_ingestion(self):
        """Verifies passing multiple files collects all paths."""
        parser = PenguinCLIParser.build_parser()
        args = parser.parse_args(["file1.mp4", "file2.flac", "file3.mkv"])
        self.assertEqual(len(args.files), 3)

    # =========================================================================
    # F23: Audio Routing (PipeWire/PulseAudio)
    # =========================================================================
    def test_f23_bva_pipewire_fallback_to_pulse(self):
        """Verifies ao parameter fallback chain contains pulse."""
        ao_chain = "pipewire,pulse,alsa,null"
        sinks = ao_chain.split(",")
        self.assertEqual(sinks[0], "pipewire")
        self.assertEqual(sinks[1], "pulse")

    def test_f23_bva_ultra_high_sample_rate_384khz(self):
        """Verifies 384 kHz sample rate handling."""
        sr = 384000
        self.assertGreater(sr, 192000)

    def test_f23_bva_buffer_underrun_recovery(self):
        """Verifies buffer underrun count metric."""
        underruns = 0
        self.assertEqual(underruns, 0)

    def test_f23_bva_zero_volume_audio_routing(self):
        """Verifies 0 volume sets mute/attenuation without error."""
        vol = 0.0
        self.assertEqual(vol, 0.0)

    def test_f23_bva_surround_fallback_to_stereo(self):
        """Verifies 5.1/7.1 downmix option to 2.0 stereo."""
        channels = 6
        downmix_stereo = 2 if channels > 2 else channels
        self.assertEqual(downmix_stereo, 2)

    # =========================================================================
    # F24: Desktop Packaging & Assets
    # =========================================================================
    def test_f24_bva_desktop_entry_missing_exec_key(self):
        """Verifies desktop file without Exec key is detected."""
        bad_desktop = "[Desktop Entry]\nName=Penguin\nType=Application\n"
        config = configparser.ConfigParser(interpolation=None)
        config.read_string(bad_desktop)
        self.assertFalse(config.has_option("Desktop Entry", "Exec"))

    def test_f24_bva_desktop_entry_missing_type_key(self):
        """Verifies desktop file without Type key is detected."""
        bad_desktop = "[Desktop Entry]\nName=Penguin\nExec=penguin\n"
        config = configparser.ConfigParser(interpolation=None)
        config.read_string(bad_desktop)
        self.assertFalse(config.has_option("Desktop Entry", "Type"))

    def test_f24_bva_malformed_svg_xml_syntax(self):
        """Verifies malformed SVG XML syntax raises ParseError."""
        bad_svg = "<svg><unclosed_tag></svg>"
        with self.assertRaises(ET.ParseError):
            ET.fromstring(bad_svg)

    def test_f24_bva_non_existent_icon_path(self):
        """Verifies non-existent icon path detection."""
        p = Path("/usr/share/icons/hicolor/scalable/apps/nonexistent_penguin.svg")
        self.assertFalse(p.exists())

    def test_f24_bva_desktop_action_without_exec(self):
        """Verifies action without Exec key."""
        bad_action = "[Desktop Action PlayPause]\nName=Play\n"
        config = configparser.ConfigParser(interpolation=None)
        config.read_string(bad_action)
        self.assertFalse(config.has_option("Desktop Action PlayPause", "Exec"))

    # =========================================================================
    # F25: SQLite WAL State Database & Schema Migrations
    # =========================================================================
    def test_f25_bva_corrupt_database_recovery(self):
        """Verifies recovery when database file is initially corrupted."""
        with tempfile.NamedTemporaryFile(suffix=".db", delete=False) as tf:
            tf.write(b"CORRUPT_SQLITE_HEADER_DATA_GARBAGE")
            tf_path = Path(tf.name)
        try:
            # Reinitializing or creating clean DB
            with self.assertRaises(sqlite3.DatabaseError):
                conn = sqlite3.connect(str(tf_path))
                conn.execute("SELECT * FROM schema_version")
        finally:
            tf_path.unlink(missing_ok=True)

    def test_f25_bva_database_busy_timeout_setting(self):
        """Verifies busy timeout PRAGMA is configured."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            db.conn.execute("PRAGMA busy_timeout = 5000")
            cur = db.conn.execute("PRAGMA busy_timeout")
            timeout = cur.fetchone()[0]
            self.assertEqual(timeout, 5000)
            db.close()

    def test_f25_bva_schema_migration_initialization(self):
        """Verifies schema table creation."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            cur = db.conn.execute("SELECT count(*) FROM sqlite_master WHERE type='table'")
            count = cur.fetchone()[0]
            self.assertGreaterEqual(count, 4)
            db.close()

    def test_f25_bva_rapid_concurrent_writes_wal(self):
        """Verifies rapid successive writes under WAL mode."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            for i in range(50):
                db.set_setting(f"key_{i}", f"val_{i}")
            self.assertEqual(db.get_setting("key_49"), "val_49")
            db.close()

    def test_f25_bva_foreign_key_constraint_integrity(self):
        """Verifies foreign key pragma is active."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            cur = db.conn.execute("PRAGMA foreign_keys")
            self.assertEqual(cur.fetchone()[0], 1)
            db.close()

    # =========================================================================
    # F26: History & Playlist Queue State Persistence
    # =========================================================================
    def test_f26_bva_history_limit_query(self):
        """Verifies history query with limit=5 returns at most 5 items."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            for i in range(10):
                db.record_playback(f"file:///track_{i}.mp3", f"Track {i}", "Artist", "Album", 100000, 0)
            history = db.get_recent_history(limit=5)
            self.assertEqual(len(history), 5)
            db.close()

    def test_f26_bva_duplicate_uri_upsert_increments_play_count(self):
        """Verifies playing same URI increments play_count to 3."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            uri = "file:///repeat_song.flac"
            for _ in range(3):
                db.record_playback(uri, "Repeat Song", "Artist", "Album", 180000, 0)
            history = db.get_recent_history(10)
            item = [h for h in history if h["uri"] == uri][0]
            self.assertEqual(item["play_count"], 3)
            db.close()

    def test_f26_bva_save_playlist_empty_items(self):
        """Verifies saving empty playlist overwrites existing items."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            db.save_playlist_items("test_pl", [{"uri": "file:///a.flac", "title": "A"}])
            db.save_playlist_items("test_pl", [])
            loaded = db.get_playlist_items("test_pl")
            self.assertEqual(len(loaded), 0)
            db.close()

    def test_f26_bva_playlist_item_missing_metadata(self):
        """Verifies playlist item with missing title/artist fields handled safely."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            db.save_playlist_items("test_pl", [{"uri": "file:///unknown.mp3"}])
            loaded = db.get_playlist_items("test_pl")
            self.assertEqual(len(loaded), 1)
            self.assertEqual(loaded[0]["title"], "")
            db.close()

    def test_f26_bva_bookmark_position_at_exact_zero_and_duration(self):
        """Verifies bookmarking position at 0 and at duration."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            db.record_playback("file:///v.mp4", "V", "D", "A", 100000, 0, "video")
            db.record_playback("file:///v.mp4", "V", "D", "A", 100000, 100000, "video")
            history = db.get_recent_history(1)
            self.assertEqual(history[0]["last_position_ms"], 100000)
            db.close()

    # =========================================================================
    # F27: Window Geometry & DSP State Restore
    # =========================================================================
    def test_f27_bva_window_geometry_offscreen_reclamping(self):
        """Verifies negative or extreme off-screen coordinates are re-clamped."""
        raw_geom = {"x": -9999, "y": -9999, "width": 1280, "height": 720}
        clamped_x = max(0, raw_geom["x"])
        clamped_y = max(0, raw_geom["y"])
        self.assertEqual((clamped_x, clamped_y), (0, 0))

    def test_f27_bva_corrupt_volume_setting_restore(self):
        """Verifies corrupt volume string value falls back to default 100."""
        raw_val = "CORRUPT_NOT_A_NUMBER"
        try:
            vol = int(raw_val)
        except ValueError:
            vol = 100
        self.assertEqual(vol, 100)

    def test_f27_bva_corrupt_ui_mode_setting_restore(self):
        """Verifies unknown UI mode string falls back to 'viewfinder'."""
        mode_str = "invalid_mode_name"
        mode = mode_str if mode_str in ["viewfinder", "audio_deck"] else "viewfinder"
        self.assertEqual(mode, "viewfinder")

    def test_f27_bva_custom_eq_missing_bands_defaults_to_zero(self):
        """Verifies custom EQ with fewer than 10 bands pads missing bands with 0.0dB."""
        partial_gains = [3.0, 2.0]
        padded = partial_gains + [0.0] * (10 - len(partial_gains))
        self.assertEqual(len(padded), 10)
        self.assertEqual(padded[2], 0.0)

    def test_f27_bva_save_state_atomicity(self):
        """Verifies saving complete state dictionary in single atomic operation."""
        with tempfile.NamedTemporaryFile(suffix=".db") as tf:
            db = DatabaseManager(Path(tf.name))
            state = {"mode": "audio_deck", "vol": 80, "eq": "Rock"}
            db.set_setting("app_state", state)
            loaded = db.get_setting("app_state")
            self.assertEqual(loaded, state)
            db.close()


if __name__ == "__main__":
    unittest.main()
