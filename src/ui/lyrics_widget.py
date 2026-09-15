"""
lyrics_widget.py - Synchronized .lrc Lyrics Teleprompter Widget for Penguin.

Features:
- Standard and enhanced .lrc cue timestamp parsing and header metadata extraction.
- Millisecond-precision active cue tracking via binary search.
- Brutalist visual hierarchy: Active line in Signal Lime (#CCFF00) with focus indicator, past lines dim (#444455), upcoming lines (#777788).
- Auto-scrolling viewport centering animation geometry.
- Interactive Click-to-Seek dispatching exact millisecond seek requests.
"""

from typing import Any, Callable, Dict, List, Optional, Tuple

from src.engine.lrc_parser import LRCParser, LRCCue, LRCToken
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


class SynchronizedLyricsTeleprompter:
    """
    Synchronized .lrc Lyrics Teleprompter Model and Viewport Controller.
    Handles cue synchronization, active line animation, and click-to-seek.
    """

    def __init__(self, width: int = 400, height: int = 300):
        self._width = max(200, int(width))
        self._height = max(150, int(height))
        self._parser = LRCParser()
        self._position_ms = 0
        self._active_cue_index = -1
        self._scroll_offset_px = 0.0
        self._line_height_px = 32

        # Signal Callbacks
        self._on_active_cue_changed: Optional[Callable[[int, Optional[LRCCue]], None]] = None
        self._on_seek_requested: Optional[Callable[[int], None]] = None

    @property
    def cues(self) -> List[LRCCue]:
        return self._parser.cues

    @property
    def cue_count(self) -> int:
        return len(self._parser.cues)

    @property
    def metadata(self) -> Dict[str, str]:
        return self._parser.metadata

    @property
    def active_cue_index(self) -> int:
        return self._active_cue_index

    @property
    def active_cue(self) -> Optional[LRCCue]:
        if 0 <= self._active_cue_index < len(self._parser.cues):
            return self._parser.cues[self._active_cue_index]
        return None

    @property
    def position_ms(self) -> int:
        return self._position_ms

    @property
    def scroll_offset_px(self) -> float:
        return self._scroll_offset_px

    # Callbacks
    def set_on_active_cue_changed(self, cb: Optional[Callable[[int, Optional[LRCCue]], None]]):
        self._on_active_cue_changed = cb

    def set_on_seek_requested(self, cb: Optional[Callable[[int], None]]):
        self._on_seek_requested = cb

    # Loading
    def load_lrc_content(self, lrc_text: str) -> bool:
        """Parses LRC formatted text string."""
        self._parser = LRCParser.parse_string(lrc_text)
        self._active_cue_index = -1
        self._update_active_cue()
        return len(self._parser.cues) > 0

    def load_lrc_file(self, file_path: str) -> bool:
        """Loads and parses an external .lrc file."""
        self._parser = LRCParser.parse_file(file_path)
        self._active_cue_index = -1
        self._update_active_cue()
        return len(self._parser.cues) > 0

    def clear(self):
        """Clears all loaded lyrics."""
        self._parser = LRCParser()
        self._active_cue_index = -1
        self._scroll_offset_px = 0.0

    # Synchronization
    def set_position_ms(self, pos_ms: int):
        """Updates current playback position and active line index."""
        self._position_ms = max(0, int(pos_ms))
        self._update_active_cue()

    def _update_active_cue(self):
        new_index = self._parser.get_active_cue_index(self._position_ms)
        if new_index != self._active_cue_index:
            self._active_cue_index = new_index
            self._recalculate_scroll_target()
            if self._on_active_cue_changed:
                cue = self.active_cue
                self._on_active_cue_changed(self._active_cue_index, cue)

    def _recalculate_scroll_target(self):
        if self._active_cue_index >= 0:
            center_y = self._height / 2.0
            target_y = self._active_cue_index * self._line_height_px
            self._scroll_offset_px = max(0.0, target_y - center_y + (self._line_height_px / 2.0))
        else:
            self._scroll_offset_px = 0.0

    def cue_at(self, index: int) -> Optional[LRCCue]:
        if 0 <= index < len(self._parser.cues):
            return self._parser.cues[index]
        return None

    # Line Item Query for Rendering
    def get_rendered_lines(self) -> List[Dict[str, Any]]:
        """
        Returns structured list of lyric lines with styling properties.
        """
        lines = []
        for idx, cue in enumerate(self._parser.cues):
            if idx == self._active_cue_index:
                state = "active"
                color = ACCENT_SIGNAL_LIME
                prefix = "► "
            elif idx < self._active_cue_index:
                state = "past"
                color = TEXT_MUTED
                prefix = "  "
            else:
                state = "upcoming"
                color = TEXT_SECONDARY_DIM
                prefix = "  "

            lines.append({
                "index": idx,
                "time_ms": cue.time_ms,
                "text": cue.text,
                "formatted_text": f"{prefix}{cue.text}",
                "state": state,
                "color": color,
                "y_pos": idx * self._line_height_px - self._scroll_offset_px,
            })
        return lines

    # Interactive Click-to-Seek
    def handle_click_line(self, cue_index: int) -> Optional[int]:
        """Dispatches seek request to timestamp of clicked line."""
        cue = self.cue_at(cue_index)
        if cue:
            target_ms = cue.time_ms
            if self._on_seek_requested:
                self._on_seek_requested(target_ms)
            return target_ms
        return None

    def handle_click_y(self, y: int) -> Optional[int]:
        """Maps Y pixel coordinate to a cue row and triggers seek."""
        abs_y = y + self._scroll_offset_px
        index = int(abs_y // self._line_height_px)
        return self.handle_click_line(index)

    def resize(self, width: int, height: int):
        self._width = max(200, int(width))
        self._height = max(150, int(height))
        self._recalculate_scroll_target()
