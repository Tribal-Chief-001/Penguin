"""
Penguin Media Player — Tactile Digital Brutalist GUI Subsystem.

Modules:
- theme: Color tokens, monospace/Swiss font stacks, and Brutalist QSS styling.
- video_viewport: Borderless video surface, safe reticles (90%/80%), diagnostics HUD, bottom dock.
- scrubber: Mechanical SMPTE timecode tick ruler scrubber with chapter diamonds.
- vu_widget: 30-segment stereo peak/RMS VU meter rack with ballistic peak hold.
- eq_widget: 10-band graphic equalizer rack (32Hz-16kHz ISO) with presets.
- lyrics_widget: Synchronized .lrc lyrics teleprompter with auto-scroll and click-to-seek.
- playlist_widget: 6-column playlist queue matrix with live search and reordering.
- audio_deck: Hi-Fi Audio Deck container assembling masthead, meters, EQ, lyrics, matrix.
- main_window: Top-level MainWindow orchestrating Viewfinder and Audio Deck modes.
"""

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
    ACCENT_CLIP_RED,
    MONOSPACE_FONT_FAMILY,
    SANS_FONT_FAMILY,
    BrutalistTheme,
)
from src.ui.scrubber import (
    ChapterMarker,
    MechanicalTickScrubber,
)
from src.ui.vu_widget import (
    VU_MIN_DB,
    VU_MAX_DB,
    VU_HEADROOM_DB,
    VU_CLIP_DB,
    VU_NUM_SEGMENTS,
    VU_CALIBRATION_MARKS,
    StereoVUMeterRack,
)
from src.ui.eq_widget import (
    EQ_NUM_BANDS,
    EQ_PRESETS,
    EqualizerRackWidget,
)
from src.ui.lyrics_widget import (
    SynchronizedLyricsTeleprompter,
)
from src.ui.playlist_widget import (
    PlaylistItem,
    PLAYLIST_COLUMNS,
    PlaylistMatrixWidget,
)
from src.ui.video_viewport import (
    DiagnosticsData,
    DiagnosticsHUDWidget,
    SafeAreaReticles,
    ViewfinderWidget,
)
from src.ui.audio_deck import (
    RepeatMode,
    AudioMasthead,
    AudioDeckWidget,
)
from src.ui.main_window import (
    UIMode,
    MainWindow,
)

__all__ = [
    # Theme & Tokens
    "BG_DEEP_OBSIDIAN",
    "SURFACE_PANEL_BASE",
    "SURFACE_RAISED",
    "GRID_STRUCTURAL_BORDER",
    "GRID_LINE_ACTIVE",
    "TEXT_HIGH_CONTRAST",
    "TEXT_SECONDARY_DIM",
    "TEXT_MUTED",
    "ACCENT_SAFETY_ORANGE",
    "ACCENT_SIGNAL_LIME",
    "ACCENT_TELEMETRY_CYAN",
    "ACCENT_CLIP_RED",
    "MONOSPACE_FONT_FAMILY",
    "SANS_FONT_FAMILY",
    "BrutalistTheme",
    # Scrubber
    "ChapterMarker",
    "MechanicalTickScrubber",
    # VU Meter
    "VU_MIN_DB",
    "VU_MAX_DB",
    "VU_HEADROOM_DB",
    "VU_CLIP_DB",
    "VU_NUM_SEGMENTS",
    "VU_CALIBRATION_MARKS",
    "StereoVUMeterRack",
    # EQ Widget
    "EQ_NUM_BANDS",
    "EQ_PRESETS",
    "EqualizerRackWidget",
    # Lyrics Widget
    "SynchronizedLyricsTeleprompter",
    # Playlist Widget
    "PlaylistItem",
    "PLAYLIST_COLUMNS",
    "PlaylistMatrixWidget",
    # Video Viewport
    "DiagnosticsData",
    "DiagnosticsHUDWidget",
    "SafeAreaReticles",
    "ViewfinderWidget",
    # Audio Deck
    "RepeatMode",
    "AudioMasthead",
    "AudioDeckWidget",
    # Main Window
    "UIMode",
    "MainWindow",
]
