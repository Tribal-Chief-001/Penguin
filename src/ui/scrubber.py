"""
scrubber.py - Mechanical SMPTE Tick Ruler Progress Scrubber Widget for Penguin.

Features:
- Dual SMPTE timecode readouts: Elapsed (HH:MM:SS:FF) and Remaining (-HH:MM:SS:FF).
- Broadcast graduation ticks: Minor graduation ticks (6px) and major ticks (12px).
- Industrial Safety Orange (#FF4400) playhead needle with triangular flag.
- Telemetry Cyan (#00E5FF) diamond chapter markers with tooltip previews.
- Interactive scrubbing, millisecond seeking, and hover previews.
"""

from dataclasses import dataclass, field
import math
from typing import Callable, List, Optional, Tuple

from src.engine.smpte import SMPTETimecode, ms_to_smpte, smpte_to_ms
from src.ui.theme import (
    BG_DEEP_OBSIDIAN,
    SURFACE_PANEL_BASE,
    SURFACE_RAISED,
    GRID_STRUCTURAL_BORDER,
    GRID_LINE_ACTIVE,
    TEXT_HIGH_CONTRAST,
    TEXT_SECONDARY_DIM,
    TEXT_MUTED,
    ACCENT_SAFETY_ORANGE,
    ACCENT_SIGNAL_LIME,
    ACCENT_TELEMETRY_CYAN,
)


@dataclass
class ChapterMarker:
    """Represents a timeline chapter diamond marker."""
    timestamp_ms: int
    title: str = ""

    def __post_init__(self):
        self.timestamp_ms = max(0, int(self.timestamp_ms))


class MechanicalTickScrubber:
    """
    Mechanical SMPTE timecode tick ruler progress scrubber.
    Maintains full state, graduation tick math, chapter markers, and seek dispatches.
    """

    def __init__(
        self,
        duration_ms: int = 0,
        position_ms: int = 0,
        fps: float = 30.0,
        drop_frame: bool = False,
        width: int = 800,
        height: int = 38,
    ):
        self._duration_ms = max(0, int(duration_ms))
        self._position_ms = max(0, min(int(position_ms), self._duration_ms))
        self._fps = max(1.0, float(fps))
        self._drop_frame = bool(drop_frame)
        self._width = max(100, int(width))
        self._height = max(24, int(height))

        self._chapters: List[ChapterMarker] = []
        self._is_dragging = False
        self._is_hovered = False
        self._hover_x = 0

        # Margins & Layout
        self.margin_left = 110   # Width for elapsed timecode box
        self.margin_right = 110  # Width for remaining timecode box
        self.tick_spacing_px = 16

        # Signal callbacks
        self._on_seek: Optional[Callable[[int], None]] = None
        self._on_scrub_start: Optional[Callable[[], None]] = None
        self._on_scrub_end: Optional[Callable[[], None]] = None
        self._on_position_changed: Optional[Callable[[int], None]] = None

    # Properties
    @property
    def position_ms(self) -> int:
        return self._position_ms

    @position_ms.setter
    def position_ms(self, ms: int):
        clamped = max(0, min(int(ms), self._duration_ms if self._duration_ms > 0 else int(ms)))
        if self._position_ms != clamped:
            self._position_ms = clamped
            if self._on_position_changed:
                self._on_position_changed(self._position_ms)

    @property
    def duration_ms(self) -> int:
        return self._duration_ms

    @duration_ms.setter
    def duration_ms(self, ms: int):
        self._duration_ms = max(0, int(ms))
        if self._position_ms > self._duration_ms and self._duration_ms > 0:
            self.position_ms = self._duration_ms

    @property
    def fps(self) -> float:
        return self._fps

    @fps.setter
    def fps(self, val: float):
        self._fps = max(1.0, float(val))

    @property
    def is_drop_frame(self) -> bool:
        return self._drop_frame

    @is_drop_frame.setter
    def is_drop_frame(self, val: bool):
        self._drop_frame = bool(val)

    @property
    def is_scrubbing(self) -> bool:
        return self._is_dragging

    @property
    def chapters(self) -> List[ChapterMarker]:
        return list(self._chapters)

    # Callbacks
    def set_on_seek(self, callback: Optional[Callable[[int], None]]):
        self._on_seek = callback

    def set_on_scrub_start(self, callback: Optional[Callable[[], None]]):
        self._on_scrub_start = callback

    def set_on_scrub_end(self, callback: Optional[Callable[[], None]]):
        self._on_scrub_end = callback

    def set_on_position_changed(self, callback: Optional[Callable[[int], None]]):
        self._on_position_changed = callback

    # Chapter operations
    def add_chapter(self, timestamp_ms: int, title: str = ""):
        marker = ChapterMarker(timestamp_ms=timestamp_ms, title=title)
        self._chapters.append(marker)
        self._chapters.sort(key=lambda c: c.timestamp_ms)

    def clear_chapters(self):
        self._chapters.clear()

    def set_chapters(self, chapters: List[ChapterMarker]):
        self._chapters = sorted(chapters, key=lambda c: c.timestamp_ms)

    # Geometry & Calculations
    @property
    def track_left(self) -> int:
        return self.margin_left

    @property
    def track_right(self) -> int:
        return self._width - self.margin_right

    @property
    def track_width(self) -> int:
        return max(1, self.track_right - self.track_left)

    def position_to_x(self, pos_ms: int) -> int:
        """Converts millisecond position to X pixel coordinate."""
        if self._duration_ms <= 0:
            return self.track_left
        ratio = max(0.0, min(1.0, float(pos_ms) / float(self._duration_ms)))
        return int(self.track_left + ratio * self.track_width)

    def x_to_position(self, x: int) -> int:
        """Converts X pixel coordinate to millisecond position."""
        if self.track_width <= 0 or self._duration_ms <= 0:
            return 0
        clamped_x = max(self.track_left, min(x, self.track_right))
        ratio = float(clamped_x - self.track_left) / float(self.track_width)
        return int(ratio * self._duration_ms)

    # SMPTE Readouts
    def elapsed_smpte(self) -> str:
        """Returns elapsed SMPTE timecode (HH:MM:SS:FF)."""
        return ms_to_smpte(self._position_ms, self._fps, self._drop_frame)

    def remaining_smpte(self) -> str:
        """Returns remaining SMPTE timecode (-HH:MM:SS:FF)."""
        pos_sec = self._position_ms / 1000.0
        dur_sec = self._duration_ms / 1000.0
        return SMPTETimecode.format_remaining_timecode(pos_sec, dur_sec, self._fps, self._drop_frame)

    def graduation_ticks(self) -> List[Tuple[int, int, bool]]:
        """
        Generates list of graduation tick coordinates for the ruler track.
        Returns tuples of (x_pos, tick_height_px, is_major).
        """
        ticks: List[Tuple[int, int, bool]] = []
        count = self.track_width // self.tick_spacing_px
        for i in range(count + 1):
            x = self.track_left + i * self.tick_spacing_px
            if x > self.track_right:
                break
            is_major = (i % 5 == 0)
            height = 12 if is_major else 6
            ticks.append((x, height, is_major))
        return ticks

    # Mouse & Interactive Actions
    def handle_mouse_press(self, x: int, y: int) -> int:
        """Handles mouse press down on scrubber."""
        self._is_dragging = True
        if self._on_scrub_start:
            self._on_scrub_start()
        target_ms = self.x_to_position(x)
        self.position_ms = target_ms
        if self._on_seek:
            self._on_seek(target_ms)
        return target_ms

    def handle_mouse_move(self, x: int, y: int) -> Optional[int]:
        """Handles mouse drag or hover movement."""
        self._hover_x = x
        self._is_hovered = True
        if self._is_dragging:
            target_ms = self.x_to_position(x)
            self.position_ms = target_ms
            if self._on_seek:
                self._on_seek(target_ms)
            return target_ms
        return None

    def handle_mouse_release(self, x: int, y: int) -> int:
        """Handles mouse button release."""
        target_ms = self.x_to_position(x)
        self._is_dragging = False
        if self._on_scrub_end:
            self._on_scrub_end()
        return target_ms

    def handle_leave(self):
        """Handles mouse cursor leaving widget."""
        self._is_hovered = False
        self._is_dragging = False

    def hover_timestamp_ms(self) -> int:
        """Returns millisecond timestamp under current hover cursor."""
        return self.x_to_position(self._hover_x)

    def hover_smpte(self) -> str:
        """Returns formatted SMPTE timecode under current hover cursor."""
        return ms_to_smpte(self.hover_timestamp_ms(), self._fps, self._drop_frame)

    def resize(self, width: int, height: int):
        """Resizes the widget dimensions."""
        self._width = max(100, int(width))
        self._height = max(24, int(height))
