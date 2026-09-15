"""
eq_widget.py - 10-Band Graphic Equalizer Rack Widget for Penguin.

Features:
- 10 ISO standard center frequencies: 32Hz, 64Hz, 125Hz, 250Hz, 500Hz, 1kHz, 2kHz, 4kHz, 8kHz, 16kHz.
- Precision ±12.0 dB gain control with 0.1 dB resolution.
- Dynamic color-coded numeric gain readouts (Signal Lime for boost, Safety Orange for cut).
- Built-in studio presets (Flat, Rock, Pop, Jazz, Classical, Bass Boost, Vocal, Electronic).
- Tactile Reset Flat action.
"""

from typing import Callable, Dict, List, Optional, Tuple

from src.engine.audio_dsp import (
    BiquadPeakingEQ,
    EqualizerDSP,
    ISO_CENTER_FREQUENCIES,
    MIN_GAIN_DB,
    MAX_GAIN_DB,
)
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

EQ_NUM_BANDS = len(ISO_CENTER_FREQUENCIES)
EQ_PRESETS: Dict[str, List[float]] = {
    "Flat": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    "Rock": [4.5, 3.0, 1.5, 0.0, -1.0, -0.5, 1.5, 3.0, 4.0, 4.5],
    "Pop": [-1.0, 1.0, 3.0, 4.0, 3.5, 1.5, -0.5, -1.0, -1.0, -0.5],
    "Jazz": [3.0, 2.0, 0.5, 1.5, -1.5, -1.5, 0.0, 1.5, 2.5, 3.0],
    "Classical": [4.0, 3.0, 2.5, 2.0, -1.0, -1.0, 0.0, 2.0, 3.0, 3.5],
    "Bass Boost": [6.0, 5.0, 4.0, 2.5, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    "Vocal": [-2.0, -1.5, -0.5, 1.5, 4.0, 4.5, 3.5, 1.5, 0.0, -1.0],
    "Electronic": [5.0, 4.5, 2.0, 0.0, -2.0, 1.5, 0.5, 2.0, 4.0, 4.5],
}


class EqualizerRackWidget:
    """
    10-Band Graphic Equalizer Rack Model and Controller.
    Manages 10 ISO band sliders, preset configurations, and parameter change notifications.
    """

    def __init__(self, width: int = 600, height: int = 160):
        self._width = max(300, int(width))
        self._height = max(100, int(height))
        self._bands: List[float] = [0.0] * EQ_NUM_BANDS
        self._current_preset = "Flat"

        # Signal Callbacks
        self._on_band_changed: Optional[Callable[[int, float], None]] = None
        self._on_preset_changed: Optional[Callable[[str], None]] = None
        self._on_all_gains_changed: Optional[Callable[[List[float]], None]] = None

    @property
    def bands(self) -> List[float]:
        return list(self._bands)

    @property
    def current_preset(self) -> str:
        return self._current_preset

    @property
    def num_bands(self) -> int:
        return EQ_NUM_BANDS

    @classmethod
    def iso_frequencies(cls) -> List[int]:
        return list(ISO_CENTER_FREQUENCIES)

    @classmethod
    def preset_names(cls) -> List[str]:
        return list(EQ_PRESETS.keys())

    # Callbacks
    def set_on_band_changed(self, cb: Optional[Callable[[int, float], None]]):
        self._on_band_changed = cb

    def set_on_preset_changed(self, cb: Optional[Callable[[str], None]]):
        self._on_preset_changed = cb

    def set_on_all_gains_changed(self, cb: Optional[Callable[[List[float]], None]]):
        self._on_all_gains_changed = cb

    # Band Control
    def band_gain(self, band_index: int) -> float:
        if 0 <= band_index < EQ_NUM_BANDS:
            return self._bands[band_index]
        return 0.0

    def set_band_gain(self, band_index: int, gain_db: float):
        if 0 <= band_index < EQ_NUM_BANDS:
            clamped = max(MIN_GAIN_DB, min(round(float(gain_db), 1), MAX_GAIN_DB))
            if self._bands[band_index] != clamped:
                self._bands[band_index] = clamped
                self._current_preset = "Custom"
                if self._on_band_changed:
                    self._on_band_changed(band_index, clamped)
                if self._on_all_gains_changed:
                    self._on_all_gains_changed(self.bands)

    def set_all_gains(self, gains: List[float]):
        if len(gains) == EQ_NUM_BANDS:
            self._bands = [max(MIN_GAIN_DB, min(round(float(g), 1), MAX_GAIN_DB)) for g in gains]
            self._current_preset = "Custom"
            if self._on_all_gains_changed:
                self._on_all_gains_changed(self.bands)

    # Preset Operations
    def set_preset(self, preset_name: str) -> bool:
        if preset_name in EQ_PRESETS:
            self._bands = list(EQ_PRESETS[preset_name])
            self._current_preset = preset_name
            if self._on_preset_changed:
                self._on_preset_changed(preset_name)
            if self._on_all_gains_changed:
                self._on_all_gains_changed(self.bands)
            return True
        return False

    def reset_flat(self):
        """Resets all bands to 0.0 dB (Flat preset)."""
        self.set_preset("Flat")

    # Formatted Labels
    @classmethod
    def format_frequency(cls, freq_hz: int) -> str:
        if freq_hz >= 1000:
            return f"{freq_hz // 1000}k"
        return f"{freq_hz}"

    def band_label(self, band_index: int) -> str:
        if 0 <= band_index < EQ_NUM_BANDS:
            freq = ISO_CENTER_FREQUENCIES[band_index]
            return self.format_frequency(freq)
        return ""

    def gain_string(self, band_index: int) -> str:
        gain = self.band_gain(band_index)
        return f"{gain:+.1f}dB"

    def gain_color(self, band_index: int) -> str:
        gain = self.band_gain(band_index)
        if gain > 0.05:
            return ACCENT_SIGNAL_LIME
        elif gain < -0.05:
            return ACCENT_SAFETY_ORANGE
        else:
            return TEXT_SECONDARY_DIM

    def resize(self, width: int, height: int):
        self._width = max(300, int(width))
        self._height = max(100, int(height))
