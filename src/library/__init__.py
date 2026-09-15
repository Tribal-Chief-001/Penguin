"""
Penguin Media Player — Media Library & SQLite WAL State Persistence.
Tactile Digital Brutalism // Studio Precision State & Storage Architecture.

Exports:
- DatabaseManager: SQLite 3 Write-Ahead Logging (WAL) database manager.
- StatePersistence: Cross-session window, audio, EQ, playlist, and bookmark persistence.
- HistoryItem: Media playback history entry dataclass.
- PlaylistItemRecord: Playlist queue item record dataclass.
- EqualizerPresetRecord: 10-band graphic equalizer preset dataclass.
- FACTORY_PRESETS: Built-in factory studio equalizer presets.
- get_default_db_path: Resolves $XDG_DATA_HOME/penguin/penguin.db with fallbacks.
"""

from src.library.db import (
    DatabaseManager,
    HistoryItem,
    PlaylistItemRecord,
    EqualizerPresetRecord,
    FACTORY_PRESETS,
    get_default_db_path,
)
from src.library.state import StatePersistence

__all__ = [
    "DatabaseManager",
    "StatePersistence",
    "HistoryItem",
    "PlaylistItemRecord",
    "EqualizerPresetRecord",
    "FACTORY_PRESETS",
    "get_default_db_path",
]
