"""
test_timecode.py - Comprehensive SMPTE Timecode Calculation and Verification Suite.

Tests non-drop frame SMPTE calculations (HH:MM:SS:FF), fractional frame rates
(23.976, 24, 25, 29.97, 30, 50, 59.94, 60), bidirectional conversions, remaining
time formatting (-HH:MM:SS:FF), single-frame stepping (+/- 1F), and edge cases.
"""

import math
import unittest
from typing import Tuple


class SMPTETimecode:
    """Standard SMPTE Timecode calculation engine implementing non-drop frame specifications."""

    @staticmethod
    def nominal_fps(fps: float) -> int:
        return int(round(fps))

    @classmethod
    def seconds_to_frame_count(cls, seconds: float, fps: float = 30.0) -> int:
        if seconds <= 0.0:
            return 0
        return int(math.floor(seconds * fps + 0.5))

    @classmethod
    def frame_count_to_seconds(cls, frame_count: int, fps: float = 30.0) -> float:
        if frame_count <= 0 or fps <= 0:
            return 0.0
        return float(frame_count) / fps

    @classmethod
    def format_timecode(cls, seconds: float, fps: float = 30.0) -> str:
        """Formats seconds into SMPTE timecode HH:MM:SS:FF."""
        if seconds < 0:
            seconds = 0.0
        if fps <= 0:
            fps = 30.0

        n_nominal = cls.nominal_fps(fps)
        total_frames = cls.seconds_to_frame_count(seconds, fps)

        frames_per_sec = n_nominal
        frames_per_min = frames_per_sec * 60
        frames_per_hr = frames_per_min * 60

        hh = total_frames // frames_per_hr
        rem = total_frames % frames_per_hr
        mm = rem // frames_per_min
        rem = rem % frames_per_min
        ss = rem // frames_per_sec
        ff = rem % frames_per_sec

        return f"{hh:02d}:{mm:02d}:{ss:02d}:{ff:02d}"

    @classmethod
    def format_remaining_timecode(cls, current_sec: float, duration_sec: float, fps: float = 30.0) -> str:
        """Formats remaining time into SMPTE timecode -HH:MM:SS:FF."""
        rem_sec = max(0.0, duration_sec - max(0.0, current_sec))
        tc = cls.format_timecode(rem_sec, fps)
        return f"-{tc}"

    @classmethod
    def parse_timecode(cls, tc_str: str, fps: float = 30.0) -> float:
        """Parses HH:MM:SS:FF into total seconds."""
        parts = tc_str.strip().lstrip("-").split(":")
        if len(parts) != 4:
            raise ValueError(f"Invalid SMPTE timecode format: '{tc_str}', expected HH:MM:SS:FF")
        try:
            hh = int(parts[0])
            mm = int(parts[1])
            ss = int(parts[2])
            ff = int(parts[3])
        except ValueError as e:
            raise ValueError(f"Non-integer components in timecode: '{tc_str}'") from e

        if mm < 0 or mm >= 60:
            raise ValueError(f"Minutes out of range [0, 59]: {mm}")
        if ss < 0 or ss >= 60:
            raise ValueError(f"Seconds out of range [0, 59]: {ss}")
        n_nominal = cls.nominal_fps(fps)
        if ff < 0 or ff >= n_nominal:
            raise ValueError(f"Frames out of range [0, {n_nominal-1}]: {ff}")

        total_frames = (hh * 3600 * n_nominal) + (mm * 60 * n_nominal) + (ss * n_nominal) + ff
        return cls.frame_count_to_seconds(total_frames, fps)

    @classmethod
    def step_frame_forward(cls, current_sec: float, duration_sec: float, fps: float = 30.0) -> float:
        """Calculates new timestamp after stepping 1 frame forward (+1F), clamped to duration."""
        if fps <= 0:
            fps = 30.0
        delta = 1.0 / fps
        return min(duration_sec, max(0.0, current_sec + delta))

    @classmethod
    def step_frame_backward(cls, current_sec: float, duration_sec: float, fps: float = 30.0) -> float:
        """Calculates new timestamp after stepping 1 frame backward (-1F), clamped to 0."""
        if fps <= 0:
            fps = 30.0
        delta = 1.0 / fps
        return max(0.0, min(duration_sec, current_sec - delta))

    @classmethod
    def jump_seconds(cls, current_sec: float, offset_sec: float, duration_sec: float) -> float:
        """Calculates relative seek jump (+/- 10s, +/- 30s), clamped to [0, duration]."""
        target = current_sec + offset_sec
        return max(0.0, min(duration_sec, target))


class TestSMPTETimecode(unittest.TestCase):
    """Test suite for SMPTE timecode formatting, parsing, stepping, and frame rates."""

    def test_zero_timestamp(self):
        for fps in [23.976, 24.0, 25.0, 29.97, 30.0, 50.0, 59.94, 60.0]:
            tc = SMPTETimecode.format_timecode(0.0, fps)
            self.assertEqual(tc, "00:00:00:00", f"Failed for fps={fps}")

    def test_standard_fps_24(self):
        # Exactly 1 second = 24 frames
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 24.0), "00:00:01:00")
        # 1 frame = 1/24 sec
        self.assertEqual(SMPTETimecode.format_timecode(1.0 / 24.0, 24.0), "00:00:00:01")
        # 23 frames = 23/24 sec
        self.assertEqual(SMPTETimecode.format_timecode(23.0 / 24.0, 24.0), "00:00:00:23")
        # 1 minute = 60s
        self.assertEqual(SMPTETimecode.format_timecode(60.0, 24.0), "00:01:00:00")
        # 1 hour = 3600s
        self.assertEqual(SMPTETimecode.format_timecode(3600.0, 24.0), "01:00:00:00")

    def test_standard_fps_30(self):
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 30.0), "00:00:01:00")
        self.assertEqual(SMPTETimecode.format_timecode(0.5, 30.0), "00:00:00:15")
        self.assertEqual(SMPTETimecode.format_timecode(862.6, 30.0), "00:14:22:18")

    def test_standard_fps_60(self):
        self.assertEqual(SMPTETimecode.format_timecode(1.0, 60.0), "00:00:01:00")
        self.assertEqual(SMPTETimecode.format_timecode(1.0 / 60.0, 60.0), "00:00:00:01")
        self.assertEqual(SMPTETimecode.format_timecode(59.0 / 60.0, 60.0), "00:00:00:59")

    def test_fractional_fps_23_976(self):
        fps = 24000.0 / 1001.0  # 23.976023976...
        tc1 = SMPTETimecode.format_timecode(1.0, fps)
        self.assertEqual(tc1, "00:00:01:00")
        tc_half = SMPTETimecode.format_timecode(0.5, fps)
        self.assertEqual(tc_half, "00:00:00:12")

    def test_fractional_fps_59_94(self):
        fps = 60000.0 / 1001.0  # 59.94005994...
        tc1 = SMPTETimecode.format_timecode(1.0, fps)
        self.assertEqual(tc1, "00:00:01:00")

    def test_remaining_time_formatting(self):
        duration = 3600.0  # 1 hour
        current = 600.0    # 10 min
        # Remaining is 3000s = 50min
        rem_tc = SMPTETimecode.format_remaining_timecode(current, duration, 30.0)
        self.assertEqual(rem_tc, "-00:50:00:00")

        # Current >= duration -> remaining is 0
        self.assertEqual(SMPTETimecode.format_remaining_timecode(3600.0, 3600.0, 30.0), "-00:00:00:00")
        self.assertEqual(SMPTETimecode.format_remaining_timecode(4000.0, 3600.0, 30.0), "-00:00:00:00")

    def test_bidirectional_roundtrip(self):
        fps = 25.0
        test_seconds = [0.0, 1.0, 14.56, 125.0, 3600.0, 7200.04]
        for s in test_seconds:
            tc = SMPTETimecode.format_timecode(s, fps)
            parsed_s = SMPTETimecode.parse_timecode(tc, fps)
            tc_reformatted = SMPTETimecode.format_timecode(parsed_s, fps)
            self.assertEqual(tc, tc_reformatted, f"Mismatch on roundtrip for seconds={s}")

    def test_single_frame_step_forward(self):
        duration = 100.0
        fps = 24.0
        pos = 0.0
        for _ in range(24):
            pos = SMPTETimecode.step_frame_forward(pos, duration, fps)
        self.assertAlmostEqual(pos, 1.0, delta=1e-5)
        self.assertEqual(SMPTETimecode.format_timecode(pos, fps), "00:00:01:00")

        # Stepping at boundary of duration
        end_pos = SMPTETimecode.step_frame_forward(100.0, 100.0, fps)
        self.assertEqual(end_pos, 100.0)

    def test_single_frame_step_backward(self):
        duration = 100.0
        fps = 24.0
        pos = 1.0
        for _ in range(24):
            pos = SMPTETimecode.step_frame_backward(pos, duration, fps)
        self.assertAlmostEqual(pos, 0.0, delta=1e-5)
        self.assertEqual(SMPTETimecode.format_timecode(pos, fps), "00:00:00:00")

        # Stepping backward at 0 remains 0
        zero_pos = SMPTETimecode.step_frame_backward(0.0, duration, fps)
        self.assertEqual(zero_pos, 0.0)

    def test_jump_seeks(self):
        duration = 120.0  # 2 minutes
        # +10s jump from 0
        self.assertEqual(SMPTETimecode.jump_seconds(0.0, 10.0, duration), 10.0)
        # -10s jump from 5s clamps to 0
        self.assertEqual(SMPTETimecode.jump_seconds(5.0, -10.0, duration), 0.0)
        # +30s jump from 100s clamps to duration (120s)
        self.assertEqual(SMPTETimecode.jump_seconds(100.0, 30.0, duration), 120.0)
        # -30s jump from 50s gives 20s
        self.assertEqual(SMPTETimecode.jump_seconds(50.0, -30.0, duration), 20.0)

    def test_edge_cases_large_durations(self):
        # 100 hours = 360,000 seconds
        tc_100h = SMPTETimecode.format_timecode(360000.0, 30.0)
        self.assertEqual(tc_100h, "100:00:00:00")
        parsed = SMPTETimecode.parse_timecode(tc_100h, 30.0)
        self.assertEqual(parsed, 360000.0)

    def test_invalid_timecode_parsing(self):
        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("00:00:00")  # Missing frames
        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("00:60:00:00")  # Invalid minutes
        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("00:00:60:00")  # Invalid seconds
        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("00:00:00:35", fps=30.0)  # Invalid frames for 30fps
        with self.assertRaises(ValueError):
            SMPTETimecode.parse_timecode("invalid:tc:format:xx")


if __name__ == "__main__":
    unittest.main()
