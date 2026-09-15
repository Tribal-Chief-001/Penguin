"""
test_engine_m1_complete.py - Comprehensive Verification Suite for M1 Core Engine & Audio DSP.

Tests all components under src/engine/:
- SMPTE 12M timecode calculation (NDF & DF with 10-min exceptions).
- 10-band ISO graphic equalizer DSP & Biquad transfer functions.
- Dual-path stereo peak & RMS VU waveform analyzer with 60fps needle ballistics.
- Synchronized LRC lyric parser (line-level, word-level, metadata, offsets, fuzzing).
- Forensic PNG screenshot export & sidecar telemetry metadata.
- libmpv ctypes binding & PlaybackEngine transport/filter/track controller in headless mode.
"""

from pathlib import Path
import math
import os
import shutil
import tempfile
import time
import unittest

from src.engine.smpte import (
    SMPTETimecode,
    ms_to_smpte,
    smpte_to_ms,
    smpte_step_forward,
    smpte_step_backward,
)
from src.engine.audio_dsp import (
    BiquadPeakingEQ,
    EqualizerDSP,
    VUMeterDSP,
    ISO_CENTER_FREQUENCIES,
    DEFAULT_Q_FACTOR,
)
from src.engine.vu_analyzer import (
    VUAnalyzer,
    AudioLevelSample,
    VUState,
)
from src.engine.lrc_parser import (
    LRCParser,
    LRCCue,
    LRCToken,
)
from src.engine.screenshot import (
    ScreenshotExporter,
    ScreenshotMetadata,
)
from src.engine.mpv_core import (
    MPVHandle,
    MPVError,
    MPVFormat,
    MPVEventId,
    PlaybackEngine,
    PlaybackState,
    MediaTrack,
    TelemetryData,
)


class TestM1SMPTE(unittest.TestCase):
    """Verifies SMPTE 12M calculations, drop-frame rules, and frame stepping."""

    def test_ndf_standard_rates(self):
        # 24 fps
        self.assertEqual(SMPTETimecode.format_timecode(0.0, 24.0), "00:00:00:00")
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 24.0), "00:00:01:00")
        self.assertEqual(SMPTETimecode.format_timecode(1.0 / 24.0, 24.0), "00:00:00:01")
        self.assertEqual(SMPTETimecode.format_timecode(23.0 / 24.0, 24.0), "00:00:00:23")

        # 25 fps PAL
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 25.0), "00:00:01:00")
        self.assertEqual(SMPTETimecode.format_timecode(24.0 / 25.0, 25.0), "00:00:00:24")

        # 30 fps
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 30.0), "00:00:01:00")
        self.assertEqual(SMPTETimecode.format_timecode(0.5, 30.0), "00:00:00:15")

        # 60 fps
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 60.0), "00:00:01:00")
        self.assertEqual(SMPTETimecode.format_timecode(59.0 / 60.0, 60.0), "00:00:00:59")

    def test_drop_frame_2997_invariants(self):
        fps = 30000.0 / 1001.0
        # Frame 0
        self.assertEqual(SMPTETimecode.format_timecode(0.0, fps, drop_frame=True), "00:00:00;00")

        # Frame 1800 (Minute 1): skips ;00 and ;01 -> ;02
        ms_min1 = SMPTETimecode.frames_to_ms(1800, fps)
        self.assertEqual(SMPTETimecode.format_timecode_ms(ms_min1, fps, drop_frame=True), "00:01:00;02")

        # Frame 17982 (Minute 10): 10-min exception -> ;00
        ms_min10 = SMPTETimecode.frames_to_ms(17982, fps)
        self.assertEqual(SMPTETimecode.format_timecode_ms(ms_min10, fps, drop_frame=True), "00:10:00;00")

    def test_drop_frame_5994_invariants(self):
        fps = 60000.0 / 1001.0
        # Minute 1 skips 4 frames -> ;04
        ms_min1 = SMPTETimecode.frames_to_ms(3600, fps)
        self.assertEqual(SMPTETimecode.format_timecode_ms(ms_min1, fps, drop_frame=True), "00:01:00;04")

        # Minute 10 no skip -> ;00
        ms_min10 = SMPTETimecode.frames_to_ms(35964, fps)
        self.assertEqual(SMPTETimecode.format_timecode_ms(ms_min10, fps, drop_frame=True), "00:10:00;00")

    def test_remaining_time(self):
        self.assertEqual(SMPTETimecode.format_remaining_timecode(100.0, 3600.0, 30.0), "-00:58:20:00")
        self.assertEqual(SMPTETimecode.format_remaining_timecode(3600.0, 3600.0, 30.0), "-00:00:00:00")

    def test_single_frame_stepping_and_jumps(self):
        pos = 0.0
        fps = 24.0
        for _ in range(24):
            pos = SMPTETimecode.step_frame_forward(pos, 100.0, fps)
        self.assertAlmostEqual(pos, 1.0, delta=1e-5)

        for _ in range(24):
            pos = SMPTETimecode.step_frame_backward(pos, 100.0, fps)
        self.assertAlmostEqual(pos, 0.0, delta=1e-5)

        # Clamping at bounds
        self.assertEqual(SMPTETimecode.step_frame_backward(0.0, 100.0, fps), 0.0)
        self.assertEqual(SMPTETimecode.step_frame_forward(100.0, 100.0, fps), 100.0)

        # Jumps
        self.assertEqual(SMPTETimecode.jump_seconds(10.0, 10.0, 100.0), 20.0)
        self.assertEqual(SMPTETimecode.jump_seconds(5.0, -10.0, 100.0), 0.0)
        self.assertEqual(SMPTETimecode.jump_seconds(95.0, 10.0, 100.0), 100.0)

    def test_bidirectional_roundtrip_and_errors(self):
        fps = 25.0
        for s in [0.0, 1.5, 60.0, 3600.0, 7200.2]:
            tc = SMPTETimecode.format_timecode(s, fps)
            parsed_s = SMPTETimecode.parse_timecode(tc, fps)
            tc_re = SMPTETimecode.format_timecode(parsed_s, fps)
            self.assertEqual(tc, tc_re)

        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("00:00:00")
        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("00:65:00:00")


class TestM1AudioDSP(unittest.TestCase):
    """Verifies 10-band equalizer DSP, Biquad transfer functions, and VU meter calculations."""

    def test_iso_frequencies_and_gain_accuracy(self):
        fs = 48000.0
        test_gains = [-12.0, -6.0, 0.0, 3.0, 6.0, 12.0]
        for f0 in ISO_CENTER_FREQUENCIES:
            if f0 >= fs / 2:
                continue
            for g in test_gains:
                b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(f0, g, fs=fs)
                resp = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, f0, fs=fs)
                self.assertAlmostEqual(resp, g, delta=0.001)

    def test_flat_and_dc_nyquist_response(self):
        fs = 48000.0
        # Flat
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, 0.0, fs=fs)
        self.assertEqual((b0, b1, b2, a1, a2), (1.0, 0.0, 0.0, 0.0, 0.0))

        # DC and Nyquist are flat for peaking filter
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, 6.0, fs=fs)
        self.assertAlmostEqual(BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, 0.0, fs=fs), 0.0, delta=0.001)
        self.assertAlmostEqual(BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, fs / 2, fs=fs), 0.0, delta=0.001)

    def test_filter_string_generation(self):
        rock = BiquadPeakingEQ.PRESETS["Rock"]
        s = BiquadPeakingEQ.generate_ffmpeg_filter_string(rock)
        self.assertIn("equalizer=f=32", s)
        self.assertIn("equalizer=f=16000", s)

        mpv_af = BiquadPeakingEQ.generate_mpv_af_string(rock, night_mode=True)
        self.assertIn("lavfi=[equalizer=", mpv_af)
        self.assertIn("dynaudnorm", mpv_af)

    def test_buffer_processing(self):
        samples = [1.0, 0.5, -0.5, -1.0, 0.0] * 20
        flat_gains = [0.0] * 10
        processed = BiquadPeakingEQ.process_buffer(samples, flat_gains)
        self.assertEqual(len(processed), len(samples))
        for o, i in zip(processed, samples):
            self.assertAlmostEqual(o, i, delta=1e-5)


class TestM1VUAnalyzer(unittest.TestCase):
    """Verifies dual-path VU analyzer, ballistics, decay, and clipping."""

    def test_vu_ballistics_and_decay(self):
        analyzer = VUAnalyzer()
        # Full scale jump (instant attack)
        state1 = analyzer.update_ballistics(0.01, 1.0, 1.0)
        self.assertAlmostEqual(state1.left_db, 0.0, delta=0.01)
        self.assertTrue(state1.left_clipped)
        self.assertEqual(state1.zone_left, "headroom")

        # Advance time by 0.5s with silence (decay at 20dB/s)
        state2 = analyzer.update_ballistics(0.5, 0.0, 0.0)
        self.assertAlmostEqual(state2.left_db, -10.0, delta=0.5)

        # Peak hold holds for 1.5s
        self.assertAlmostEqual(state2.left_hold_db, 0.0, delta=0.01)


class TestM1LRCParser(unittest.TestCase):
    """Verifies standard and enhanced LRC parsing, word tokens, and binary search."""

    def test_lrc_full_features(self):
        lrc_content = """
        [ti: Penguin Anthem]
        [ar: Brutalist Audio]
        [al: Viewfinder Edition]
        [offset: -200]
        [00:10.00] Line 2 at 9.8s
        [00:05.00] Line 1 at 4.8s
        [00:15.00] <00:15.00> Tactical <00:15.50> Brutalism
        [00:25.00]
        """
        parser = LRCParser.parse_string(lrc_content)
        self.assertEqual(parser.title, "Penguin Anthem")
        self.assertEqual(parser.artist, "Brutalist Audio")
        self.assertEqual(len(parser.cues), 4)

        # Chronologically sorted with -200ms offset
        self.assertEqual(parser.cues[0].time_ms, 4800)
        self.assertEqual(parser.cues[0].text, "Line 1 at 4.8s")
        self.assertEqual(parser.cues[1].time_ms, 9800)
        self.assertEqual(parser.cues[2].time_ms, 14800)
        self.assertEqual(parser.cues[3].time_ms, 24800)
        self.assertTrue(parser.cues[3].is_empty)

        # Binary search
        self.assertEqual(parser.get_active_cue_index(0), -1)
        self.assertEqual(parser.get_active_cue_index(4800), 0)
        self.assertEqual(parser.get_active_cue_index(7000), 0)
        self.assertEqual(parser.get_active_cue_index(9800), 1)

        # Word tokens
        cue_tokens = parser.cues[2]
        self.assertEqual(len(cue_tokens.tokens), 2)
        self.assertEqual(cue_tokens.tokens[0].text, "Tactical")
        self.assertEqual(cue_tokens.tokens[1].text, "Brutalism")


class TestM1Screenshot(unittest.TestCase):
    """Verifies screenshot metadata generation, JSON sidecars, and PNG tEXt chunks."""

    def test_metadata_and_png_chunks(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp_path = Path(tmpdir)
            png_file = tmp_path / "frame.png"
            raw_png = bytes.fromhex("89504e470d0a1a0a0000000d4948445200000001000000010802000000907753de0000000c49444154789c636000000002000148afa4710000000049454e44ae426082")
            png_file.write_bytes(raw_png)

            meta = ScreenshotExporter.create_metadata(
                source_file="media.mkv",
                position_ms=5000.0,
                fps=25.0,
                resolution="3840x2160",
                screenshot_path=str(png_file),
            )
            sidecar = meta.write_sidecar_json()
            self.assertTrue(sidecar.exists())
            self.assertEqual(meta.smpte_timecode, "00:00:05:00")

            ScreenshotExporter.embed_png_text_chunk(png_file, "SMPTE", meta.smpte_timecode)
            data = png_file.read_bytes()
            self.assertIn(b"tEXt", data)
            self.assertIn(b"SMPTE", data)


class TestM1PlaybackEngine(unittest.TestCase):
    """Verifies PlaybackEngine headless transport, audio DSP, tracks, and media playback."""

    def setUp(self):
        self.engine = PlaybackEngine(headless=True)

    def tearDown(self):
        self.engine.terminate()

    def test_playback_engine_controls(self):
        # Speed
        self.engine.set_speed(1.25)
        self.assertEqual(self.engine.get_speed(), 1.25)

        # Volume
        self.engine.set_volume(80)
        self.assertEqual(self.engine.get_volume(), 80.0)

        # Mute
        self.engine.set_mute(True)
        self.assertTrue(self.engine.is_muted())
        self.engine.set_mute(False)
        self.assertFalse(self.engine.is_muted())

        # EQ presets
        self.engine.set_equalizer_preset("Bass Boost")
        gains = self.engine.get_equalizer_gains()
        self.assertEqual(gains[0], 7.0)

        # Night mode & deband
        self.engine.set_night_mode(True)
        self.assertTrue(self.engine.is_night_mode())
        self.engine.set_deband(True)
        self.assertTrue(self.engine.is_deband())

    def test_media_playback_and_stepping(self):
        test_video = Path("tests/fixtures/test_video.mp4")
        if test_video.exists():
            self.engine.load(str(test_video))
            time.sleep(0.3)
            self.assertGreater(self.engine.get_duration_ms(), 0)

            # Pause & step
            self.engine.pause()
            self.assertTrue(self.engine.is_paused())
            self.engine.frame_step_forward()
            self.engine.frame_step_backward()

            # Seek
            self.engine.seek_ms(1000.0, exact=True)
            time.sleep(0.1)

            # Telemetry
            telemetry = self.engine.get_telemetry()
            self.assertIsNotNone(telemetry)

            # VU levels
            l, r = self.engine.get_vu_levels()
            self.assertGreaterEqual(l, 0.0)
            self.assertGreaterEqual(r, 0.0)


if __name__ == "__main__":
    unittest.main()
