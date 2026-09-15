"""
audio_deck.py - Hi-Fi Audio Deck Container Widget for Penguin.

Features:
- Typographic Metadata Masthead: 110x110 artwork frame, Swiss typography hierarchy, cyan technical badge.
- Stereo Peak VU Meter Rack (CH_L & CH_R 30-segment LED ballistics).
- 10-Band Graphic Equalizer Rack (32Hz-16kHz ISO sliders with presets).
- Synchronized .lrc Lyrics Teleprompter.
- Playlist Queue Matrix.
- Mechanical SMPTE tick scrubber.
- Tactile bottom audio dock with transport, repeat mode cycle, shuffle, volume/mute.
"""

from dataclasses import dataclass
from enum import Enum
from typing import Any, Callable, Dict, List, Optional, Tuple

from src.ui.eq_widget import EqualizerRackWidget
from src.ui.lyrics_widget import SynchronizedLyricsTeleprompter
from src.ui.playlist_widget import PlaylistItem, PlaylistMatrixWidget
from src.ui.scrubber import MechanicalTickScrubber
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
)
from src.ui.vu_widget import StereoVUMeterRack


class RepeatMode(Enum):
    Off = 0
    RepeatAll = 1
    RepeatOne = 2


@dataclass
class AudioMasthead:
    """Typographic metadata masthead model and layout."""
    title: str = "No Media Loaded"
    artist: str = "--"
    album: str = "--"
    sample_rate_hz: int = 48000
    bit_depth: int = 24
    bitrate_kbps: int = 0
    channels: str = "STEREO"
    artwork_path: Optional[str] = None
    artwork_width: int = 110
    artwork_height: int = 110

    def technical_badge(self) -> str:
        """Formats high-res technical audio badge string."""
        sample_rate_khz = self.sample_rate_hz / 1000.0
        bitrate_str = f"{self.bitrate_kbps:,} kbps" if self.bitrate_kbps > 0 else "-- kbps"
        return f"[AUDIO DECK // {self.bit_depth}-BIT / {sample_rate_khz:.1f} kHz / {bitrate_str}] [{self.channels.upper()}]"


class AudioDeckWidget:
    """
    Hi-Fi Audio Deck Container.
    Assembles Masthead, VU meters, EQ rack, Lyrics teleprompter, Playlist matrix, and Dock.
    """

    def __init__(self, playback_engine=None, width: int = 900, height: int = 600):
        self._width = max(600, int(width))
        self._height = max(400, int(height))
        self._engine = playback_engine

        # Subcomponents
        self._masthead = AudioMasthead()
        self._vu_meters = StereoVUMeterRack(width=360, height=72)
        self._eq_rack = EqualizerRackWidget(width=600, height=160)
        self._teleprompter = SynchronizedLyricsTeleprompter(width=400, height=300)
        self._playlist_matrix = PlaylistMatrixWidget(width=500, height=300)
        self._scrubber = MechanicalTickScrubber(width=self._width, height=38)

        # Transport & State
        self._repeat_mode = RepeatMode.Off
        self._shuffle_enabled = False

    @property
    def masthead(self) -> AudioMasthead:
        return self._masthead

    @property
    def vu_meters(self) -> StereoVUMeterRack:
        return self._vu_meters

    @property
    def eq_rack(self) -> EqualizerRackWidget:
        return self._eq_rack

    @property
    def teleprompter(self) -> SynchronizedLyricsTeleprompter:
        return self._teleprompter

    @property
    def playlist_matrix(self) -> PlaylistMatrixWidget:
        return self._playlist_matrix

    @property
    def scrubber(self) -> MechanicalTickScrubber:
        return self._scrubber

    @property
    def repeat_mode(self) -> RepeatMode:
        return self._repeat_mode

    def is_shuffle_enabled(self) -> bool:
        return self._shuffle_enabled

    def cycle_repeat_mode(self) -> RepeatMode:
        """Cycles RepeatMode: Off -> RepeatAll -> RepeatOne -> Off."""
        if self._repeat_mode == RepeatMode.Off:
            self._repeat_mode = RepeatMode.RepeatAll
        elif self._repeat_mode == RepeatMode.RepeatAll:
            self._repeat_mode = RepeatMode.RepeatOne
        else:
            self._repeat_mode = RepeatMode.Off
        return self._repeat_mode

    def set_repeat_mode(self, mode: RepeatMode):
        self._repeat_mode = mode

    def toggle_shuffle(self) -> bool:
        self._shuffle_enabled = not self._shuffle_enabled
        if self._shuffle_enabled:
            self._playlist_matrix.shuffle()
        return self._shuffle_enabled

    def set_shuffle(self, enabled: bool):
        self._shuffle_enabled = bool(enabled)
        if self._shuffle_enabled:
            self._playlist_matrix.shuffle()

    def update_metadata(
        self,
        title: str = "",
        artist: str = "",
        album: str = "",
        sample_rate_hz: int = 48000,
        bit_depth: int = 24,
        bitrate_kbps: int = 0,
        channels: str = "STEREO",
        artwork_path: Optional[str] = None,
    ):
        self._masthead = AudioMasthead(
            title=title or "Unknown Track",
            artist=artist or "Unknown Artist",
            album=album or "Unknown Album",
            sample_rate_hz=sample_rate_hz or 48000,
            bit_depth=bit_depth or 24,
            bitrate_kbps=bitrate_kbps or 0,
            channels=channels or "STEREO",
            artwork_path=artwork_path,
        )

    def resize(self, width: int, height: int):
        self._width = max(600, int(width))
        self._height = max(400, int(height))
        self._scrubber.resize(self._width, 38)
        self._teleprompter.resize(self._width // 2, self._height // 2)
        self._playlist_matrix.resize(self._width // 2, self._height // 2)
