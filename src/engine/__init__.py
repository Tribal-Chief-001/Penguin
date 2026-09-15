"""
Penguin Media Player — Engine & Audio DSP Backend.

Modules:
- mpv_core: Robust libmpv ctypes binding and PlaybackEngine controller.
- smpte: SMPTE 12M timecode calculations (NDF / DF).
- audio_dsp: 10-Band ISO graphic equalizer DSP, biquads, and night mode filter.
- vu_analyzer: Dual-path stereo peak and RMS audio waveform meter.
- lrc_parser: Synchronized .lrc lyric parser and teleprompter engine.
- screenshot: Forensic lossless PNG screenshot exporter with telemetry metadata.
"""

from src.engine.smpte import (
    SMPTETimecode,
    ms_to_smpte,
    smpte_to_ms,
    smpte_step_forward,
    smpte_step_backward,
)
from src.engine.audio_dsp import (
    BiquadPeakingEQ,
    EqualizerDSP,
    VUMeterDSP,
    ISO_CENTER_FREQUENCIES,
    DEFAULT_Q_FACTOR,
    MIN_GAIN_DB,
    MAX_GAIN_DB,
)
from src.engine.vu_analyzer import (
    VUAnalyzer,
    AudioLevelSample,
    VUState,
)
from src.engine.lrc_parser import (
    LRCParser,
    LRCCue,
    LRCToken,
)
from src.engine.screenshot import (
    ScreenshotExporter,
    ScreenshotMetadata,
)
from src.engine.mpv_core import (
    MPVHandle,
    MPVError,
    MPVFormat,
    MPVEventId,
    MPVEvent,
    PlaybackEngine,
    PlaybackState,
    MediaTrack,
    TelemetryData,
)

__all__ = [
    # SMPTE
    "SMPTETimecode",
    "ms_to_smpte",
    "smpte_to_ms",
    "smpte_step_forward",
    "smpte_step_backward",
    # Audio DSP & EQ
    "BiquadPeakingEQ",
    "EqualizerDSP",
    "VUMeterDSP",
    "ISO_CENTER_FREQUENCIES",
    "DEFAULT_Q_FACTOR",
    "MIN_GAIN_DB",
    "MAX_GAIN_DB",
    # VU Analyzer
    "VUAnalyzer",
    "AudioLevelSample",
    "VUState",
    # LRC Parser
    "LRCParser",
    "LRCCue",
    "LRCToken",
    # Forensic Screenshot
    "ScreenshotExporter",
    "ScreenshotMetadata",
    # MPV Core & Playback Engine
    "MPVHandle",
    "MPVError",
    "MPVFormat",
    "MPVEventId",
    "MPVEvent",
    "PlaybackEngine",
    "PlaybackState",
    "MediaTrack",
    "TelemetryData",
]
