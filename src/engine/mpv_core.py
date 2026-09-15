"""
mpv_core.py - Robust libmpv Ctypes FFI Binding & High-Level PlaybackEngine Controller.

Implements:
- Direct C-API ctypes binding to libmpv (/lib/x86_64-linux-gnu/libmpv.so.2).
- Complete error checking, memory management (mpv_free), and thread safety.
- High-level PlaybackEngine controller exposing:
  - Transport: load, play, pause, toggle_pause, stop, seek_ms, seek_smpte,
    frame stepping (< 1F / 1F >), relative jumps (+/- 10s).
  - Scaled pitch-preserved playback rate control (0.5x to 2.0x).
  - Audio and subtitle track discovery, inspection, and live switching.
  - External subtitle loading (.srt, .ass, .vtt).
  - A-B repeat looping (ab-loop-a / ab-loop-b).
  - 10-Band Graphic Equalizer DSP + Night Mode Dialogue Compressor (`af`/`lavfi`).
  - Deband dithering video processing filter.
  - Lossless forensic PNG screenshot capture with sidecar telemetry metadata.
  - OSD & real-time telemetry diagnostics HUD (FPS, dropped frames, bitrate, codec).
  - Dual-path real-time VU meter level sampling.
  - Background event pump with callback dispatching for seamless GUI & MPRIS2 bindings.
  - Headless execution mode with `vo=null,ao=null`.
"""

from __future__ import annotations
import ctypes
from ctypes import (
    POINTER, Structure, c_char, c_char_p, c_double, c_int, c_int64,
    c_size_t, c_uint64, c_ulong, c_void_p, cast, create_string_buffer
)
from dataclasses import asdict, dataclass, field
from enum import Enum
import json
import locale
import math
import os
from pathlib import Path
import re
import threading
import time
from typing import Any, Callable, Dict, List, Optional, Tuple, Union

from src.engine.audio_dsp import BiquadPeakingEQ, VUMeterDSP
from src.engine.screenshot import ScreenshotExporter, ScreenshotMetadata
from src.engine.smpte import SMPTETimecode
from src.engine.vu_analyzer import VUAnalyzer


# =============================================================================
# 1. LIBMPV CTYPES C-API DEFINITIONS & FFI BINDING
# =============================================================================

class MPVFormat(c_int):
    NONE = 0
    STRING = 1
    OSD_STRING = 2
    FLAG = 3
    INT64 = 4
    DOUBLE = 5
    NODE = 6
    NODE_ARRAY = 7
    NODE_MAP = 8
    BYTE_ARRAY = 9


class MPVEventId(c_int):
    NONE = 0
    SHUTDOWN = 1
    LOG_MESSAGE = 2
    GET_PROPERTY_REPLY = 3
    SET_PROPERTY_REPLY = 4
    COMMAND_REPLY = 5
    START_FILE = 6
    END_FILE = 7
    FILE_LOADED = 8
    CLIENT_MESSAGE = 16
    VIDEO_RECONFIG = 17
    AUDIO_RECONFIG = 18
    SEEK = 20
    PLAYBACK_RESTART = 21
    PROPERTY_CHANGE = 22
    QUEUE_OVERFLOW = 24
    HOOK = 25


class MPVEvent(Structure):
    _fields_ = [
        ("event_id", c_int),
        ("error", c_int),
        ("reply_userdata", c_uint64),
        ("data", c_void_p),
    ]


class MPVEventProperty(Structure):
    _fields_ = [
        ("name", c_char_p),
        ("format", c_int),
        ("data", c_void_p),
    ]


class MPVError(Exception):
    """Exception raised when a libmpv C-API call returns an error code."""
    def __init__(self, error_code: int, error_str: str = ""):
        self.error_code = error_code
        self.error_str = error_str or f"libmpv error code {error_code}"
        super().__init__(self.error_str)


def _load_libmpv_dll() -> ctypes.CDLL:
    """Discovers and dynamically loads the libmpv shared library."""
    candidate_paths = [
        "/lib/x86_64-linux-gnu/libmpv.so.2",
        "/usr/lib/x86_64-linux-gnu/libmpv.so.2",
        "/lib/x86_64-linux-gnu/libmpv.so",
        "/usr/lib/x86_64-linux-gnu/libmpv.so",
        "libmpv.so.2",
        "libmpv.so",
    ]
    for path in candidate_paths:
        try:
            return ctypes.cdll.LoadLibrary(path)
        except OSError:
            continue

    from ctypes.util import find_library
    discovered = find_library("mpv")
    if discovered:
        try:
            return ctypes.cdll.LoadLibrary(discovered)
        except OSError:
            pass

    raise OSError("Could not locate libmpv.so.2 dynamic library on this system")


class MPVHandle:
    """Low-level wrapper over a raw mpv_handle pointer."""

    _lib = None
    _lib_lock = threading.Lock()

    @classmethod
    def get_lib(cls) -> ctypes.CDLL:
        with cls._lib_lock:
            if cls._lib is None:
                cls._lib = _load_libmpv_dll()
                cls._setup_prototypes(cls._lib)
            return cls._lib

    @classmethod
    def _setup_prototypes(cls, lib: ctypes.CDLL):
        lib.mpv_client_api_version.restype = c_ulong
        lib.mpv_client_api_version.argtypes = []

        lib.mpv_error_string.restype = c_char_p
        lib.mpv_error_string.argtypes = [c_int]

        lib.mpv_create.restype = c_void_p
        lib.mpv_create.argtypes = []

        lib.mpv_initialize.restype = c_int
        lib.mpv_initialize.argtypes = [c_void_p]

        lib.mpv_destroy.restype = None
        lib.mpv_destroy.argtypes = [c_void_p]

        lib.mpv_terminate_destroy.restype = None
        lib.mpv_terminate_destroy.argtypes = [c_void_p]

        lib.mpv_command.restype = c_int
        lib.mpv_command.argtypes = [c_void_p, POINTER(c_char_p)]

        lib.mpv_command_string.restype = c_int
        lib.mpv_command_string.argtypes = [c_void_p, c_char_p]

        lib.mpv_set_option.restype = c_int
        lib.mpv_set_option.argtypes = [c_void_p, c_char_p, c_int, c_void_p]

        lib.mpv_set_option_string.restype = c_int
        lib.mpv_set_option_string.argtypes = [c_void_p, c_char_p, c_char_p]

        lib.mpv_set_property.restype = c_int
        lib.mpv_set_property.argtypes = [c_void_p, c_char_p, c_int, c_void_p]

        lib.mpv_set_property_string.restype = c_int
        lib.mpv_set_property_string.argtypes = [c_void_p, c_char_p, c_char_p]

        lib.mpv_get_property.restype = c_int
        lib.mpv_get_property.argtypes = [c_void_p, c_char_p, c_int, c_void_p]

        lib.mpv_get_property_string.restype = c_void_p
        lib.mpv_get_property_string.argtypes = [c_void_p, c_char_p]

        lib.mpv_observe_property.restype = c_int
        lib.mpv_observe_property.argtypes = [c_void_p, c_uint64, c_char_p, c_int]

        lib.mpv_unobserve_property.restype = c_int
        lib.mpv_unobserve_property.argtypes = [c_void_p, c_uint64]

        lib.mpv_wait_event.restype = POINTER(MPVEvent)
        lib.mpv_wait_event.argtypes = [c_void_p, c_double]

        lib.mpv_wakeup.restype = None
        lib.mpv_wakeup.argtypes = [c_void_p]

        lib.mpv_free.restype = None
        lib.mpv_free.argtypes = [c_void_p]

    def __init__(self):
        self.lib = self.get_lib()
        self.handle: Optional[c_void_p] = self.lib.mpv_create()
        if not self.handle:
            raise RuntimeError("Failed to create libmpv handle instance via mpv_create()")

    def error_string(self, code: int) -> str:
        res = self.lib.mpv_error_string(code)
        return res.decode("utf-8") if res else f"error {code}"

    def check_err(self, code: int, action: str = ""):
        if code < 0:
            err_msg = self.error_string(code)
            raise MPVError(code, f"MPV error {code} ({err_msg}) during {action}")

    def initialize(self):
        code = self.lib.mpv_initialize(self.handle)
        self.check_err(code, "mpv_initialize")

    def destroy(self):
        if self.handle:
            self.lib.mpv_terminate_destroy(self.handle)
            self.handle = None

    def command(self, *args: str):
        if not self.handle:
            return
        c_args = [c_char_p(arg.encode("utf-8")) for arg in args] + [c_char_p(None)]
        arr = (c_char_p * len(c_args))(*c_args)
        code = self.lib.mpv_command(self.handle, arr)
        self.check_err(code, f"command {args}")

    def command_string(self, cmd_str: str):
        if not self.handle:
            return
        code = self.lib.mpv_command_string(self.handle, cmd_str.encode("utf-8"))
        self.check_err(code, f"command_string '{cmd_str}'")

    def set_option_string(self, name: str, value: str):
        if not self.handle:
            return
        code = self.lib.mpv_set_option_string(self.handle, name.encode("utf-8"), value.encode("utf-8"))
        self.check_err(code, f"set_option_string {name}={value}")

    def set_property_string(self, name: str, value: str):
        if not self.handle:
            return
        code = self.lib.mpv_set_property_string(self.handle, name.encode("utf-8"), value.encode("utf-8"))
        self.check_err(code, f"set_property_string {name}={value}")

    def set_property_flag(self, name: str, value: bool):
        if not self.handle:
            return
        c_val = c_int(1 if value else 0)
        code = self.lib.mpv_set_property(
            self.handle, name.encode("utf-8"), MPVFormat.FLAG, ctypes.byref(c_val)
        )
        self.check_err(code, f"set_property_flag {name}={value}")

    def set_property_double(self, name: str, value: float):
        if not self.handle:
            return
        c_val = c_double(value)
        code = self.lib.mpv_set_property(
            self.handle, name.encode("utf-8"), MPVFormat.DOUBLE, ctypes.byref(c_val)
        )
        self.check_err(code, f"set_property_double {name}={value}")

    def set_property_int64(self, name: str, value: int):
        if not self.handle:
            return
        c_val = c_int64(value)
        code = self.lib.mpv_set_property(
            self.handle, name.encode("utf-8"), MPVFormat.INT64, ctypes.byref(c_val)
        )
        self.check_err(code, f"set_property_int64 {name}={value}")

    def get_property_string(self, name: str) -> Optional[str]:
        if not self.handle:
            return None
        ptr = self.lib.mpv_get_property_string(self.handle, name.encode("utf-8"))
        if not ptr:
            return None
        try:
            val_str = cast(ptr, c_char_p).value
            return val_str.decode("utf-8", errors="ignore") if val_str else None
        finally:
            self.lib.mpv_free(ptr)

    def get_property_osd(self, name: str) -> Optional[str]:
        if not self.handle:
            return None
        c_str_ptr = c_void_p()
        code = self.lib.mpv_get_property(
            self.handle, name.encode("utf-8"), MPVFormat.OSD_STRING, ctypes.byref(c_str_ptr)
        )
        if code < 0 or not c_str_ptr:
            return None
        try:
            val_str = cast(c_str_ptr, c_char_p).value
            return val_str.decode("utf-8", errors="ignore") if val_str else None
        finally:
            self.lib.mpv_free(c_str_ptr)

    def get_property_flag(self, name: str) -> Optional[bool]:
        if not self.handle:
            return None
        c_val = c_int(0)
        code = self.lib.mpv_get_property(
            self.handle, name.encode("utf-8"), MPVFormat.FLAG, ctypes.byref(c_val)
        )
        if code < 0:
            return None
        return bool(c_val.value)

    def get_property_double(self, name: str) -> Optional[float]:
        if not self.handle:
            return None
        c_val = c_double(0.0)
        code = self.lib.mpv_get_property(
            self.handle, name.encode("utf-8"), MPVFormat.DOUBLE, ctypes.byref(c_val)
        )
        if code < 0:
            return None
        return float(c_val.value)

    def get_property_int64(self, name: str) -> Optional[int]:
        if not self.handle:
            return None
        c_val = c_int64(0)
        code = self.lib.mpv_get_property(
            self.handle, name.encode("utf-8"), MPVFormat.INT64, ctypes.byref(c_val)
        )
        if code < 0:
            return None
        return int(c_val.value)

    def observe_property(self, reply_userdata: int, name: str, mpv_format: int = MPVFormat.NONE):
        if not self.handle:
            return
        code = self.lib.mpv_observe_property(
            self.handle, c_uint64(reply_userdata), name.encode("utf-8"), mpv_format
        )
        self.check_err(code, f"observe_property {name}")

    def unobserve_property(self, reply_userdata: int):
        if not self.handle:
            return
        code = self.lib.mpv_unobserve_property(self.handle, c_uint64(reply_userdata))
        self.check_err(code, f"unobserve_property {reply_userdata}")

    def wait_event(self, timeout: float = 0.0) -> Optional[MPVEvent]:
        if not self.handle:
            return None
        ptr = self.lib.mpv_wait_event(self.handle, c_double(timeout))
        return ptr.contents if ptr else None

    def wakeup(self):
        if self.handle:
            self.lib.mpv_wakeup(self.handle)


# =============================================================================
# 2. PLAYBACK ENGINE DATA STRUCTURES
# =============================================================================

class PlaybackState(str, Enum):
    STOPPED = "Stopped"
    PLAYING = "Playing"
    PAUSED = "Paused"
    BUFFERING = "Buffering"
    SEEKING = "Seeking"


@dataclass
class MediaTrack:
    """Represents an embedded or external audio/video/subtitle track."""
    id: int
    type: str  # "video", "audio", "sub"
    title: str = ""
    lang: str = ""
    codec: str = ""
    channels: int = 2
    selected: bool = False
    external: bool = False
    external_filename: str = ""


@dataclass
class TelemetryData:
    """Hardware diagnostics and real-time playback telemetry."""
    fps: float = 0.0
    estimated_fps: float = 0.0
    dropped_frames: int = 0
    video_codec: str = ""
    audio_codec: str = ""
    video_bitrate: int = 0
    audio_bitrate: int = 0
    resolution: str = ""
    video_width: int = 0
    video_height: int = 0
    pixel_format: str = ""
    hwdec: str = ""
    render_time_ms: float = 0.0
    av_sync_ms: float = 0.0

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


# =============================================================================
# 3. HIGH-LEVEL PLAYBACK ENGINE CONTROLLER
# =============================================================================

class PlaybackEngine:
    """
    High-level core media playback engine controller.
    """

    def __init__(
        self,
        wid: Optional[int] = None,
        headless: bool = False,
        hwdec: str = "auto-safe",
        log_level: str = "no",
    ):
        # Set C locale for numeric decimals
        try:
            locale.setlocale(locale.LC_NUMERIC, "C")
        except Exception:
            pass

        self.wid = wid
        self.headless = headless
        self.hwdec = hwdec
        self.log_level = log_level

        self.current_uri: str = ""
        self.state: PlaybackState = PlaybackState.STOPPED
        self._speed: float = 1.0
        self._volume: float = 100.0
        self._mute: bool = False
        self._ab_loop_a: Optional[float] = None
        self._ab_loop_b: Optional[float] = None
        self._night_mode: bool = False
        self._deband: bool = False
        self._eq_gains: List[float] = [0.0] * 10
        self._is_seeking: bool = False

        # Audio VU analyzer integration
        self.vu_analyzer: VUAnalyzer = VUAnalyzer(sample_rate_hz=50)

        # Callbacks
        self.on_position_changed: Optional[Callable[[float, str], None]] = None
        self.on_duration_changed: Optional[Callable[[float], None]] = None
        self.on_state_changed: Optional[Callable[[PlaybackState], None]] = None
        self.on_track_list_changed: Optional[Callable[[List[MediaTrack]], None]] = None
        self.on_metadata_loaded: Optional[Callable[[Dict[str, Any]], None]] = None
        self.on_end_of_file: Optional[Callable[[], None]] = None

        # Threading & Event Pump
        self.mpv = MPVHandle()
        self._running: bool = False
        self._event_thread: Optional[threading.Thread] = None
        self._lock = threading.RLock()

        self._configure_and_initialize()

    def _configure_and_initialize(self):
        """Sets up default MPV options and initializes backend."""
        # Core options
        self.mpv.set_option_string("keep-open", "yes")
        self.mpv.set_option_string("audio-pitch-correction", "yes")
        self.mpv.set_option_string("sub-auto", "fuzzy")
        self.mpv.set_option_string("msg-level", f"all={self.log_level}")

        if self.headless:
            self.mpv.set_option_string("vo", "null")
            self.mpv.set_option_string("ao", "null")
        else:
            if self.wid is not None:
                self.mpv.set_option_string("wid", str(int(self.wid)))
            self.mpv.set_option_string("hwdec", self.hwdec)

        self.mpv.initialize()

        # Start background event pumping thread
        self._running = True
        self._event_thread = threading.Thread(target=self._event_loop, daemon=True)
        self._event_thread.start()

    def _event_loop(self):
        """Background thread polling MPV events."""
        last_pos_ms = -1.0
        while self._running and self.mpv.handle:
            try:
                event = self.mpv.wait_event(0.02)
                if not event:
                    continue

                event_id = event.event_id
                if event_id == MPVEventId.NONE:
                    # Update position periodically
                    if self.is_playing():
                        pos_ms = self.get_position_ms()
                        if abs(pos_ms - last_pos_ms) >= 15.0:
                            last_pos_ms = pos_ms
                            smpte_str = self.get_position_smpte()
                            if self.on_position_changed:
                                self.on_position_changed(pos_ms, smpte_str)
                            self._check_ab_loop(pos_ms)
                    continue

                elif event_id == MPVEventId.FILE_LOADED:
                    self._on_file_loaded()

                elif event_id == MPVEventId.END_FILE:
                    self._on_end_file()

                elif event_id == MPVEventId.SEEK:
                    self._is_seeking = False
                    pos_ms = self.get_position_ms()
                    smpte_str = self.get_position_smpte()
                    if self.on_position_changed:
                        self.on_position_changed(pos_ms, smpte_str)

                elif event_id == MPVEventId.SHUTDOWN:
                    break

            except Exception:
                if not self._running:
                    break

    def _on_file_loaded(self):
        """Triggered when a new media stream is ready."""
        dur_ms = self.get_duration_ms()
        if self.on_duration_changed:
            self.on_duration_changed(dur_ms)

        tracks = self.get_tracks()
        if self.on_track_list_changed:
            self.on_track_list_changed(tracks)

        meta = self.get_metadata()
        if self.on_metadata_loaded:
            self.on_metadata_loaded(meta)

        # Pre-extract waveform peaks if local file
        if self.current_uri and not self.current_uri.startswith(("http://", "https://", "rtsp://")):
            threading.Thread(
                target=self.vu_analyzer.extract_from_audio_file,
                args=(self.current_uri,),
                daemon=True,
            ).start()

        self._set_state(PlaybackState.PLAYING if not self.is_paused() else PlaybackState.PAUSED)

    def _on_end_file(self):
        """Triggered upon reaching end of file."""
        self._set_state(PlaybackState.STOPPED)
        if self.on_end_of_file:
            self.on_end_of_file()

    def _check_ab_loop(self, current_pos_ms: float):
        """Enforces A-B repeat boundary jumping."""
        if self._ab_loop_a is not None and self._ab_loop_b is not None:
            if self._ab_loop_b > self._ab_loop_a:
                if current_pos_ms >= self._ab_loop_b:
                    self.seek_ms(self._ab_loop_a, exact=True)

    def _set_state(self, new_state: PlaybackState):
        if self.state != new_state:
            self.state = new_state
            if self.on_state_changed:
                self.on_state_changed(new_state)

    # -------------------------------------------------------------------------
    # Transport Controls
    # -------------------------------------------------------------------------

    def load(self, uri: str, auto_play: bool = True):
        """Loads a file or network stream URL."""
        with self._lock:
            self.current_uri = str(uri)
            self._ab_loop_a = None
            self._ab_loop_b = None
            self.vu_analyzer.clear_cache()

            self.mpv.command("loadfile", self.current_uri, "replace")
            if not auto_play:
                self.pause()
            else:
                self.play()
            self._set_state(PlaybackState.PLAYING if auto_play else PlaybackState.PAUSED)

    def play(self):
        """Resumes playback."""
        with self._lock:
            self.mpv.set_property_flag("pause", False)
            self._set_state(PlaybackState.PLAYING)

    def pause(self):
        """Pauses playback."""
        with self._lock:
            self.mpv.set_property_flag("pause", True)
            self._set_state(PlaybackState.PAUSED)

    def toggle_pause(self):
        """Toggles between play and pause."""
        with self._lock:
            if self.is_paused():
                self.play()
            else:
                self.pause()

    def stop(self):
        """Stops playback and resets position to 0."""
        with self._lock:
            self.mpv.command("stop")
            self._set_state(PlaybackState.STOPPED)

    def seek_ms(self, pos_ms: float, exact: bool = True):
        """Seeks to absolute millisecond timestamp."""
        with self._lock:
            target_sec = max(0.0, float(pos_ms) / 1000.0)
            mode = "absolute+exact" if exact else "absolute+keyframes"
            try:
                self.mpv.command("seek", f"{target_sec:.4f}", mode)
            except MPVError as e:
                if e.error_code not in (-12, -1):
                    raise

    def seek_relative_ms(self, offset_ms: float, exact: bool = True):
        """Seeks relative to current position in milliseconds."""
        with self._lock:
            target_sec = float(offset_ms) / 1000.0
            mode = "relative+exact" if exact else "relative+keyframes"
            try:
                self.mpv.command("seek", f"{target_sec:.4f}", mode)
            except MPVError as e:
                if e.error_code not in (-12, -1):
                    raise

    def seek_smpte(self, tc_str: str, fps: Optional[float] = None):
        """Seeks to SMPTE timecode string."""
        with self._lock:
            target_fps = fps or self.get_fps() or 30.0
            ms_val = SMPTETimecode.timecode_to_ms(tc_str, target_fps)
            self.seek_ms(ms_val, exact=True)

    def frame_step_forward(self):
        """Steps forward by 1 frame (1F >) and pauses."""
        with self._lock:
            try:
                self.mpv.command("frame-step")
                self._set_state(PlaybackState.PAUSED)
            except MPVError as e:
                if e.error_code not in (-12, -1):
                    raise

    def frame_step_backward(self):
        """Steps backward by 1 frame (< 1F) and pauses."""
        with self._lock:
            try:
                self.mpv.command("frame-back-step")
                self._set_state(PlaybackState.PAUSED)
            except MPVError as e:
                if e.error_code not in (-12, -1):
                    raise

    def jump_forward(self, seconds: float = 10.0):
        """Jumps forward by N seconds (default +10s)."""
        self.seek_relative_ms(seconds * 1000.0)

    def jump_backward(self, seconds: float = 10.0):
        """Jumps backward by N seconds (default -10s)."""
        self.seek_relative_ms(-seconds * 1000.0)

    # -------------------------------------------------------------------------
    # Speed, Volume, and Mute
    # -------------------------------------------------------------------------

    def set_speed(self, speed: float):
        """Sets playback speed multiplier [0.5, 2.0]."""
        with self._lock:
            clamped = max(0.5, min(2.0, float(speed)))
            self._speed = clamped
            self.mpv.set_property_double("speed", clamped)

    def get_speed(self) -> float:
        val = self.mpv.get_property_double("speed")
        return val if val is not None else self._speed

    def set_volume(self, vol: Union[int, float]):
        """Sets audio volume [0, 100]."""
        with self._lock:
            clamped = max(0.0, min(100.0, float(vol)))
            self._volume = clamped
            self.mpv.set_property_double("volume", clamped)

    def get_volume(self) -> float:
        val = self.mpv.get_property_double("volume")
        return val if val is not None else self._volume

    def set_mute(self, mute: bool):
        """Mutes or unmutes audio."""
        with self._lock:
            self._mute = bool(mute)
            self.mpv.set_property_flag("mute", self._mute)

    def is_muted(self) -> bool:
        val = self.mpv.get_property_flag("mute")
        return val if val is not None else self._mute

    # -------------------------------------------------------------------------
    # State and Positions
    # -------------------------------------------------------------------------

    def is_playing(self) -> bool:
        if self.state == PlaybackState.STOPPED:
            return False
        return not self.is_paused()

    def is_paused(self) -> bool:
        if self.state == PlaybackState.STOPPED:
            return False
        if self.state == PlaybackState.PAUSED:
            return True
        val = self.mpv.get_property_flag("pause")
        return bool(val) if val is not None else False

    def is_stopped(self) -> bool:
        return self.state == PlaybackState.STOPPED

    def get_position_ms(self) -> float:
        val = self.mpv.get_property_double("time-pos")
        return (val * 1000.0) if (val is not None and val >= 0) else 0.0

    def get_duration_ms(self) -> float:
        val = self.mpv.get_property_double("duration")
        return (val * 1000.0) if (val is not None and val >= 0) else 0.0

    def get_fps(self) -> float:
        val = self.mpv.get_property_double("container-fps")
        if val is None or val <= 0:
            val = self.mpv.get_property_double("estimated-vf-fps")
        return float(val) if (val is not None and val > 0) else 30.0

    def get_position_smpte(self, fps: Optional[float] = None) -> str:
        f = fps or self.get_fps()
        return SMPTETimecode.format_timecode_ms(self.get_position_ms(), fps=f, drop_frame=False)

    def get_remaining_smpte(self, fps: Optional[float] = None) -> str:
        f = fps or self.get_fps()
        return SMPTETimecode.format_remaining_timecode_ms(
            self.get_position_ms(), self.get_duration_ms(), fps=f, drop_frame=False
        )

    # -------------------------------------------------------------------------
    # Track & Subtitle Management
    # -------------------------------------------------------------------------

    @staticmethod
    def parse_channel_layout(layout: Any) -> int:
        """Safely parses audio channel layout/count from track metadata dict or raw layout string/int.

        Handles integer channel counts and string layouts such as 'mono' (1), 'stereo' (2),
        '2.1' (3), '5.1' (6), '7.1' (8), 'unknownX', and falls back cleanly to default 2
        without raising ValueError or any other exception.
        """
        if isinstance(layout, (int, float)):
            try:
                val = int(layout)
                return val if val > 0 else 2
            except (ValueError, TypeError, OverflowError):
                return 2

        raw_val = None
        if isinstance(layout, dict):
            # Prefer demux-channel-count, then demux-channels, then audio-channels
            raw_val = layout.get("demux-channel-count")
            if raw_val is None:
                raw_val = layout.get("demux-channels")
            if raw_val is None:
                raw_val = layout.get("audio-channels")
        else:
            raw_val = layout

        if raw_val is None:
            return 2

        if isinstance(raw_val, (int, float)):
            try:
                val = int(raw_val)
                return val if val > 0 else 2
            except (ValueError, TypeError, OverflowError):
                return 2

        s = str(raw_val).strip().lower()
        if not s:
            return 2

        if s.isdigit():
            try:
                val = int(s)
                return val if val > 0 else 2
            except (ValueError, TypeError):
                return 2

        known_layouts = {
            "mono": 1,
            "stereo": 2,
            "2.0": 2,
            "2.1": 3,
            "3.0": 3,
            "3.1": 4,
            "4.0": 4,
            "quad": 4,
            "quad(side)": 4,
            "4.1": 5,
            "5.0": 5,
            "5.0(side)": 5,
            "5.1": 6,
            "5.1(side)": 6,
            "5.1(alu)": 6,
            "6.0": 6,
            "6.0(front)": 6,
            "hexagonal": 6,
            "6.1": 7,
            "6.1(back)": 7,
            "6.1(front)": 7,
            "7.0": 7,
            "7.0(front)": 7,
            "7.1": 8,
            "7.1(wide)": 8,
            "7.1(wide-side)": 8,
            "7.1(rear)": 8,
            "octagonal": 8,
            "5.1.2": 8,
            "5.1.4": 10,
            "7.1.2": 10,
            "7.1.4": 12,
        }
        if s in known_layouts:
            return known_layouts[s]

        # Pattern: unknownX (e.g., unknown1, unknown2, unknown6)
        m_unknown = re.match(r"^unknown(\d+)$", s)
        if m_unknown:
            try:
                val = int(m_unknown.group(1))
                return val if val > 0 else 2
            except (ValueError, TypeError):
                return 2

        # Pattern: X.Y or X.Y.Z (e.g., 5.1(side), 7.1(wide), 2.1, 5.1.2)
        m_dots = re.match(r"^(\d+)\.(\d+)(?:\.(\d+))?", s)
        if m_dots:
            try:
                ch = int(m_dots.group(1)) + int(m_dots.group(2))
                if m_dots.group(3):
                    ch += int(m_dots.group(3))
                return ch if ch > 0 else 2
            except (ValueError, TypeError):
                return 2

        # Pattern: dash-separated channel names (e.g., "fl-fr-fc-lfe-bl-br")
        if "-" in s:
            parts = [p for p in s.split("-") if p]
            if len(parts) > 1:
                return len(parts)

        return 2

    _parse_channel_count = parse_channel_layout

    def get_tracks(self, track_type: Optional[str] = None) -> List[MediaTrack]:
        """Queries MPV for embedded and external track listing."""
        tracks: List[MediaTrack] = []
        track_list_json = self.mpv.get_property_string("track-list")
        if not track_list_json:
            return tracks

        try:
            raw_list = json.loads(track_list_json)
            if not isinstance(raw_list, list):
                return tracks
            for item in raw_list:
                if not isinstance(item, dict):
                    continue
                t_type = item.get("type", "")
                if track_type and t_type != track_type:
                    continue
                try:
                    channels = self.parse_channel_layout(item)
                    tracks.append(
                        MediaTrack(
                            id=int(item.get("id", 1)),
                            type=t_type,
                            title=str(item.get("title", "")),
                            lang=str(item.get("lang", "")),
                            codec=str(item.get("codec", "")),
                            channels=channels,
                            selected=bool(item.get("selected", False)),
                            external=bool(item.get("external", False)),
                            external_filename=str(item.get("external-filename", "")),
                        )
                    )
                except Exception:
                    continue
        except Exception:
            pass
        return tracks

    def get_audio_tracks(self) -> List[MediaTrack]:
        return self.get_tracks("audio")

    def set_audio_track(self, track_id: Union[int, str]):
        """Selects active audio track ID or 'no'."""
        with self._lock:
            self.mpv.set_property_string("aid", str(track_id))

    def get_subtitle_tracks(self) -> List[MediaTrack]:
        return self.get_tracks("sub")

    def set_subtitle_track(self, track_id: Union[int, str]):
        """Selects active subtitle track ID or 'no'."""
        with self._lock:
            self.mpv.set_property_string("sid", str(track_id))

    def load_external_subtitle(self, path: Union[str, Path], auto_select: bool = True):
        """Loads an external .srt, .ass, or .vtt subtitle file."""
        with self._lock:
            flag = "select" if auto_select else "auto"
            self.mpv.command("sub-add", str(path), flag)

    # -------------------------------------------------------------------------
    # Audio DSP: Equalizer, Night Mode, and Video Deband
    # -------------------------------------------------------------------------

    def _sync_audio_filter_chain(self):
        """Re-synthesizes and updates the active MPV `af` filter graph."""
        af_str = BiquadPeakingEQ.generate_mpv_af_string(self._eq_gains, night_mode=self._night_mode)
        self.mpv.set_property_string("af", af_str)

    def set_equalizer_gains(self, gains: Sequence[float]):
        """Sets 10-band graphic EQ gains in dB [-12.0, +12.0]."""
        with self._lock:
            if len(gains) != 10:
                raise ValueError(f"Expected 10 equalizer band gains, got {len(gains)}")
            self._eq_gains = [BiquadPeakingEQ.clamp_gain(g) for g in gains]
            self._sync_audio_filter_chain()

    def get_equalizer_gains(self) -> List[float]:
        return list(self._eq_gains)

    def set_equalizer_preset(self, preset_name: str):
        """Applies a named equalizer factory preset."""
        with self._lock:
            if preset_name not in BiquadPeakingEQ.PRESETS:
                raise ValueError(f"Unknown EQ preset '{preset_name}'")
            self.set_equalizer_gains(BiquadPeakingEQ.PRESETS[preset_name])

    def set_night_mode(self, enabled: bool):
        """Enables/disables Night Mode dynamic dialogue compressor."""
        with self._lock:
            self._night_mode = bool(enabled)
            self._sync_audio_filter_chain()

    def is_night_mode(self) -> bool:
        return self._night_mode

    def set_deband(self, enabled: bool):
        """Toggles GPU deband & dithering filter."""
        with self._lock:
            self._deband = bool(enabled)
            self.mpv.set_property_flag("deband", self._deband)

    def is_deband(self) -> bool:
        return self._deband

    # -------------------------------------------------------------------------
    # A-B Repeat Looping
    # -------------------------------------------------------------------------

    def set_ab_loop(self, point_a_ms: Optional[float], point_b_ms: Optional[float]):
        """Configures A-B repeat loop boundaries."""
        with self._lock:
            self._ab_loop_a = point_a_ms
            self._ab_loop_b = point_b_ms
            if point_a_ms is not None:
                self.mpv.set_property_double("ab-loop-a", point_a_ms / 1000.0)
            else:
                self.mpv.set_property_string("ab-loop-a", "no")

            if point_b_ms is not None:
                self.mpv.set_property_double("ab-loop-b", point_b_ms / 1000.0)
            else:
                self.mpv.set_property_string("ab-loop-b", "no")

    def clear_ab_loop(self):
        """Clears A-B repeat loop points."""
        self.set_ab_loop(None, None)

    def get_ab_loop(self) -> Tuple[Optional[float], Optional[float]]:
        return self._ab_loop_a, self._ab_loop_b

    # -------------------------------------------------------------------------
    # Forensic Screenshots
    # -------------------------------------------------------------------------

    def take_screenshot(
        self,
        output_path: Union[str, Path],
        mode: str = "video",
        include_sidecar: bool = True,
    ) -> ScreenshotMetadata:
        """
        Takes a frame-accurate lossless PNG screenshot and writes sidecar metadata.
        Modes: 'video', 'subtitles', 'window'.
        """
        with self._lock:
            dest = Path(output_path)
            dest.parent.mkdir(parents=True, exist_ok=True)

            # Ensure MPV screenshot format is PNG
            self.mpv.set_property_string("screenshot-format", "png")
            self.mpv.set_property_string("screenshot-png-compression", "7")
            self.mpv.set_property_string("screenshot-high-bit-depth", "yes")

            # Dispatch screenshot-to-file command
            self.mpv.command("screenshot-to-file", str(dest), mode)

            pos_ms = self.get_position_ms()
            fps = self.get_fps()
            telemetry = self.get_telemetry()

            meta = ScreenshotExporter.create_metadata(
                source_file=self.current_uri or str(dest.name),
                position_ms=pos_ms,
                fps=fps,
                resolution=telemetry.resolution or "1920x1080",
                video_codec=telemetry.video_codec or "unknown",
                audio_codec=telemetry.audio_codec or "unknown",
                screenshot_path=str(dest),
            )

            if include_sidecar:
                meta.write_sidecar_json()

            return meta

    # -------------------------------------------------------------------------
    # Telemetry & VU Meters
    # -------------------------------------------------------------------------

    def get_telemetry(self) -> TelemetryData:
        """Queries MPV core for live playback & rendering diagnostics."""
        fps = self.mpv.get_property_double("container-fps") or 0.0
        est_fps = self.mpv.get_property_double("estimated-vf-fps") or fps
        dropped = self.mpv.get_property_int64("drop-frame-count") or 0
        v_codec = self.mpv.get_property_string("video-codec") or ""
        a_codec = self.mpv.get_property_string("audio-codec") or ""
        v_bitrate = self.mpv.get_property_int64("video-bitrate") or 0
        a_bitrate = self.mpv.get_property_int64("audio-bitrate") or 0
        w = self.mpv.get_property_int64("video-out-params/w") or self.mpv.get_property_int64("width") or 0
        h = self.mpv.get_property_int64("video-out-params/h") or self.mpv.get_property_int64("height") or 0
        res_str = f"{w}x{h}" if (w > 0 and h > 0) else ""
        pix_fmt = self.mpv.get_property_string("video-out-params/pixelformat") or ""
        hwdec_curr = self.mpv.get_property_string("hwdec-current") or "no"
        av_sync = self.mpv.get_property_double("avsync") or 0.0

        return TelemetryData(
            fps=round(float(fps), 2),
            estimated_fps=round(float(est_fps), 2),
            dropped_frames=int(dropped),
            video_codec=v_codec,
            audio_codec=a_codec,
            video_bitrate=int(v_bitrate),
            audio_bitrate=int(a_bitrate),
            resolution=res_str,
            video_width=int(w),
            video_height=int(h),
            pixel_format=pix_fmt,
            hwdec=hwdec_curr,
            render_time_ms=0.0,
            av_sync_ms=round(float(av_sync * 1000.0), 3),
        )

    def get_metadata(self) -> Dict[str, Any]:
        """Extracts media tags and metadata dictionary."""
        meta: Dict[str, Any] = {}
        title = self.mpv.get_property_string("media-title")
        if title:
            meta["title"] = title

        # Extract metadata property list if present
        meta_json = self.mpv.get_property_string("metadata")
        if meta_json:
            try:
                parsed = json.loads(meta_json)
                if isinstance(parsed, dict):
                    meta.update(parsed)
            except Exception:
                pass
        return meta

    def get_vu_levels(self) -> Tuple[float, float]:
        """
        Returns instantaneous stereo Left and Right level amplitudes normalized in [0.0, 1.0].
        Uses the VUAnalyzer needle ballistics sampler.
        """
        if not self.is_playing():
            state = self.vu_analyzer.sample_at_time(0.0)
            return state.left_norm, state.right_norm

        pos_ms = self.get_position_ms()
        state = self.vu_analyzer.sample_at_time(pos_ms)
        return state.left_norm, state.right_norm

    # -------------------------------------------------------------------------
    # Lifecycle & Cleanup
    # -------------------------------------------------------------------------

    def terminate(self):
        """Cleanly terminates event loop thread and destroys MPV handle."""
        self._running = False
        if self.mpv and self.mpv.handle:
            self.mpv.wakeup()
        if self._event_thread and self._event_thread.is_alive():
            self._event_thread.join(timeout=1.0)
        if self.mpv and self.mpv.handle:
            self.mpv.destroy()

    def __del__(self):
        try:
            self.terminate()
        except Exception:
            pass
