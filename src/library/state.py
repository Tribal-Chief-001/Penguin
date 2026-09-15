"""
state.py - State Persistence Layer for Penguin Desktop Media Player.
Tactile Digital Brutalism // Studio Precision State Engine.

Provides:
- Seamless cross-session restoration of application state.
- Window geometry and multi-monitor placement tracking.
- Audio volume, mute, playback speed, and 10-band equalizer settings persistence.
- Active UI mode (Viewfinder Video vs Hi-Fi Audio Deck) and telemetry preferences.
- Playlist matrix queue preservation and current track index tracking.
- Media playback resume bookmark position tracking.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Union

from src.library.db import DatabaseManager, get_default_db_path


class StatePersistence:
    """
    State Persistence Layer.
    Binds high-level player components (window geometry, transport, EQ, playlists)
    to the underlying SQLite WAL DatabaseManager.
    """

    DEFAULT_WINDOW_GEOMETRY = {
        "x": 100,
        "y": 100,
        "width": 1280,
        "height": 720,
        "is_maximized": False,
        "is_fullscreen": False,
    }

    DEFAULT_AUDIO_SETTINGS = {
        "volume": 80,
        "is_muted": False,
        "speed": 1.0,
        "eq_preset": "Flat",
        "eq_gains": [0.0] * 10,
    }

    DEFAULT_TELEMETRY_PREFERENCES = {
        "osd_enabled": True,
        "reticles_enabled": True,
    }

    def __init__(self, db_manager: Optional[DatabaseManager] = None):
        """
        Initializes StatePersistence with a DatabaseManager instance.
        If db_manager is None, instantiates a default DatabaseManager.
        """
        if db_manager is None:
            self._db = DatabaseManager()
            self._owns_db = True
        else:
            self._db = db_manager
            self._owns_db = False

    @property
    def db(self) -> DatabaseManager:
        """Returns the underlying DatabaseManager instance."""
        return self._db

    def close(self):
        """Closes the underlying database if owned."""
        if self._owns_db and self._db.is_open():
            self._db.close()

    def __enter__(self) -> StatePersistence:
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    # =========================================================================
    # 1. WINDOW GEOMETRY & DISPLAY STATE
    # =========================================================================

    def save_window_state(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        is_maximized: bool = False,
        is_fullscreen: bool = False,
    ) -> bool:
        """Persists window geometry coordinates and display flags."""
        geom = {
            "x": int(x),
            "y": int(y),
            "width": max(320, int(width)),
            "height": max(240, int(height)),
            "is_maximized": bool(is_maximized),
            "is_fullscreen": bool(is_fullscreen),
        }
        return self._db.set_setting("window_geometry", geom)

    def restore_window_state(self, default_geometry: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Restores window geometry from database.
        Falls back to default geometry if not present.
        """
        defaults = dict(default_geometry or self.DEFAULT_WINDOW_GEOMETRY)
        saved = self._db.get_setting("window_geometry", defaults)
        if isinstance(saved, dict):
            return {
                "x": saved.get("x", defaults["x"]),
                "y": saved.get("y", defaults["y"]),
                "width": max(320, saved.get("width", defaults["width"])),
                "height": max(240, saved.get("height", defaults["height"])),
                "is_maximized": bool(saved.get("is_maximized", defaults["is_maximized"])),
                "is_fullscreen": bool(saved.get("is_fullscreen", defaults["is_fullscreen"])),
            }
        return defaults

    # =========================================================================
    # 2. AUDIO & EQUALIZER SETTINGS
    # =========================================================================

    def save_audio_settings(
        self,
        volume: int,
        is_muted: bool = False,
        speed: float = 1.0,
        eq_preset: str = "Flat",
        eq_gains: Optional[List[float]] = None,
    ) -> bool:
        """Persists audio volume, mute state, playback speed, and EQ configuration."""
        gains = list(eq_gains) if eq_gains is not None and len(eq_gains) == 10 else [0.0] * 10
        payload = {
            "volume": max(0, min(100, int(volume))),
            "is_muted": bool(is_muted),
            "speed": max(0.25, min(4.0, float(speed))),
            "eq_preset": str(eq_preset or "Flat"),
            "eq_gains": [float(g) for g in gains],
        }
        return self._db.set_setting("audio_settings", payload)

    def restore_audio_settings(self) -> Dict[str, Any]:
        """Restores audio and EQ settings."""
        saved = self._db.get_setting("audio_settings", self.DEFAULT_AUDIO_SETTINGS)
        if isinstance(saved, dict):
            gains = saved.get("eq_gains", [0.0] * 10)
            if not isinstance(gains, list) or len(gains) != 10:
                gains = [0.0] * 10
            return {
                "volume": max(0, min(100, int(saved.get("volume", 80)))),
                "is_muted": bool(saved.get("is_muted", False)),
                "speed": max(0.25, min(4.0, float(saved.get("speed", 1.0)))),
                "eq_preset": str(saved.get("eq_preset", "Flat")),
                "eq_gains": [float(g) for g in gains],
            }
        return dict(self.DEFAULT_AUDIO_SETTINGS)

    # =========================================================================
    # 3. UI MODE & TELEMETRY PREFERENCES
    # =========================================================================

    def save_ui_mode(self, mode: Union[str, int]) -> bool:
        """Persists active UI mode ('video' or 'audio' or integer 0/1)."""
        mode_str = "audio" if str(mode) in ("1", "audio", "HiFiAudioDeck") else "video"
        return self._db.set_setting("ui_mode", mode_str)

    def restore_ui_mode(self, default_mode: str = "video") -> str:
        """Restores active UI mode ('video' or 'audio')."""
        val = self._db.get_setting("ui_mode", default_mode)
        return "audio" if str(val) in ("1", "audio", "HiFiAudioDeck") else "video"

    def save_telemetry_preferences(self, osd_enabled: bool, reticles_enabled: bool) -> bool:
        """Persists OSD telemetry HUD and safe-area reticles visibility."""
        prefs = {
            "osd_enabled": bool(osd_enabled),
            "reticles_enabled": bool(reticles_enabled),
        }
        return self._db.set_setting("telemetry_preferences", prefs)

    def restore_telemetry_preferences(self) -> Dict[str, bool]:
        """Restores telemetry HUD and reticles visibility settings."""
        saved = self._db.get_setting("telemetry_preferences", self.DEFAULT_TELEMETRY_PREFERENCES)
        if isinstance(saved, dict):
            return {
                "osd_enabled": bool(saved.get("osd_enabled", True)),
                "reticles_enabled": bool(saved.get("reticles_enabled", True)),
            }
        return dict(self.DEFAULT_TELEMETRY_PREFERENCES)

    # =========================================================================
    # 4. PLAYLIST QUEUE & REORDERING
    # =========================================================================

    def save_current_playlist(self, items: List[Dict[str, Any]], current_index: int = -1) -> bool:
        """Persists current playlist items and active track index."""
        ok = self._db.save_playlist_items("default", items)
        self._db.set_setting("current_playlist_index", int(current_index))
        return ok

    def restore_current_playlist(self) -> Tuple[List[Dict[str, Any]], int]:
        """Restores current playlist items and active track index."""
        items = self._db.get_playlist_items("default")
        current_index = int(self._db.get_setting("current_playlist_index", -1))
        if items and (current_index < 0 or current_index >= len(items)):
            current_index = 0
        elif not items:
            current_index = -1
        return items, current_index

    # =========================================================================
    # 5. PLAYBACK RESUME & BOOKMARKS
    # =========================================================================

    def record_playback(
        self,
        uri: str,
        title: str = "",
        artist: str = "",
        album: str = "",
        duration_ms: int = 0,
        position_ms: int = 0,
        media_type: str = "audio",
        file_size: int = 0,
        cover_art_path: str = "",
    ) -> bool:
        """Records playback progress and updates resume bookmark in history."""
        return self._db.record_playback(
            uri=uri,
            title=title,
            artist=artist,
            album=album,
            duration_ms=duration_ms,
            position_ms=position_ms,
            media_type=media_type,
            file_size=file_size,
            cover_art_path=cover_art_path,
        )

    def get_resume_position(self, uri: str) -> int:
        """Retrieves last saved playback position in milliseconds or 0."""
        item = self._db.get_history_item(uri)
        if item:
            return int(item.get("last_position_ms", 0))
        return 0

    def save_playback_bookmark(self, uri: str, position_ms: int) -> bool:
        """Saves or updates explicit resume bookmark for a URI."""
        if not uri:
            return False
        item = self._db.get_history_item(uri)
        if item:
            return self._db.record_playback(
                uri=uri,
                title=item.get("title", ""),
                artist=item.get("artist", ""),
                album=item.get("album", ""),
                duration_ms=item.get("duration_ms", 0),
                position_ms=position_ms,
                media_type=item.get("media_type", "audio"),
            )
        else:
            return self._db.record_playback(
                uri=uri,
                position_ms=position_ms,
            )

    def get_playback_bookmark(self, uri: str) -> int:
        """Gets bookmark position for a URI."""
        return self.get_resume_position(uri)
