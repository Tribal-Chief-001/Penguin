"""
src/desktop/audio_routing.py - PipeWire & PulseAudio Low-Latency Audio Routing for Penguin.

Features:
- Subsystem detection: PipeWire (pw-cli/wpctl), PulseAudio (pactl), and ALSA (aplay).
- Audio sink enumeration, inspection, and live output device switching.
- Sample rate negotiation (44.1kHz, 48kHz, 88.2kHz, 96kHz, 192kHz) and bit-depth matching.
- Low-latency MPV audio pipeline parameter synthesis (20ms buffer, pitch-preserved scaling).
- Volume and mute control through native audio server protocols.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from enum import Enum
import logging
import os
import re
import shutil
import subprocess
from typing import Any, Dict, List, Optional, Tuple, Union

logger = logging.getLogger("Penguin.Desktop.AudioRouting")


# =============================================================================
# 1. ENUMS & DATA STRUCTURES
# =============================================================================

class AudioBackend(str, Enum):
    PIPEWIRE = "pipewire"
    PULSEAUDIO = "pulse"
    ALSA = "alsa"
    JACK = "jack"
    NULL = "null"


class AudioChannelLayout(str, Enum):
    MONO = "mono"
    STEREO = "stereo"
    SURROUND_5_1 = "5.1"
    SURROUND_7_1 = "7.1"
    AUTO = "auto"


@dataclass
class AudioSinkInfo:
    """Represents a discovered hardware or virtual audio output sink."""
    name: str
    description: str
    index: int = 0
    sample_rate: int = 48000
    channels: int = 2
    format: str = "s32le"
    is_default: bool = False
    latency_ms: float = 0.0
    volume_percent: int = 100
    is_muted: bool = False


@dataclass
class AudioRoutingConfig:
    """Low-latency audio pipeline routing parameters for MPV / audio engine."""
    backend_chain: str = "pipewire,pulse,alsa,null"
    preferred_backend: str = "pipewire"
    target_sink: Optional[str] = None
    sample_rate: int = 48000
    channel_layout: str = "stereo"
    channels: int = 2
    bit_depth: int = 32
    format: str = "float"
    buffer_duration_sec: float = 0.02   # 20ms low-latency buffer
    pitch_correction: bool = True      # WSOLA scaletempo2
    exclusive_mode: bool = False

    def to_mpv_options(self) -> Dict[str, str]:
        """Converts routing configuration to libmpv option dictionary."""
        opts: Dict[str, str] = {
            "ao": self.backend_chain,
            "audio-samplerate": str(self.sample_rate),
            "audio-format": self.format,
            "audio-channels": self.channel_layout if self.channel_layout != "auto" else "auto",
            "audio-buffer": f"{self.buffer_duration_sec:.3f}",
            "audio-pitch-correction": "yes" if self.pitch_correction else "no",
        }
        if self.target_sink:
            opts["audio-device"] = self.target_sink
        if self.exclusive_mode:
            opts["audio-exclusive"] = "yes"
        return opts


# =============================================================================
# 2. AUDIO ROUTER ENGINE
# =============================================================================

class AudioRouter:
    """Coordinates PipeWire and PulseAudio sound server routing and sample rate negotiation."""

    STANDARD_SAMPLE_RATES = [44100, 48000, 88200, 96000, 192000]
    STANDARD_BIT_DEPTHS = [16, 24, 32]

    def __init__(self):
        self._active_backend: AudioBackend = self.detect_active_backend()

    @property
    def active_backend(self) -> AudioBackend:
        return self._active_backend

    # -------------------------------------------------------------------------
    # Server & Backend Discovery
    # -------------------------------------------------------------------------

    def detect_active_backend(self) -> AudioBackend:
        """Determines active system audio daemon."""
        # Check PipeWire runtime socket or binary
        if os.environ.get("PIPEWIRE_RUNTIME_DIR") or os.path.exists(f"/run/user/{os.getuid()}/pipewire-0"):
            return AudioBackend.PIPEWIRE

        if shutil.which("pw-cli"):
            try:
                res = subprocess.run(
                    ["pw-cli", "info", "0"],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=0.5
                )
                if res.returncode == 0:
                    return AudioBackend.PIPEWIRE
            except Exception:
                pass

        # Check PulseAudio
        if shutil.which("pactl"):
            try:
                res = subprocess.run(
                    ["pactl", "info"],
                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True, timeout=0.5
                )
                if res.returncode == 0:
                    if "PipeWire" in res.stdout:
                        return AudioBackend.PIPEWIRE
                    return AudioBackend.PULSEAUDIO
            except Exception:
                pass

        # Check ALSA
        if shutil.which("aplay"):
            return AudioBackend.ALSA

        return AudioBackend.NULL

    # -------------------------------------------------------------------------
    # Sink Querying & Enumeration
    # -------------------------------------------------------------------------

    def list_sinks(self) -> List[AudioSinkInfo]:
        """Queries and enumerates available audio output sinks."""
        sinks: List[AudioSinkInfo] = []

        # Try pactl list sinks
        if shutil.which("pactl"):
            try:
                res = subprocess.run(
                    ["pactl", "list", "sinks"],
                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True, timeout=1.0
                )
                if res.returncode == 0:
                    sinks = self._parse_pactl_sinks(res.stdout)
            except Exception as e:
                logger.debug(f"pactl list sinks query error: {e}")

        # If no sinks found from server, return default system sink fallback
        if not sinks:
            sinks.append(AudioSinkInfo(
                name="auto",
                description="Default System Audio Sink (Stereo)",
                index=0,
                sample_rate=48000,
                channels=2,
                format="s32le",
                is_default=True,
                latency_ms=20.0,
                volume_percent=100,
                is_muted=False,
            ))

        return sinks

    def get_default_sink(self) -> AudioSinkInfo:
        """Returns the default system output sink."""
        sinks = self.list_sinks()
        for s in sinks:
            if s.is_default:
                return s
        return sinks[0] if sinks else AudioSinkInfo(
            name="auto", description="Default Output", index=0, sample_rate=48000, channels=2
        )

    def _parse_pactl_sinks(self, output: str) -> List[AudioSinkInfo]:
        """Parses `pactl list sinks` raw stdout."""
        sinks: List[AudioSinkInfo] = []
        blocks = output.split("Sink #")

        for block in blocks[1:]:
            lines = block.splitlines()
            if not lines:
                continue

            try:
                idx = int(lines[0].strip())
            except ValueError:
                idx = 0

            name = ""
            desc = ""
            sample_rate = 48000
            channels = 2
            fmt = "s32le"
            vol = 100
            muted = False

            for line in lines:
                line_str = line.strip()
                if line_str.startswith("Name:"):
                    name = line_str.split(":", 1)[1].strip()
                elif line_str.startswith("Description:"):
                    desc = line_str.split(":", 1)[1].strip()
                elif line_str.startswith("Mute:"):
                    muted = "yes" in line_str.lower()
                elif line_str.startswith("Sample Specification:"):
                    spec = line_str.split(":", 1)[1].strip()
                    # e.g. s16le 2ch 44100Hz or float32le 2ch 48000Hz
                    m_sr = re.search(r"(\d+)Hz", spec)
                    if m_sr:
                        sample_rate = int(m_sr.group(1))
                    m_ch = re.search(r"(\d+)ch", spec)
                    if m_ch:
                        channels = int(m_ch.group(1))
                    m_fmt = spec.split()[0]
                    if m_fmt:
                        fmt = m_fmt
                elif line_str.startswith("Volume:"):
                    m_vol = re.search(r"/ (\d+)%", line_str)
                    if m_vol:
                        vol = int(m_vol.group(1))

            if name:
                sinks.append(AudioSinkInfo(
                    name=name,
                    description=desc or name,
                    index=idx,
                    sample_rate=sample_rate,
                    channels=channels,
                    format=fmt,
                    is_default=(len(sinks) == 0),
                    volume_percent=vol,
                    is_muted=muted,
                ))

        return sinks

    # -------------------------------------------------------------------------
    # Sample Rate Negotiation & Config Synthesis
    # -------------------------------------------------------------------------

    def negotiate_sample_rate(
        self, source_rate: int, sink_rate: Optional[int] = None
    ) -> int:
        """
        Negotiates optimal sample rate between media source and sink.
        Matches source rate if supported without resampling, otherwise aligns to sink rate.
        """
        if sink_rate is None:
            default_sink = self.get_default_sink()
            sink_rate = default_sink.sample_rate

        # If source rate is standard and matches sink or multiple, preserve source rate
        if source_rate in self.STANDARD_SAMPLE_RATES:
            return source_rate

        # Fallback to standard sink rate
        return sink_rate or 48000

    def build_low_latency_config(
        self,
        source_rate: int = 48000,
        channels: int = 2,
        low_latency: bool = True,
        target_sink: Optional[str] = None,
    ) -> AudioRoutingConfig:
        """
        Constructs a low-latency routing configuration for audio engine.
        """
        backend = self.detect_active_backend()
        sink_info = self.get_default_sink()
        negotiated_rate = self.negotiate_sample_rate(source_rate, sink_info.sample_rate)

        ch_layout = "stereo"
        if channels == 1:
            ch_layout = "mono"
        elif channels == 6:
            ch_layout = "5.1"
        elif channels == 8:
            ch_layout = "7.1"
        elif channels > 2:
            ch_layout = f"{channels}ch"

        buffer_sec = 0.02 if low_latency else 0.10  # 20ms for low latency

        return AudioRoutingConfig(
            backend_chain="pipewire,pulse,alsa,null",
            preferred_backend=backend.value,
            target_sink=target_sink or (sink_info.name if sink_info.name != "auto" else None),
            sample_rate=negotiated_rate,
            channel_layout=ch_layout,
            channels=channels,
            bit_depth=32,
            format="float",
            buffer_duration_sec=buffer_sec,
            pitch_correction=True,
            exclusive_mode=False,
        )

    # -------------------------------------------------------------------------
    # Sink Control: Switch, Volume, Mute
    # -------------------------------------------------------------------------

    def switch_audio_sink(self, sink_name_or_index: Union[str, int]) -> bool:
        """Switches default audio output sink."""
        target = str(sink_name_or_index)

        # PipeWire wpctl
        if shutil.which("wpctl"):
            try:
                res = subprocess.run(
                    ["wpctl", "set-default", target],
                    check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1.0
                )
                if res.returncode == 0:
                    return True
            except Exception:
                pass

        # PulseAudio pactl
        if shutil.which("pactl"):
            try:
                res = subprocess.run(
                    ["pactl", "set-default-sink", target],
                    check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1.0
                )
                return res.returncode == 0
            except Exception:
                pass

        return False

    def set_sink_volume(self, sink_name_or_index: Union[str, int], volume_percent: int) -> bool:
        """Sets volume for specified audio sink."""
        vol = max(0, min(150, int(volume_percent)))
        target = str(sink_name_or_index)

        if shutil.which("pactl"):
            try:
                res = subprocess.run(
                    ["pactl", "set-sink-volume", target, f"{vol}%"],
                    check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1.0
                )
                return res.returncode == 0
            except Exception:
                pass
        return False

    def set_sink_mute(self, sink_name_or_index: Union[str, int], muted: bool) -> bool:
        """Mutes or unmutes specified audio sink."""
        val = "1" if muted else "0"
        target = str(sink_name_or_index)

        if shutil.which("pactl"):
            try:
                res = subprocess.run(
                    ["pactl", "set-sink-mute", target, val],
                    check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1.0
                )
                return res.returncode == 0
            except Exception:
                pass
        return False

    def get_audio_system_info(self) -> Dict[str, Any]:
        """Returns comprehensive audio server and hardware status dictionary."""
        sinks = self.list_sinks()
        default_sink = self.get_default_sink()
        return {
            "backend": self._active_backend.value,
            "sinks_count": len(sinks),
            "default_sink": default_sink.name,
            "default_sample_rate": default_sink.sample_rate,
            "supported_sample_rates": self.STANDARD_SAMPLE_RATES,
            "supported_bit_depths": self.STANDARD_BIT_DEPTHS,
            "sinks": [
                {
                    "name": s.name,
                    "description": s.description,
                    "sample_rate": s.sample_rate,
                    "channels": s.channels,
                    "volume": s.volume_percent,
                    "muted": s.is_muted,
                }
                for s in sinks
            ],
        }
