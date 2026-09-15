"""
vu_widget.py - Stereo Peak & RMS VU Meter Rack Widget for Penguin.

Features:
- Dual-channel (CH_L & CH_R) 30-segment LED ladder architecture.
- Logarithmic -60 dB to +3 dB studio calibration scale.
- Three signal zones: Nominal (Signal Lime #CCFF00), Headroom (Safety Orange #FF4400), Clip (Red #FF2200).
- Real-time peak hold needle with exponential ballistic decay.
- Instantaneous numeric dBFS readout and prominent CLIP alert badge.
"""

import math
from typing import Any, Dict, List, Optional, Tuple

from src.engine.audio_dsp import VUMeterDSP
from src.ui.theme import (
    BG_DEEP_OBSIDIAN,
    SURFACE_PANEL_BASE,
    SURFACE_RAISED,
    GRID_STRUCTURAL_BORDER,
    TEXT_HIGH_CONTRAST,
    TEXT_SECONDARY_DIM,
    TEXT_MUTED,
    ACCENT_SAFETY_ORANGE,
    ACCENT_SIGNAL_LIME,
    ACCENT_CLIP_RED,
)

VU_MIN_DB = -60.0
VU_MAX_DB = 3.0
VU_HEADROOM_DB = -3.0
VU_CLIP_DB = 0.0
VU_NUM_SEGMENTS = 30
VU_CALIBRATION_MARKS = [-60.0, -40.0, -20.0, -12.0, -6.0, -3.0, 0.0, 3.0]


class StereoVUMeterRack:
    """
    30-Segment Stereo Peak/RMS VU Meter Rack Model and Controller.
    Manages channel levels, LED segment quantization, peak hold ballistics, and clip alerts.
    """

    def __init__(
        self,
        decay_rate_db_per_sec: float = 20.0,
        peak_hold_decay_rate: float = 12.0,
        width: int = 360,
        height: int = 72,
    ):
        self._width = max(120, int(width))
        self._height = max(40, int(height))
        self._decay_rate = float(decay_rate_db_per_sec)
        self._peak_hold_decay = float(peak_hold_decay_rate)

        # Current Levels
        self._left_peak_db = VU_MIN_DB
        self._right_peak_db = VU_MIN_DB
        self._left_rms_db = VU_MIN_DB
        self._right_rms_db = VU_MIN_DB

        # Peak Hold
        self._left_peak_hold_db = VU_MIN_DB
        self._right_peak_hold_db = VU_MIN_DB

        # Clip Flags
        self._left_clipped = False
        self._right_clipped = False
        self._clip_hold_time_s = 1.5
        self._left_clip_timer = 0.0
        self._right_clip_timer = 0.0

    # Properties
    @property
    def left_peak_db(self) -> float:
        return self._left_peak_db

    @property
    def right_peak_db(self) -> float:
        return self._right_peak_db

    @property
    def left_rms_db(self) -> float:
        return self._left_rms_db

    @property
    def right_rms_db(self) -> float:
        return self._right_rms_db

    @property
    def left_peak_hold_db(self) -> float:
        return self._left_peak_hold_db

    @property
    def right_peak_hold_db(self) -> float:
        return self._right_peak_hold_db

    @property
    def is_left_clipped(self) -> bool:
        return self._left_clipped

    @property
    def is_right_clipped(self) -> bool:
        return self._right_clipped

    @property
    def is_clipped(self) -> bool:
        return self._left_clipped or self._right_clipped

    def set_levels(
        self,
        left_peak_db: float,
        right_peak_db: float,
        left_rms_db: Optional[float] = None,
        right_rms_db: Optional[float] = None,
    ):
        """Sets instantaneous stereo audio levels and updates peak holds and clips."""
        self._left_peak_db = max(VU_MIN_DB, min(float(left_peak_db), VU_MAX_DB))
        self._right_peak_db = max(VU_MIN_DB, min(float(right_peak_db), VU_MAX_DB))

        if left_rms_db is not None:
            self._left_rms_db = max(VU_MIN_DB, min(float(left_rms_db), VU_MAX_DB))
        else:
            self._left_rms_db = self._left_peak_db - 3.0

        if right_rms_db is not None:
            self._right_rms_db = max(VU_MIN_DB, min(float(right_rms_db), VU_MAX_DB))
        else:
            self._right_rms_db = self._right_peak_db - 3.0

        # Update peak holds
        if self._left_peak_db >= self._left_peak_hold_db:
            self._left_peak_hold_db = self._left_peak_db
        if self._right_peak_db >= self._right_peak_hold_db:
            self._right_peak_hold_db = self._right_peak_db

        # Check clipping
        if self._left_peak_db >= VU_CLIP_DB:
            self._left_clipped = True
            self._left_clip_timer = self._clip_hold_time_s

        if self._right_peak_db >= VU_CLIP_DB:
            self._right_clipped = True
            self._right_clip_timer = self._clip_hold_time_s

    def update_ballistics(self, dt: float = 0.016):
        """Advances ballistic needle decay and clip timer by delta time in seconds."""
        dt = max(0.0, float(dt))

        # Peak decay
        self._left_peak_db = max(VU_MIN_DB, self._left_peak_db - self._decay_rate * dt)
        self._right_peak_db = max(VU_MIN_DB, self._right_peak_db - self._decay_rate * dt)
        self._left_rms_db = max(VU_MIN_DB, self._left_rms_db - self._decay_rate * dt)
        self._right_rms_db = max(VU_MIN_DB, self._right_rms_db - self._decay_rate * dt)

        # Peak hold decay
        self._left_peak_hold_db = max(VU_MIN_DB, self._left_peak_hold_db - self._peak_hold_decay * dt)
        self._right_peak_hold_db = max(VU_MIN_DB, self._right_peak_hold_db - self._peak_hold_decay * dt)

        # Clip timers
        if self._left_clipped:
            self._left_clip_timer -= dt
            if self._left_clip_timer <= 0.0 and self._left_peak_db < VU_CLIP_DB:
                self._left_clipped = False

        if self._right_clipped:
            self._right_clip_timer -= dt
            if self._right_clip_timer <= 0.0 and self._right_peak_db < VU_CLIP_DB:
                self._right_clipped = False

    def reset(self):
        """Resets all meter levels, peak holds, and clip alerts to floor."""
        self._left_peak_db = VU_MIN_DB
        self._right_peak_db = VU_MIN_DB
        self._left_rms_db = VU_MIN_DB
        self._right_rms_db = VU_MIN_DB
        self._left_peak_hold_db = VU_MIN_DB
        self._right_peak_hold_db = VU_MIN_DB
        self._left_clipped = False
        self._right_clipped = False
        self._left_clip_timer = 0.0
        self._right_clip_timer = 0.0

    # Segment Mapping Math
    @classmethod
    def db_to_segment_count(cls, db: float, num_segments: int = VU_NUM_SEGMENTS) -> int:
        """Calculates active segment count (0 to num_segments) for a given dB level."""
        if db <= VU_MIN_DB:
            return 0
        if db >= VU_MAX_DB:
            return num_segments
        ratio = (db - VU_MIN_DB) / (VU_MAX_DB - VU_MIN_DB)
        return max(0, min(num_segments, int(math.ceil(ratio * num_segments))))

    @classmethod
    def segment_index_to_db(cls, index: int, num_segments: int = VU_NUM_SEGMENTS) -> float:
        """Returns approximate dB threshold for a given segment index (0 to num_segments-1)."""
        ratio = float(index) / float(max(1, num_segments - 1))
        return VU_MIN_DB + ratio * (VU_MAX_DB - VU_MIN_DB)

    @classmethod
    def segment_color(cls, segment_index: int, num_segments: int = VU_NUM_SEGMENTS) -> str:
        """Returns the appropriate color token for a segment index based on its dB value."""
        db = cls.segment_index_to_db(segment_index, num_segments)
        if db > VU_CLIP_DB:
            return ACCENT_CLIP_RED
        elif db > VU_HEADROOM_DB:
            return ACCENT_SAFETY_ORANGE
        else:
            return ACCENT_SIGNAL_LIME

    def get_channel_segments(self, channel: str = "L") -> List[Dict[str, Any]]:
        """
        Returns list of segment descriptors for channel ('L' or 'R').
        Each item: {'index': int, 'active': bool, 'color': str, 'db': float}
        """
        peak = self._left_peak_db if channel.upper() == "L" else self._right_peak_db
        active_count = self.db_to_segment_count(peak, VU_NUM_SEGMENTS)
        segments = []
        for i in range(VU_NUM_SEGMENTS):
            db = self.segment_index_to_db(i, VU_NUM_SEGMENTS)
            color = self.segment_color(i, VU_NUM_SEGMENTS)
            segments.append({
                "index": i,
                "active": (i < active_count),
                "color": color,
                "db": db,
            })
        return segments

    def left_db_string(self) -> str:
        """Returns formatted string for Left channel peak dB."""
        if self._left_peak_db <= -59.0:
            return "-∞ dB"
        return f"{self._left_peak_db:+.1f} dB"

    def right_db_string(self) -> str:
        """Returns formatted string for Right channel peak dB."""
        if self._right_peak_db <= -59.0:
            return "-∞ dB"
        return f"{self._right_peak_db:+.1f} dB"

    def resize(self, width: int, height: int):
        self._width = max(120, int(width))
        self._height = max(40, int(height))
