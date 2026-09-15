"""
db.py - SQLite 3 WAL Mode Database Manager for Penguin Desktop Media Player.
Tactile Digital Brutalism // Studio Precision Persistence Layer.

Provides:
- Robust SQLite connection management in Write-Ahead Logging (WAL) journal mode.
- Automatic database path resolution via $XDG_DATA_HOME with safe fallbacks.
- Schema versioning and migration framework.
- Media playback history tracking with indices and resume bookmark positions.
- Playlist matrix and queue item persistence with transactional reordering.
- 10-Band ISO equalizer preset storage and factory preset seeding.
- Key-value application settings storage with JSON serialization.
- Thread-safe query execution and graceful resource cleanup.
"""

from __future__ import annotations
from dataclasses import asdict, dataclass, field
import json
import os
from pathlib import Path
import sqlite3
import threading
from typing import Any, Dict, List, Optional, Tuple, Union


# =============================================================================
# 1. DATA MODELS & CONSTANTS
# =============================================================================

@dataclass
class HistoryItem:
    """Represents a recorded media playback history entry."""
    id: int = 0
    uri: str = ""
    title: str = ""
    artist: str = ""
    album: str = ""
    duration_ms: int = 0
    last_position_ms: int = 0
    play_count: int = 1
    last_played_at: str = ""
    media_type: str = "audio"  # "audio", "video", "stream"
    file_size: int = 0
    cover_art_path: str = ""


@dataclass
class PlaylistItemRecord:
    """Represents an item in a playlist queue matrix."""
    id: int = 0
    playlist_name: str = "default"
    position_order: int = 0
    uri: str = ""
    title: str = ""
    artist: str = ""
    album: str = ""
    duration_ms: int = 0
    format: str = ""
    added_at: str = ""


@dataclass
class EqualizerPresetRecord:
    """Represents a 10-band graphic equalizer preset with preamp gain."""
    id: int = 0
    name: str = ""
    gains: List[float] = field(default_factory=lambda: [0.0] * 10)
    preamp: float = 0.0
    is_builtin: bool = False


# Factory Studio Equalizer Presets
FACTORY_PRESETS = [
    {
        "name": "Flat",
        "gains": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "preamp": 0.0,
        "is_builtin": True,
    },
    {
        "name": "Bass Boost",
        "gains": [6.0, 5.0, 3.5, 1.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "preamp": -2.0,
        "is_builtin": True,
    },
    {
        "name": "Studio Mastering",
        "gains": [1.0, 1.5, 0.0, -0.5, 0.5, 1.0, 1.5, 2.0, 2.5, 2.0],
        "preamp": -1.0,
        "is_builtin": True,
    },
    {
        "name": "Vocal Clarity",
        "gains": [-2.0, -1.0, 0.0, 1.5, 3.0, 3.5, 3.0, 1.5, 0.0, -1.0],
        "preamp": 0.0,
        "is_builtin": True,
    },
    {
        "name": "Electronic / Synth",
        "gains": [5.5, 4.5, 2.0, 0.0, -1.0, 1.5, 2.5, 3.5, 4.5, 4.0],
        "preamp": -2.0,
        "is_builtin": True,
    },
    {
        "name": "Acoustic / Live",
        "gains": [2.0, 1.5, 1.0, 0.0, 1.0, 2.0, 2.5, 3.0, 3.5, 3.0],
        "preamp": -1.0,
        "is_builtin": True,
    },
    {
        "name": "Rock / Metal",
        "gains": [4.5, 3.5, 1.0, -0.5, -1.5, 1.0, 2.0, 3.5, 4.0, 4.5],
        "preamp": -1.5,
        "is_builtin": True,
    },
    {
        "name": "Rock",  # Alias for compatibility
        "gains": [4.5, 3.0, 1.5, 0.0, -1.0, -0.5, 1.5, 3.0, 4.0, 4.5],
        "preamp": -1.5,
        "is_builtin": True,
    },
    {
        "name": "Night Mode (Low Dynamic)",
        "gains": [-4.0, -3.0, -1.5, 0.0, 1.0, 2.0, 2.0, 1.0, -1.0, -3.0],
        "preamp": 1.0,
        "is_builtin": True,
    },
]


def get_default_db_path() -> Path:
    """
    Resolves standard XDG database path ($XDG_DATA_HOME/penguin/penguin.db)
    with fallback to ~/.local/share/penguin/penguin.db or /tmp/penguin_<uid>/penguin.db.
    """
    xdg_data = os.environ.get("XDG_DATA_HOME")
    if xdg_data:
        base_dir = Path(xdg_data) / "penguin"
    else:
        base_dir = Path.home() / ".local" / "share" / "penguin"

    try:
        base_dir.mkdir(parents=True, exist_ok=True)
        db_path = base_dir / "penguin.db"
        # Test writability
        test_file = base_dir / ".write_test"
        test_file.touch(exist_ok=True)
        test_file.unlink(missing_ok=True)
        return db_path
    except (OSError, PermissionError):
        # Sandbox / read-only fallback
        uid = os.getuid() if hasattr(os, "getuid") else 1000
        fallback_dir = Path(f"/tmp/penguin_{uid}")
        fallback_dir.mkdir(parents=True, exist_ok=True)
        return fallback_dir / "penguin.db"


# =============================================================================
# 2. DATABASE MANAGER
# =============================================================================

class DatabaseManager:
    """
    SQLite 3 WAL Mode Database Manager.
    Manages persistence for application settings, media playback history,
    playlists, equalizer presets, and window/session state.
    """

    SCHEMA_VERSION = 1

    SCHEMA_SQL = """
    PRAGMA journal_mode = WAL;
    PRAGMA synchronous = NORMAL;
    PRAGMA foreign_keys = ON;
    PRAGMA busy_timeout = 5000;

    CREATE TABLE IF NOT EXISTS schema_version (
        version INTEGER PRIMARY KEY,
        applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

    CREATE TABLE IF NOT EXISTS app_settings (
        key TEXT PRIMARY KEY,
        value TEXT NOT NULL,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

    CREATE TABLE IF NOT EXISTS media_history (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        uri TEXT UNIQUE NOT NULL,
        title TEXT,
        artist TEXT,
        album TEXT,
        duration_ms INTEGER DEFAULT 0,
        last_position_ms INTEGER DEFAULT 0,
        play_count INTEGER DEFAULT 1,
        last_played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        media_type TEXT CHECK(media_type IN ('audio', 'video', 'stream')) NOT NULL DEFAULT 'audio',
        file_size INTEGER DEFAULT 0,
        cover_art_path TEXT
    );

    CREATE TABLE IF NOT EXISTS saved_playlists (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT UNIQUE NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

    CREATE TABLE IF NOT EXISTS playlist_items (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        playlist_name TEXT NOT NULL DEFAULT 'default',
        position_order INTEGER NOT NULL,
        uri TEXT NOT NULL,
        title TEXT,
        artist TEXT,
        album TEXT,
        duration_ms INTEGER DEFAULT 0,
        format TEXT,
        added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE(playlist_name, position_order)
    );

    CREATE TABLE IF NOT EXISTS equalizer_presets (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT UNIQUE NOT NULL,
        band_32hz REAL DEFAULT 0.0,
        band_64hz REAL DEFAULT 0.0,
        band_125hz REAL DEFAULT 0.0,
        band_250hz REAL DEFAULT 0.0,
        band_500hz REAL DEFAULT 0.0,
        band_1khz REAL DEFAULT 0.0,
        band_2khz REAL DEFAULT 0.0,
        band_4khz REAL DEFAULT 0.0,
        band_8khz REAL DEFAULT 0.0,
        band_16khz REAL DEFAULT 0.0,
        preamp REAL DEFAULT 0.0,
        is_builtin BOOLEAN DEFAULT 0
    );

    CREATE INDEX IF NOT EXISTS idx_history_last_played ON media_history(last_played_at DESC);
    CREATE INDEX IF NOT EXISTS idx_playlist_order ON playlist_items(playlist_name, position_order ASC);
    """

    def __init__(self, db_path: Optional[Union[str, Path]] = None, timeout: float = 5.0):
        """
        Initializes the DatabaseManager.
        If db_path is None, uses default XDG path.
        If db_path is ':memory:', creates an in-memory SQLite database.
        """
        if db_path is None:
            self.db_path = get_default_db_path()
        elif str(db_path) == ":memory:":
            self.db_path = Path(":memory:")
        else:
            self.db_path = Path(db_path)
            if str(self.db_path) != ":memory:":
                self.db_path.parent.mkdir(parents=True, exist_ok=True)

        self._timeout = timeout
        self._lock = threading.RLock()
        self._conn: Optional[sqlite3.Connection] = None
        self._is_open = False
        self._initialize()

    def _initialize(self):
        """Opens database connection and sets up WAL schema and factory presets."""
        with self._lock:
            if str(self.db_path) == ":memory:":
                self._conn = sqlite3.connect(":memory:", timeout=self._timeout, check_same_thread=False)
            else:
                self._conn = sqlite3.connect(str(self.db_path), timeout=self._timeout, check_same_thread=False)

            self._conn.row_factory = sqlite3.Row
            self._is_open = True

            with self._conn:
                self._conn.executescript(self.SCHEMA_SQL)
                self._conn.execute(
                    "INSERT OR IGNORE INTO schema_version (version) VALUES (?)",
                    (self.SCHEMA_VERSION,)
                )

            self._seed_factory_presets()

    def _seed_factory_presets(self):
        """Seeds built-in equalizer factory presets if not already present."""
        with self._lock, self._conn:
            for preset in FACTORY_PRESETS:
                gains = preset["gains"]
                self._conn.execute(
                    """
                    INSERT OR IGNORE INTO equalizer_presets
                    (name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz,
                     band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                    """,
                    (
                        preset["name"],
                        gains[0], gains[1], gains[2], gains[3], gains[4],
                        gains[5], gains[6], gains[7], gains[8], gains[9],
                        preset.get("preamp", 0.0),
                        1 if preset.get("is_builtin", True) else 0,
                    )
                )

    @property
    def conn(self) -> sqlite3.Connection:
        """Returns the active SQLite connection."""
        if not self._is_open or self._conn is None:
            raise sqlite3.OperationalError("Database is closed.")
        return self._conn

    def is_open(self) -> bool:
        """Returns True if the database connection is open and active."""
        return self._is_open and self._conn is not None

    def close(self):
        """Safely closes the database connection."""
        with self._lock:
            if self._is_open and self._conn is not None:
                try:
                    self._conn.close()
                except Exception:
                    pass
                self._conn = None
                self._is_open = False

    def __enter__(self) -> DatabaseManager:
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    # =========================================================================
    # APP SETTINGS KEY-VALUE CRUD
    # =========================================================================

    def set_setting(self, key: str, value: Any) -> bool:
        """Saves an application setting serialized to JSON."""
        with self._lock, self.conn:
            json_val = json.dumps(value)
            self.conn.execute(
                """
                INSERT INTO app_settings (key, value, updated_at)
                VALUES (?, ?, CURRENT_TIMESTAMP)
                ON CONFLICT(key) DO UPDATE SET
                    value = excluded.value,
                    updated_at = CURRENT_TIMESTAMP
                """,
                (key, json_val),
            )
        return True

    def get_setting(self, key: str, default: Any = None) -> Any:
        """Retrieves a setting by key, deserializing from JSON."""
        with self._lock:
            cursor = self.conn.execute("SELECT value FROM app_settings WHERE key = ?", (key,))
            row = cursor.fetchone()
            if row is None:
                return default
            try:
                return json.loads(row["value"])
            except (json.JSONDecodeError, TypeError):
                return row["value"]

    def remove_setting(self, key: str) -> bool:
        """Removes a setting by key."""
        with self._lock, self.conn:
            cursor = self.conn.execute("DELETE FROM app_settings WHERE key = ?", (key,))
            return cursor.rowcount > 0

    # =========================================================================
    # MEDIA PLAYBACK HISTORY CRUD
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
        """
        Records or updates a media playback entry in history.
        Increments play_count and updates last_position_ms and timestamp.
        """
        if not uri:
            return False

        with self._lock, self.conn:
            self.conn.execute(
                """
                INSERT INTO media_history
                (uri, title, artist, album, duration_ms, last_position_ms,
                 media_type, file_size, cover_art_path, play_count, last_played_at)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 1, CURRENT_TIMESTAMP)
                ON CONFLICT(uri) DO UPDATE SET
                    title = excluded.title,
                    artist = excluded.artist,
                    album = excluded.album,
                    duration_ms = excluded.duration_ms,
                    last_position_ms = excluded.last_position_ms,
                    media_type = excluded.media_type,
                    file_size = excluded.file_size,
                    cover_art_path = excluded.cover_art_path,
                    play_count = media_history.play_count + 1,
                    last_played_at = CURRENT_TIMESTAMP
                """,
                (
                    uri,
                    title or "",
                    artist or "",
                    album or "",
                    int(duration_ms),
                    int(position_ms),
                    media_type if media_type in ("audio", "video", "stream") else "audio",
                    int(file_size),
                    cover_art_path or "",
                ),
            )
        return True

    def get_recent_history(self, limit: int = 50) -> List[Dict[str, Any]]:
        """Retrieves recent playback history ordered by last played timestamp descending."""
        with self._lock:
            cursor = self.conn.execute(
                """
                SELECT id, uri, title, artist, album, duration_ms, last_position_ms,
                       play_count, last_played_at, media_type, file_size, cover_art_path
                FROM media_history
                ORDER BY last_played_at DESC
                LIMIT ?
                """,
                (limit,),
            )
            return [dict(row) for row in cursor.fetchall()]

    def get_history_item(self, uri: str) -> Optional[Dict[str, Any]]:
        """Retrieves history record for a specific URI or None if not found."""
        if not uri:
            return None
        with self._lock:
            cursor = self.conn.execute(
                """
                SELECT id, uri, title, artist, album, duration_ms, last_position_ms,
                       play_count, last_played_at, media_type, file_size, cover_art_path
                FROM media_history
                WHERE uri = ?
                """,
                (uri,),
            )
            row = cursor.fetchone()
            return dict(row) if row else None

    def remove_history_item(self, uri: str) -> bool:
        """Removes a media history entry by URI."""
        if not uri:
            return False
        with self._lock, self.conn:
            cursor = self.conn.execute("DELETE FROM media_history WHERE uri = ?", (uri,))
            return cursor.rowcount > 0

    def clear_history(self) -> bool:
        """Clears all playback history records."""
        with self._lock, self.conn:
            self.conn.execute("DELETE FROM media_history")
        return True

    # =========================================================================
    # PLAYLIST MATRIX CRUD
    # =========================================================================

    def save_playlist_items(self, playlist_name: str, items: List[Dict[str, Any]]) -> bool:
        """
        Atomically saves and replaces playlist items for a named playlist.
        """
        if not playlist_name:
            playlist_name = "default"

        with self._lock, self.conn:
            self.conn.execute("DELETE FROM playlist_items WHERE playlist_name = ?", (playlist_name,))
            for idx, item in enumerate(items):
                self.conn.execute(
                    """
                    INSERT INTO playlist_items
                    (playlist_name, position_order, uri, title, artist, album, duration_ms, format)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                    """,
                    (
                        playlist_name,
                        idx,
                        item.get("uri") or item.get("filePath") or item.get("path") or "",
                        item.get("title", ""),
                        item.get("artist", ""),
                        item.get("album", ""),
                        int(item.get("duration_ms", 0)),
                        item.get("format", ""),
                    ),
                )

            # Upsert saved_playlists master entry
            self.conn.execute(
                """
                INSERT INTO saved_playlists (name, updated_at)
                VALUES (?, CURRENT_TIMESTAMP)
                ON CONFLICT(name) DO UPDATE SET updated_at = CURRENT_TIMESTAMP
                """,
                (playlist_name,),
            )
        return True

    def get_playlist_items(self, playlist_name: str = "default") -> List[Dict[str, Any]]:
        """Retrieves playlist items ordered by position."""
        if not playlist_name:
            playlist_name = "default"

        with self._lock:
            cursor = self.conn.execute(
                """
                SELECT id, playlist_name, position_order, uri, title, artist, album,
                       duration_ms, format, added_at
                FROM playlist_items
                WHERE playlist_name = ?
                ORDER BY position_order ASC
                """,
                (playlist_name,),
            )
            return [dict(row) for row in cursor.fetchall()]

    def clear_playlist(self, playlist_name: str = "default") -> bool:
        """Clears items in a playlist."""
        if not playlist_name:
            playlist_name = "default"
        with self._lock, self.conn:
            self.conn.execute("DELETE FROM playlist_items WHERE playlist_name = ?", (playlist_name,))
        return True

    def get_saved_playlist_names(self) -> List[str]:
        """Retrieves list of all saved playlist names."""
        with self._lock:
            cursor = self.conn.execute("SELECT name FROM saved_playlists ORDER BY name ASC")
            return [row["name"] for row in cursor.fetchall()]

    def delete_saved_playlist(self, playlist_name: str) -> bool:
        """Deletes a saved playlist and its items."""
        if not playlist_name:
            return False
        with self._lock, self.conn:
            self.conn.execute("DELETE FROM playlist_items WHERE playlist_name = ?", (playlist_name,))
            cursor = self.conn.execute("DELETE FROM saved_playlists WHERE name = ?", (playlist_name,))
            return cursor.rowcount > 0

    # =========================================================================
    # EQUALIZER PRESETS CRUD
    # =========================================================================

    def save_equalizer_preset(
        self,
        name: str,
        gains: List[float],
        preamp: float = 0.0,
        is_builtin: bool = False,
    ) -> bool:
        """Saves or updates a 10-band equalizer preset."""
        if not name:
            raise ValueError("Equalizer preset name cannot be empty")
        if len(gains) != 10:
            raise ValueError("Equalizer preset requires exactly 10 band gains")

        with self._lock, self.conn:
            self.conn.execute(
                """
                INSERT INTO equalizer_presets
                (name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz,
                 band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(name) DO UPDATE SET
                    band_32hz = excluded.band_32hz,
                    band_64hz = excluded.band_64hz,
                    band_125hz = excluded.band_125hz,
                    band_250hz = excluded.band_250hz,
                    band_500hz = excluded.band_500hz,
                    band_1khz = excluded.band_1khz,
                    band_2khz = excluded.band_2khz,
                    band_4khz = excluded.band_4khz,
                    band_8khz = excluded.band_8khz,
                    band_16khz = excluded.band_16khz,
                    preamp = excluded.preamp,
                    is_builtin = excluded.is_builtin
                """,
                (
                    name,
                    float(gains[0]), float(gains[1]), float(gains[2]), float(gains[3]), float(gains[4]),
                    float(gains[5]), float(gains[6]), float(gains[7]), float(gains[8]), float(gains[9]),
                    float(preamp),
                    1 if is_builtin else 0,
                ),
            )
        return True

    def get_equalizer_preset(self, name: str) -> Optional[Dict[str, Any]]:
        """Retrieves an equalizer preset by name."""
        if not name:
            return None
        with self._lock:
            cursor = self.conn.execute(
                """
                SELECT id, name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz,
                       band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin
                FROM equalizer_presets
                WHERE name = ?
                """,
                (name,),
            )
            row = cursor.fetchone()
            if not row:
                return None

            d = dict(row)
            gains = [
                d["band_32hz"], d["band_64hz"], d["band_125hz"], d["band_250hz"], d["band_500hz"],
                d["band_1khz"], d["band_2khz"], d["band_4khz"], d["band_8khz"], d["band_16khz"],
            ]
            return {
                "id": d["id"],
                "name": d["name"],
                "gains": gains,
                "preamp": d["preamp"],
                "is_builtin": bool(d["is_builtin"]),
            }

    def get_all_equalizer_presets(self) -> List[Dict[str, Any]]:
        """Retrieves all equalizer presets (built-in first, then alphabetical)."""
        with self._lock:
            cursor = self.conn.execute(
                """
                SELECT id, name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz,
                       band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin
                FROM equalizer_presets
                ORDER BY is_builtin DESC, name ASC
                """
            )
            results = []
            for row in cursor.fetchall():
                d = dict(row)
                gains = [
                    d["band_32hz"], d["band_64hz"], d["band_125hz"], d["band_250hz"], d["band_500hz"],
                    d["band_1khz"], d["band_2khz"], d["band_4khz"], d["band_8khz"], d["band_16khz"],
                ]
                results.append({
                    "id": d["id"],
                    "name": d["name"],
                    "gains": gains,
                    "preamp": d["preamp"],
                    "is_builtin": bool(d["is_builtin"]),
                })
            return results

    def delete_equalizer_preset(self, name: str) -> bool:
        """Deletes a custom equalizer preset (cannot delete built-in presets)."""
        if not name:
            return False
        with self._lock, self.conn:
            cursor = self.conn.execute(
                "DELETE FROM equalizer_presets WHERE name = ? AND is_builtin = 0",
                (name,),
            )
            return cursor.rowcount > 0
