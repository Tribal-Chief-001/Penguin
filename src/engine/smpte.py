"""
smpte.py - SMPTE 12M Timecode Calculation & Conversion Engine.

Implements standard SMPTE 12M specifications:
- Non-Drop Frame (NDF) formatting: HH:MM:SS:FF (24, 25, 30, 50, 60, 23.976 fps).
- Drop-Frame (DF) formatting: HH:MM:SS;FF (29.97 and 59.94 fps).
  - Drops 2 frames/minute (29.97 fps) or 4 frames/minute (59.94 fps).
  - 10-minute exception: no frames dropped when minute % 10 == 0.
- Bidirectional conversions (ms/seconds <-> SMPTE string).
- Single-frame stepping (+/- 1F) and relative jumps.
- Negative timestamp and large duration (>100h) handling.
"""

from __future__ import annotations
import math
import re
from typing import Tuple, Union


class SMPTETimecode:
    """SMPTE 12M Timecode engine supporting NDF and DF standards."""

    @staticmethod
    def nominal_fps(fps: float) -> int:
        """Returns integer nominal FPS."""
        if fps <= 0.0:
            return 30
        return int(round(fps))

    @staticmethod
    def is_drop_frame_rate(fps: float) -> bool:
        """Determines if a frame rate is nominally drop-frame (29.97 or 59.94)."""
        return (
            abs(fps - 29.97) < 0.01
            or abs(fps - (30000.0 / 1001.0)) < 0.001
            or abs(fps - 59.94) < 0.01
            or abs(fps - (60000.0 / 1001.0)) < 0.001
        )

    @classmethod
    def frames_to_ms(cls, frames: int, fps: float = 30.0) -> int:
        """Converts total frames to milliseconds."""
        if fps <= 0.0:
            fps = 30.0
        return int(round((frames * 1000.0) / fps))

    @classmethod
    def ms_to_frames(cls, position_ms: float, fps: float = 30.0) -> int:
        """Converts milliseconds to total frame count."""
        if fps <= 0.0:
            fps = 30.0
        return int(math.floor((position_ms / 1000.0) * fps + 0.5))

    @classmethod
    def seconds_to_frame_count(cls, seconds: float, fps: float = 30.0) -> int:
        """Converts seconds to frame count."""
        if seconds <= 0.0 or fps <= 0.0:
            return 0
        return int(math.floor(seconds * fps + 0.5))

    @classmethod
    def frame_count_to_seconds(cls, frame_count: int, fps: float = 30.0) -> float:
        """Converts frame count to seconds."""
        if frame_count <= 0 or fps <= 0.0:
            return 0.0
        return float(frame_count) / fps

    @classmethod
    def format_timecode(
        cls,
        position: float,
        fps: float = 30.0,
        drop_frame: bool = False,
        is_milliseconds: bool = False,
    ) -> str:
        """
        Formats seconds (default) or milliseconds into SMPTE timecode string:
        - Non-Drop Frame: HH:MM:SS:FF
        - Drop-Frame: HH:MM:SS;FF
        """
        if fps <= 0.0:
            fps = 30.0

        is_negative = position < 0
        abs_pos = abs(position)

        if is_milliseconds:
            seconds = abs_pos / 1000.0
        else:
            seconds = abs_pos

        fps_int = cls.nominal_fps(fps)
        if fps_int <= 0:
            fps_int = 30

        use_drop_frame = drop_frame and cls.is_drop_frame_rate(fps)

        if use_drop_frame:
            drop_frames_per_minute = 4 if fps_int == 60 else 2
            frames_per_10min = 35964 if fps_int == 60 else 17982
            frames_per_min0 = fps_int * 60
            frames_per_min_rem = fps_int * 60 - drop_frames_per_minute

            total_frames = int(math.floor(seconds * fps + 0.5))
            d = total_frames // frames_per_10min
            m = total_frames % frames_per_10min

            adjusted_frames = total_frames + (drop_frames_per_minute * 9 * d)
            if m >= frames_per_min0:
                adjusted_frames += drop_frames_per_minute * (
                    1 + (m - frames_per_min0) // frames_per_min_rem
                )

            ff = adjusted_frames % fps_int
            ss = (adjusted_frames // fps_int) % 60
            mm = (adjusted_frames // (fps_int * 60)) % 60
            hh = adjusted_frames // (fps_int * 3600)

            sign = "-" if is_negative else ""
            return f"{sign}{hh:02d}:{mm:02d}:{ss:02d};{ff:02d}"
        else:
            total_frames = int(math.floor(seconds * fps + 0.5))
            ff = total_frames % fps_int
            ss = (total_frames // fps_int) % 60
            mm = (total_frames // (fps_int * 60)) % 60
            hh = total_frames // (fps_int * 3600)

            sign = "-" if is_negative else ""
            return f"{sign}{hh:02d}:{mm:02d}:{ss:02d}:{ff:02d}"

    @classmethod
    def format_timecode_ms(cls, position_ms: float, fps: float = 30.0, drop_frame: bool = False) -> str:
        """Formats milliseconds into SMPTE timecode."""
        return cls.format_timecode(position_ms, fps=fps, drop_frame=drop_frame, is_milliseconds=True)

    @classmethod
    def format_remaining_timecode(
        cls,
        current_sec: float,
        duration_sec: float,
        fps: float = 30.0,
        drop_frame: bool = False,
    ) -> str:
        """Formats remaining time into SMPTE timecode -HH:MM:SS:FF."""
        rem_sec = max(0.0, duration_sec - max(0.0, current_sec))
        tc = cls.format_timecode(rem_sec, fps, drop_frame=drop_frame)
        return f"-{tc}"

    @classmethod
    def format_remaining_timecode_ms(
        cls,
        current_ms: float,
        duration_ms: float,
        fps: float = 30.0,
        drop_frame: bool = False,
    ) -> str:
        """Formats remaining time in milliseconds into SMPTE timecode -HH:MM:SS:FF."""
        rem_ms = max(0.0, duration_ms - max(0.0, current_ms))
        tc = cls.format_timecode_ms(rem_ms, fps, drop_frame=drop_frame)
        return f"-{tc}"

    @classmethod
    def parse_timecode(cls, tc_str: str, fps: float = 30.0) -> float:
        """
        Parses HH:MM:SS:FF or HH:MM:SS;FF into total seconds.
        Raises ValueError if format is invalid.
        """
        if fps <= 0.0:
            fps = 30.0

        clean = tc_str.strip()
        is_negative = clean.startswith("-")
        if is_negative:
            clean = clean[1:]

        is_df = ";" in clean
        normalized = clean.replace(";", ":")
        parts = normalized.split(":")

        if len(parts) != 4:
            raise ValueError(f"Invalid SMPTE timecode format: '{tc_str}', expected HH:MM:SS:FF or HH:MM:SS;FF")

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
            raise ValueError(f"Frames out of range [0, {n_nominal - 1}]: {ff}")

        if is_df and cls.is_drop_frame_rate(fps):
            drop_frames_per_minute = 4 if n_nominal == 60 else 2
            total_minutes = 60 * hh + mm
            drop_count = drop_frames_per_minute * (total_minutes - (total_minutes // 10))
            total_frames = (hh * 3600 + mm * 60 + ss) * n_nominal + ff - drop_count
        else:
            total_frames = (hh * 3600 * n_nominal) + (mm * 60 * n_nominal) + (ss * n_nominal) + ff

        sec = cls.frame_count_to_seconds(total_frames, fps)
        return -sec if is_negative else sec

    @classmethod
    def timecode_to_ms(cls, timecode: str, fps: float = 30.0) -> int:
        """Converts timecode string into milliseconds (robust parsing with fallback)."""
        if fps <= 0.0:
            fps = 30.0
        fps_int = cls.nominal_fps(fps)
        if fps_int <= 0:
            fps_int = 30

        clean = timecode.strip()
        is_negative = clean.startswith("-")
        if is_negative:
            clean = clean[1:]

        is_drop_frame = ";" in clean
        clean = clean.replace(";", ":").replace(".", ":")
        parts = clean.split(":")

        if len(parts) == 4:
            try:
                hh, mm, ss, ff = int(parts[0]), int(parts[1]), int(parts[2]), int(parts[3])
            except ValueError:
                return 0

            if is_drop_frame and cls.is_drop_frame_rate(fps):
                drop_frames_per_minute = 4 if fps_int == 60 else 2
                total_minutes = 60 * hh + mm
                drop_count = drop_frames_per_minute * (total_minutes - (total_minutes // 10))
                total_frames = (hh * 3600 + mm * 60 + ss) * fps_int + ff - drop_count
            else:
                total_frames = (hh * 3600 + mm * 60 + ss) * fps_int + ff

            ms = cls.frames_to_ms(total_frames, fps)
            return -ms if is_negative else ms
        elif len(parts) == 3:
            try:
                p1, p2, p3 = int(parts[0]), int(parts[1]), int(parts[2])
                ms = (p1 * 3600 + p2 * 60 + p3) * 1000
                return -ms if is_negative else ms
            except ValueError:
                return 0
        elif len(parts) == 2:
            try:
                p1, p2 = int(parts[0]), int(parts[1])
                ms = (p1 * 60 + p2) * 1000
                return -ms if is_negative else ms
            except ValueError:
                return 0
        return 0

    @classmethod
    def step_frame_forward(cls, current_sec: float, duration_sec: float, fps: float = 30.0) -> float:
        """Calculates new timestamp after stepping 1 frame forward (+1F), clamped to duration."""
        if fps <= 0.0:
            fps = 30.0
        delta = 1.0 / fps
        return min(duration_sec, max(0.0, current_sec + delta))

    @classmethod
    def step_frame_backward(cls, current_sec: float, duration_sec: float, fps: float = 30.0) -> float:
        """Calculates new timestamp after stepping 1 frame backward (-1F), clamped to 0."""
        if fps <= 0.0:
            fps = 30.0
        delta = 1.0 / fps
        return max(0.0, min(duration_sec, current_sec - delta))

    @classmethod
    def step_frame_forward_ms(cls, current_ms: float, duration_ms: float, fps: float = 30.0) -> float:
        """Steps 1 frame forward in milliseconds."""
        if fps <= 0.0:
            fps = 30.0
        delta_ms = 1000.0 / fps
        return min(duration_ms, max(0.0, current_ms + delta_ms))

    @classmethod
    def step_frame_backward_ms(cls, current_ms: float, duration_ms: float, fps: float = 30.0) -> float:
        """Steps 1 frame backward in milliseconds."""
        if fps <= 0.0:
            fps = 30.0
        delta_ms = 1000.0 / fps
        return max(0.0, min(duration_ms, current_ms - delta_ms))

    @classmethod
    def jump_seconds(cls, current_sec: float, offset_sec: float, duration_sec: float) -> float:
        """Calculates relative seek jump (+/- 10s, +/- 30s), clamped to [0, duration]."""
        target = current_sec + offset_sec
        return max(0.0, min(duration_sec, target))

    @classmethod
    def jump_ms(cls, current_ms: float, offset_ms: float, duration_ms: float) -> float:
        """Calculates relative seek jump in ms (+/- 10000ms), clamped to [0, duration_ms]."""
        target = current_ms + offset_ms
        return max(0.0, min(duration_ms, target))


# Top-level helper functions for direct access
def ms_to_smpte(ms: float, fps: float = 30.0, drop_frame: bool = False) -> str:
    return SMPTETimecode.format_timecode_ms(ms, fps=fps, drop_frame=drop_frame)


def smpte_to_ms(tc_str: str, fps: float = 30.0) -> int:
    return SMPTETimecode.timecode_to_ms(tc_str, fps=fps)


def smpte_step_forward(current_ms: float, duration_ms: float, fps: float = 30.0) -> float:
    return SMPTETimecode.step_frame_forward_ms(current_ms, duration_ms, fps=fps)


def smpte_step_backward(current_ms: float, duration_ms: float, fps: float = 30.0) -> float:
    return SMPTETimecode.step_frame_backward_ms(current_ms, duration_ms, fps=fps)
