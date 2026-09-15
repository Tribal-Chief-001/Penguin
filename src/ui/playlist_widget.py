"""
playlist_widget.py - Playlist Queue Matrix Widget for Penguin.

Features:
- 6-column Matrix Schema (#, TITLE, ARTIST, ALBUM, DUR, FORMAT).
- Active track row glow styling with Signal Lime (#CCFF00) indicator (►).
- Real-time search filter matching across all text metadata.
- Queue operations: Insertion, deletion, drag reordering, and randomized shuffle.
"""

from dataclasses import dataclass
import random
from typing import Any, Callable, Dict, List, Optional, Tuple

from src.engine.smpte import SMPTETimecode
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

PLAYLIST_COLUMNS = ["#", "TITLE", "ARTIST", "ALBUM", "DUR", "FORMAT"]


@dataclass
class PlaylistItem:
    """Represents a single track entry in the playlist queue matrix."""
    file_path: str
    title: str = ""
    artist: str = ""
    album: str = ""
    duration_ms: int = 0
    format: str = ""
    order: int = 0

    @property
    def formatted_duration(self) -> str:
        """Formats duration_ms into MM:SS."""
        total_sec = max(0, self.duration_ms // 1000)
        minutes = total_sec // 60
        seconds = total_sec % 60
        return f"{minutes:02d}:{seconds:02d}"


class PlaylistMatrixWidget:
    """
    Playlist Queue Matrix Widget and Model Controller.
    Manages queue items, active selection, search filtering, reordering, and playback triggers.
    """

    def __init__(self, width: int = 500, height: int = 300):
        self._width = max(200, int(width))
        self._height = max(100, int(height))
        self._items: List[PlaylistItem] = []
        self._current_index = -1
        self._search_query = ""

        # Signal Callbacks
        self._on_playlist_changed: Optional[Callable[[], None]] = None
        self._on_current_index_changed: Optional[Callable[[int], None]] = None
        self._on_track_double_clicked: Optional[Callable[[int, PlaylistItem], None]] = None

    @property
    def items(self) -> List[PlaylistItem]:
        return list(self._items)

    @property
    def count(self) -> int:
        return len(self._items)

    @property
    def current_index(self) -> int:
        return self._current_index

    @property
    def current_item(self) -> Optional[PlaylistItem]:
        if 0 <= self._current_index < len(self._items):
            return self._items[self._current_index]
        return None

    @property
    def search_query(self) -> str:
        return self._search_query

    @classmethod
    def columns(cls) -> List[str]:
        return list(PLAYLIST_COLUMNS)

    # Callbacks
    def set_on_playlist_changed(self, cb: Optional[Callable[[], None]]):
        self._on_playlist_changed = cb

    def set_on_current_index_changed(self, cb: Optional[Callable[[int], None]]):
        self._on_current_index_changed = cb

    def set_on_track_double_clicked(self, cb: Optional[Callable[[int, PlaylistItem], None]]):
        self._on_track_double_clicked = cb

    # Item Management
    def add_item(
        self,
        file_path: str,
        title: str = "",
        artist: str = "",
        album: str = "",
        duration_ms: int = 0,
        format: str = "",
    ) -> PlaylistItem:
        """Appends a new track to the playlist queue."""
        order = len(self._items)
        if not title:
            # Fallback to filename
            title = file_path.split("/")[-1]
        item = PlaylistItem(
            file_path=file_path,
            title=title,
            artist=artist,
            album=album,
            duration_ms=duration_ms,
            format=format.upper() if format else (file_path.split(".")[-1].upper() if "." in file_path else "MEDIA"),
            order=order,
        )
        self._items.append(item)
        if self._on_playlist_changed:
            self._on_playlist_changed()
        return item

    def insert_item(self, index: int, item: PlaylistItem):
        index = max(0, min(index, len(self._items)))
        self._items.insert(index, item)
        self._reindex_orders()
        if self._on_playlist_changed:
            self._on_playlist_changed()

    def remove_item(self, index: int) -> Optional[PlaylistItem]:
        """Removes track at index."""
        if 0 <= index < len(self._items):
            removed = self._items.pop(index)
            if self._current_index == index:
                if len(self._items) == 0:
                    self._current_index = -1
                elif self._current_index >= len(self._items):
                    self._current_index = len(self._items) - 1
            elif self._current_index > index:
                self._current_index -= 1
            self._reindex_orders()
            if self._on_playlist_changed:
                self._on_playlist_changed()
            return removed
        return None

    def move_item(self, from_index: int, to_index: int):
        """Moves track position in queue."""
        if 0 <= from_index < len(self._items) and 0 <= to_index < len(self._items):
            item = self._items.pop(from_index)
            self._items.insert(to_index, item)
            if self._current_index == from_index:
                self._current_index = to_index
            elif from_index < self._current_index <= to_index:
                self._current_index -= 1
            elif to_index <= self._current_index < from_index:
                self._current_index += 1
            self._reindex_orders()
            if self._on_playlist_changed:
                self._on_playlist_changed()

    def clear_playlist(self):
        """Clears all tracks from queue."""
        self._items.clear()
        self._current_index = -1
        if self._on_playlist_changed:
            self._on_playlist_changed()

    def shuffle(self, seed: Optional[int] = None):
        """Randomly shuffles the playlist items."""
        if len(self._items) <= 1:
            return
        rng = random.Random(seed)
        current_item = self.current_item
        rng.shuffle(self._items)
        if current_item and current_item in self._items:
            self._current_index = self._items.index(current_item)
        self._reindex_orders()
        if self._on_playlist_changed:
            self._on_playlist_changed()

    def item_at(self, index: int) -> Optional[PlaylistItem]:
        if 0 <= index < len(self._items):
            return self._items[index]
        return None

    def set_current_index(self, index: int):
        clamped = max(-1, min(index, len(self._items) - 1))
        if self._current_index != clamped:
            self._current_index = clamped
            if self._on_current_index_changed:
                self._on_current_index_changed(self._current_index)

    def trigger_double_click(self, index: int):
        """Simulates user double-clicking a track to initiate playback."""
        if 0 <= index < len(self._items):
            self.set_current_index(index)
            if self._on_track_double_clicked:
                self._on_track_double_clicked(index, self._items[index])

    # Search & Filtering
    def set_search_filter(self, query: str):
        self._search_query = query.strip()

    def filtered_items(self) -> List[Tuple[int, PlaylistItem]]:
        """
        Returns list of (original_index, PlaylistItem) matching current search filter.
        """
        if not self._search_query:
            return list(enumerate(self._items))
        q = self._search_query.lower()
        results = []
        for idx, item in enumerate(self._items):
            if (
                q in item.title.lower()
                or q in item.artist.lower()
                or q in item.album.lower()
                or q in item.format.lower()
                or q in str(item.order + 1)
            ):
                results.append((idx, item))
        return results

    def _reindex_orders(self):
        for idx, item in enumerate(self._items):
            item.order = idx

    def resize(self, width: int, height: int):
        self._width = max(200, int(width))
        self._height = max(100, int(height))
