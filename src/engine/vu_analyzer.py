"""
vu_analyzer.py - Dual-Path Stereo Peak & RMS Audio Waveform Analyzer.

Implements:
- Dual-Path Analysis Architecture:
  1. 50Hz PCM Waveform Cache (20ms time slices) for zero-latency lookups.
  2. 60 FPS Studio Needle Ballistics & Peak-Hold Decay Engine.
- Instantaneous fast attack (< 5ms) and smooth 20 dB/s studio decay (~300ms).
- 1.5-second peak-hold indicator dynamics.
- Overload / clipping detection (>= -0.5 dBFS) with sticky clip alert.
- Linear and dBFS normalization ([0.0, 1.0] mapping to [-60.0, 0.0] dBFS).
"""

from __future__ import annotations
import bisect
from dataclasses import dataclass, field
import math
import os
from pathlib import Path
import shutil
import struct
import subprocess
import threading
import time
from typing import List, Optional, Sequence, Tuple
import wave

from src.engine.audio_dsp import VUMeterDSP


@dataclass
class AudioLevelSample:
    """Represents an instantaneous audio level sample at a given timestamp."""
    time_ms: int
    left_peak: float  # [0.0, 1.0+] linear amplitude
    right_peak: float
    left_rms: float
    right_rms: float

    @property
    def left_peak_db(self) -> float:
        return 20.0 * math.log10(max(1e-6, self.left_peak))

    @property
    def right_peak_db(self) -> float:
        return 20.0 * math.log10(max(1e-6, self.right_peak))

    @property
    def left_rms_db(self) -> float:
        return 20.0 * math.log10(max(1e-6, self.left_rms))

    @property
    def right_rms_db(self) -> float:
        return 20.0 * math.log10(max(1e-6, self.right_rms))


@dataclass
class VUState:
    """Current state of stereo VU meter needles, peak-hold, and clip indicators."""
    left_norm: float = 0.0          # [0.0, 1.0]
    right_norm: float = 0.0         # [0.0, 1.0]
    left_hold_norm: float = 0.0     # [0.0, 1.0]
    right_hold_norm: float = 0.0    # [0.0, 1.0]
    left_db: float = -60.0          # dBFS
    right_db: float = -60.0         # dBFS
    left_hold_db: float = -60.0     # dBFS
    right_hold_db: float = -60.0    # dBFS
    left_clipped: bool = False
    right_clipped: bool = False
    zone_left: str = "nominal"      # "nominal", "headroom", "clipping"
    zone_right: str = "nominal"


class VUAnalyzer:
    """
    Dual-path real-time VU and Waveform Peak/RMS analyzer.
    """

    MIN_DB: float = -60.0
    MAX_DB: float = 3.0
    CLIP_THRESHOLD_DB: float = -0.5
    DECAY_RATE_DB_PER_SEC: float = 20.0  # ~300ms full decay
    PEAK_HOLD_TIME_SEC: float = 1.5      # Hold time before decay
    CLIP_HOLD_TIME_SEC: float = 0.6      # Hold time for clip LED

    def __init__(self, sample_rate_hz: int = 50):
        self.sample_rate_hz: int = sample_rate_hz
        self.interval_ms: int = int(round(1000.0 / sample_rate_hz))  # 20ms for 50Hz
        self.waveform_cache: List[AudioLevelSample] = []
        self._cache_times: List[int] = []
        self._lock = threading.Lock()

        # Ballistics state
        self._current_left_db: float = self.MIN_DB
        self._current_right_db: float = self.MIN_DB
        self._left_hold_db: float = self.MIN_DB
        self._right_hold_db: float = self.MIN_DB
        self._left_hold_timer: float = 0.0
        self._right_hold_timer: float = 0.0
        self._left_clip_timer: float = 0.0
        self._right_clip_timer: float = 0.0

    def clear_cache(self) -> None:
        """Clears pre-computed waveform cache and resets ballistics."""
        with self._lock:
            self.waveform_cache.clear()
            self._cache_times.clear()
            self.reset_ballistics()

    def reset_ballistics(self) -> None:
        """Resets needle dynamics to zero/silence."""
        self._current_left_db = self.MIN_DB
        self._current_right_db = self.MIN_DB
        self._left_hold_db = self.MIN_DB
        self._right_hold_db = self.MIN_DB
        self._left_hold_timer = 0.0
        self._right_hold_timer = 0.0
        self._left_clip_timer = 0.0
        self._right_clip_timer = 0.0

    def load_samples(self, samples: Sequence[AudioLevelSample]) -> None:
        """Loads a sequence of pre-computed AudioLevelSample instances."""
        with self._lock:
            self.waveform_cache = sorted(samples, key=lambda s: s.time_ms)
            self._cache_times = [s.time_ms for s in self.waveform_cache]

    def extract_from_audio_file(self, file_path: str | Path, max_duration_sec: float = 3600.0) -> bool:
        """
        Extracts 50Hz stereo audio waveform peaks from an audio or video file.
        Uses pure python `wave` if WAV, or FFmpeg pipe if available.
        """
        path = Path(file_path)
        if not path.exists():
            return False

        samples: List[AudioLevelSample] = []

        if path.suffix.lower() == ".wav":
            try:
                with wave.open(str(path), "rb") as wf:
                    channels = wf.getnchannels()
                    sampwidth = wf.getsampwidth()
                    framerate = wf.getframerate()
                    if sampwidth == 2 and channels in (1, 2) and framerate > 0:
                        chunk_frames = int(framerate / self.sample_rate_hz)
                        time_ms = 0
                        while True:
                            data = wf.readframes(chunk_frames)
                            if not data:
                                break
                            n_frames = len(data) // (sampwidth * channels)
                            if n_frames == 0:
                                break
                            raw_s = struct.unpack(f"<{n_frames * channels}h", data)
                            
                            l_peak = 0.0
                            r_peak = 0.0
                            l_sq = 0.0
                            r_sq = 0.0
                            
                            if channels == 1:
                                for val in raw_s:
                                    norm = abs(val) / 32768.0
                                    if norm > l_peak:
                                        l_peak = norm
                                    l_sq += norm * norm
                                l_rms = math.sqrt(l_sq / n_frames)
                                r_peak, r_rms = l_peak, l_rms
                            else:
                                for idx in range(0, len(raw_s), 2):
                                    lv = abs(raw_s[idx]) / 32768.0
                                    rv = abs(raw_s[idx + 1]) / 32768.0
                                    if lv > l_peak:
                                        l_peak = lv
                                    if rv > r_peak:
                                        r_peak = rv
                                    l_sq += lv * lv
                                    r_sq += rv * rv
                                l_rms = math.sqrt(l_sq / n_frames)
                                r_rms = math.sqrt(r_sq / n_frames)
                                
                            samples.append(AudioLevelSample(time_ms, l_peak, r_peak, l_rms, r_rms))
                            time_ms += self.interval_ms
                        self.load_samples(samples)
                        return True
            except Exception:
                pass

        # Try FFmpeg extraction fallback
        ffmpeg_bin = shutil.which("ffmpeg")
        if ffmpeg_bin:
            try:
                cmd = [
                    ffmpeg_bin,
                    "-v", "error",
                    "-i", str(path),
                    "-t", str(max_duration_sec),
                    "-vn",
                    "-f", "s16le",
                    "-ac", "2",
                    "-ar", "44100",
                    "-",
                ]
                proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
                chunk_bytes = int(44100 / self.sample_rate_hz) * 4  # 2 channels * 2 bytes
                time_ms = 0
                try:
                    while True:
                        data = proc.stdout.read(chunk_bytes)
                        if not data or len(data) < 4:
                            break
                        n_samples = len(data) // 4
                        raw_s = struct.unpack(f"<{n_samples * 2}h", data[:n_samples * 4])
                        
                        l_peak = 0.0
                        r_peak = 0.0
                        l_sq = 0.0
                        r_sq = 0.0
                        for idx in range(0, len(raw_s), 2):
                            lv = abs(raw_s[idx]) / 32768.0
                            rv = abs(raw_s[idx + 1]) / 32768.0
                            if lv > l_peak:
                                l_peak = lv
                            if rv > r_peak:
                                r_peak = rv
                            l_sq += lv * lv
                            r_sq += rv * rv
                        l_rms = math.sqrt(l_sq / n_samples)
                        r_rms = math.sqrt(r_sq / n_samples)
                        
                        samples.append(AudioLevelSample(time_ms, l_peak, r_peak, l_rms, r_rms))
                        time_ms += self.interval_ms
                finally:
                    if proc.stdout:
                        proc.stdout.close()
                    proc.wait()

                if samples:
                    self.load_samples(samples)
                    return True
            except Exception:
                pass

        return False

    def query_raw_levels(self, time_pos_ms: float) -> Tuple[float, float, float, float]:
        """
        Binary-search queries waveform cache for instantaneous (L_peak, R_peak, L_rms, R_rms).
        Returns linear amplitudes [0.0, 1.0].
        """
        with self._lock:
            if not self.waveform_cache:
                return 0.0, 0.0, 0.0, 0.0

            pos = int(round(time_pos_ms))
            if pos <= self._cache_times[0]:
                s = self.waveform_cache[0]
                return s.left_peak, s.right_peak, s.left_rms, s.right_rms
            if pos >= self._cache_times[-1]:
                s = self.waveform_cache[-1]
                return s.left_peak, s.right_peak, s.left_rms, s.right_rms

            idx = bisect.bisect_right(self._cache_times, pos) - 1
            idx = max(0, min(len(self.waveform_cache) - 1, idx))
            s = self.waveform_cache[idx]
            return s.left_peak, s.right_peak, s.left_rms, s.right_rms

    def update_ballistics(
        self,
        delta_time_sec: float,
        target_left_peak: float,
        target_right_peak: float,
    ) -> VUState:
        """
        Calculates 60 FPS studio needle ballistics:
        - Fast attack (< 5ms): needle jumps immediately to higher peak.
        - Studio decay (~300ms / 20 dB/s): smooth needle falloff.
        - Peak hold: holds max peak for 1.5s before decaying.
        - Clip detection: sticky clip alert on peaks >= -0.5 dBFS.
        """
        dt = max(0.001, delta_time_sec)

        # Convert target peaks to dBFS
        t_l_db = 20.0 * math.log10(max(1e-6, target_left_peak))
        t_r_db = 20.0 * math.log10(max(1e-6, target_right_peak))

        t_l_db = max(self.MIN_DB, min(self.MAX_DB, t_l_db))
        t_r_db = max(self.MIN_DB, min(self.MAX_DB, t_r_db))

        # 1. Left channel needle dynamics (Fast attack, 20dB/s decay)
        if t_l_db >= self._current_left_db:
            self._current_left_db = t_l_db  # Instant attack
        else:
            decay_amount = self.DECAY_RATE_DB_PER_SEC * dt
            self._current_left_db = max(self.MIN_DB, self._current_left_db - decay_amount)

        # 2. Right channel needle dynamics
        if t_r_db >= self._current_right_db:
            self._current_right_db = t_r_db  # Instant attack
        else:
            decay_amount = self.DECAY_RATE_DB_PER_SEC * dt
            self._current_right_db = max(self.MIN_DB, self._current_right_db - decay_amount)

        # 3. Peak-hold dynamics (Left)
        if t_l_db >= self._left_hold_db:
            self._left_hold_db = t_l_db
            self._left_hold_timer = self.PEAK_HOLD_TIME_SEC
        else:
            if self._left_hold_timer > 0.0:
                self._left_hold_timer -= dt
            else:
                self._left_hold_db = max(
                    self._current_left_db,
                    self._left_hold_db - (self.DECAY_RATE_DB_PER_SEC * dt),
                )

        # 4. Peak-hold dynamics (Right)
        if t_r_db >= self._right_hold_db:
            self._right_hold_db = t_r_db
            self._right_hold_timer = self.PEAK_HOLD_TIME_SEC
        else:
            if self._right_hold_timer > 0.0:
                self._right_hold_timer -= dt
            else:
                self._right_hold_db = max(
                    self._current_right_db,
                    self._right_hold_db - (self.DECAY_RATE_DB_PER_SEC * dt),
                )

        # 5. Overload Clip Detection (>= -0.5 dBFS)
        if t_l_db >= self.CLIP_THRESHOLD_DB:
            self._left_clip_timer = self.CLIP_HOLD_TIME_SEC
        elif self._left_clip_timer > 0.0:
            self._left_clip_timer -= dt

        if t_r_db >= self.CLIP_THRESHOLD_DB:
            self._right_clip_timer = self.CLIP_HOLD_TIME_SEC
        elif self._right_clip_timer > 0.0:
            self._right_clip_timer -= dt

        l_norm = VUMeterDSP.db_to_normalized(self._current_left_db, min_db=self.MIN_DB, max_db=0.0)
        r_norm = VUMeterDSP.db_to_normalized(self._current_right_db, min_db=self.MIN_DB, max_db=0.0)
        l_hold_norm = VUMeterDSP.db_to_normalized(self._left_hold_db, min_db=self.MIN_DB, max_db=0.0)
        r_hold_norm = VUMeterDSP.db_to_normalized(self._right_hold_db, min_db=self.MIN_DB, max_db=0.0)

        return VUState(
            left_norm=l_norm,
            right_norm=r_norm,
            left_hold_norm=l_hold_norm,
            right_hold_norm=r_hold_norm,
            left_db=self._current_left_db,
            right_db=self._current_right_db,
            left_hold_db=self._left_hold_db,
            right_hold_db=self._right_hold_db,
            left_clipped=(self._left_clip_timer > 0.0),
            right_clipped=(self._right_clip_timer > 0.0),
            zone_left=VUMeterDSP.classify_zone(self._current_left_db),
            zone_right=VUMeterDSP.classify_zone(self._current_right_db),
        )

    def sample_at_time(self, time_pos_ms: float, delta_time_sec: float = 1.0 / 60.0) -> VUState:
        """Queries waveform cache and advances 60fps needle ballistics."""
        l_peak, r_peak, _, _ = self.query_raw_levels(time_pos_ms)
        return self.update_ballistics(delta_time_sec, l_peak, r_peak)
