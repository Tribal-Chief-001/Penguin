"""
test_persistence.py - SQLite WAL State & Playback History Persistence Test Suite.

Tests database initialization in WAL journal mode, media playback history tracking,
recent items queries, playlist matrix reordering, equalizer preset seeding and retrieval,
and atomic key-value application settings persistence.
"""

import json
from pathlib import Path
import sqlite3
import tempfile
import unittest
from typing import Any, Dict, List, Optional, Tuple


class DatabaseManager:
    """Manages SQLite database storage for history, playlists, presets, and settings."""

    SCHEMA_SQL = """
    PRAGMA journal_mode = WAL;
    PRAGMA synchronous = NORMAL;
    PRAGMA foreign_keys = ON;

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

    CREATE TABLE IF NOT EXISTS playlist_items (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        playlist_name TEXT NOT NULL DEFAULT 'default',
        position_order INTEGER NOT NULL,
        uri TEXT NOT NULL,
        title TEXT,
        artist TEXT,
        duration_ms INTEGER DEFAULT 0,
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

    def __init__(self, db_path: Path):
        self.db_path = db_path
        self.conn = sqlite3.connect(str(db_path))
        self.conn.row_factory = sqlite3.Row
        self.init_schema()

    def init_schema(self):
        with self.conn:
            self.conn.executescript(self.SCHEMA_SQL)
            # Insert initial schema version if not present
            self.conn.execute("INSERT OR IGNORE INTO schema_version (version) VALUES (1)")

    def close(self):
        self.conn.close()

    # Settings
    def set_setting(self, key: str, value: Any):
        json_val = json.dumps(value)
        with self.conn:
            self.conn.execute(
                "INSERT INTO app_settings (key, value, updated_at) VALUES (?, ?, CURRENT_TIMESTAMP) "
                "ON CONFLICT(key) DO UPDATE SET value=excluded.value, updated_at=CURRENT_TIMESTAMP",
                (key, json_val),
            )

    def get_setting(self, key: str, default: Any = None) -> Any:
        cursor = self.conn.execute("SELECT value FROM app_settings WHERE key = ?", (key,))
        row = cursor.fetchone()
        if row is None:
            return default
        try:
            return json.loads(row["value"])
        except json.JSONDecodeError:
            return row["value"]

    # History
    def record_playback(
        self,
        uri: str,
        title: str,
        artist: str,
        album: str,
        duration_ms: int,
        position_ms: int,
        media_type: str = "audio",
    ):
        with self.conn:
            self.conn.execute(
                """
                INSERT INTO media_history (uri, title, artist, album, duration_ms, last_position_ms, media_type, play_count, last_played_at)
                VALUES (?, ?, ?, ?, ?, ?, ?, 1, CURRENT_TIMESTAMP)
                ON CONFLICT(uri) DO UPDATE SET
                    title = excluded.title,
                    artist = excluded.artist,
                    album = excluded.album,
                    duration_ms = excluded.duration_ms,
                    last_position_ms = excluded.last_position_ms,
                    media_type = excluded.media_type,
                    play_count = media_history.play_count + 1,
                    last_played_at = CURRENT_TIMESTAMP
                """,
                (uri, title, artist, album, duration_ms, position_ms, media_type),
            )

    def get_recent_history(self, limit: int = 50) -> List[Dict[str, Any]]:
        cursor = self.conn.execute(
            "SELECT * FROM media_history ORDER BY last_played_at DESC LIMIT ?", (limit,)
        )
        return [dict(row) for row in cursor.fetchall()]

    # Playlists
    def save_playlist_items(self, playlist_name: str, items: List[Dict[str, Any]]):
        with self.conn:
            self.conn.execute("DELETE FROM playlist_items WHERE playlist_name = ?", (playlist_name,))
            for idx, it in enumerate(items):
                self.conn.execute(
                    """
                    INSERT INTO playlist_items (playlist_name, position_order, uri, title, artist, duration_ms)
                    VALUES (?, ?, ?, ?, ?, ?)
                    """,
                    (
                        playlist_name,
                        idx,
                        it.get("uri", ""),
                        it.get("title", ""),
                        it.get("artist", ""),
                        it.get("duration_ms", 0),
                    ),
                )

    def get_playlist_items(self, playlist_name: str = "default") -> List[Dict[str, Any]]:
        cursor = self.conn.execute(
            "SELECT * FROM playlist_items WHERE playlist_name = ? ORDER BY position_order ASC",
            (playlist_name,),
        )
        return [dict(row) for row in cursor.fetchall()]

    # Presets
    def save_equalizer_preset(self, name: str, gains: List[float], preamp: float = 0.0, is_builtin: bool = False):
        if len(gains) != 10:
            raise ValueError("Equalizer preset requires exactly 10 band gains")
        with self.conn:
            self.conn.execute(
                """
                INSERT OR REPLACE INTO equalizer_presets
                (name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz, band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (name, *gains, preamp, int(is_builtin)),
            )

    def get_equalizer_preset(self, name: str) -> Optional[Dict[str, Any]]:
        cursor = self.conn.execute("SELECT * FROM equalizer_presets WHERE name = ?", (name,))
        row = cursor.fetchone()
        if not row:
            return None
        d = dict(row)
        gains = [
            d["band_32hz"], d["band_64hz"], d["band_125hz"], d["band_250hz"], d["band_500hz"],
            d["band_1khz"], d["band_2khz"], d["band_4khz"], d["band_8khz"], d["band_16khz"],
        ]
        return {"name": d["name"], "gains": gains, "preamp": d["preamp"], "is_builtin": bool(d["is_builtin"])}


class TestPersistence(unittest.TestCase):
    """Test suite verifying SQLite WAL database persistence."""

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory(prefix="penguin_db_test_")
        self.db_path = Path(self.temp_dir.name) / "test_history.db"
        self.db = DatabaseManager(self.db_path)

    def tearDown(self):
        self.db.close()
        self.temp_dir.cleanup()

    def test_wal_journal_mode_active(self):
        cursor = self.db.conn.execute("PRAGMA journal_mode")
        mode = cursor.fetchone()[0]
        self.assertEqual(mode.lower(), "wal")

    def test_app_settings_key_value_storage(self):
        self.db.set_setting("volume", 0.85)
        self.db.set_setting("ui_mode", "video")
        self.db.set_setting("window_geometry", {"x": 100, "y": 100, "width": 1280, "height": 720})

        self.assertAlmostEqual(self.db.get_setting("volume"), 0.85)
        self.assertEqual(self.db.get_setting("ui_mode"), "video")
        geom = self.db.get_setting("window_geometry")
        self.assertEqual(geom["width"], 1280)
        self.assertEqual(self.db.get_setting("non_existent_key", "default_val"), "default_val")

    def test_media_history_upsert_and_resume_position(self):
        # 1. Play track first time
        self.db.record_playback(
            uri="file:///music/song1.flac",
            title="Neon Lights",
            artist="Kraftwerk",
            album="The Man-Machine",
            duration_ms=240000,
            position_ms=45000,
            media_type="audio",
        )

        history = self.db.get_recent_history(10)
        self.assertEqual(len(history), 1)
        self.assertEqual(history[0]["title"], "Neon Lights")
        self.assertEqual(history[0]["last_position_ms"], 45000)
        self.assertEqual(history[0]["play_count"], 1)

        # 2. Play same track again with new position
        self.db.record_playback(
            uri="file:///music/song1.flac",
            title="Neon Lights",
            artist="Kraftwerk",
            album="The Man-Machine",
            duration_ms=240000,
            position_ms=120000,
            media_type="audio",
        )

        history2 = self.db.get_recent_history(10)
        self.assertEqual(len(history2), 1)
        self.assertEqual(history2[0]["last_position_ms"], 120000)
        self.assertEqual(history2[0]["play_count"], 2)

    def test_playlist_matrix_save_and_reorder(self):
        items = [
            {"uri": "file:///track1.mp3", "title": "Track 1", "artist": "Artist 1", "duration_ms": 180000},
            {"uri": "file:///track2.mp3", "title": "Track 2", "artist": "Artist 2", "duration_ms": 200000},
            {"uri": "file:///track3.mp3", "title": "Track 3", "artist": "Artist 3", "duration_ms": 220000},
        ]
        self.db.save_playlist_items("default", items)

        loaded = self.db.get_playlist_items("default")
        self.assertEqual(len(loaded), 3)
        self.assertEqual(loaded[0]["title"], "Track 1")
        self.assertEqual(loaded[2]["title"], "Track 3")

        # Reorder items (swap item 0 and 2)
        reordered = [items[2], items[1], items[0]]
        self.db.save_playlist_items("default", reordered)

        loaded_reordered = self.db.get_playlist_items("default")
        self.assertEqual(loaded_reordered[0]["title"], "Track 3")
        self.assertEqual(loaded_reordered[2]["title"], "Track 1")

    def test_equalizer_presets_save_and_load(self):
        rock_gains = [4.5, 3.0, 1.5, 0.0, -1.0, -0.5, 1.5, 3.0, 4.0, 4.5]
        self.db.save_equalizer_preset("Rock", rock_gains, preamp=-1.5, is_builtin=True)

        preset = self.db.get_equalizer_preset("Rock")
        self.assertIsNotNone(preset)
        self.assertEqual(preset["name"], "Rock")
        self.assertEqual(preset["gains"], rock_gains)
        self.assertEqual(preset["preamp"], -1.5)
        self.assertTrue(preset["is_builtin"])


if __name__ == "__main__":
    unittest.main()
