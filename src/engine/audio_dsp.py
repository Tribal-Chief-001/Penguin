"""
audio_dsp.py - 10-Band Graphic Equalizer DSP & Night Mode Dynamic Audio Filters.

Implements:
- 10-band ISO octave center frequencies (32 Hz to 16 kHz).
- Audio EQ Cookbook (Robert Bristow-Johnson) Biquad Peaking Filter math with Q=1.4142.
- Transfer function evaluation |H(z)| in dB.
- Factory presets (Flat, Rock, Pop, Jazz, Classical, Bass Boost, Vocal Boost, Electronic, Acoustic, Studio Mastering, Night Mode).
- Real-time live audio buffer IIR filtering (Direct Form II).
- MPV / FFmpeg audio filter (`af`/`lavfi`) pipeline synthesis.
- Night Mode dynamic dialogue compressor (`dynaudnorm` / `acompressor`).
- Analytical VU meter DSP calculations (Peak, RMS, Ballistics, Decay, Clipping detection).
"""

from __future__ import annotations
import cmath
from dataclasses import dataclass
import math
from typing import Dict, List, Optional, Sequence, Tuple


# ISO 266 Standard 1-octave band center frequencies in Hz
ISO_CENTER_FREQUENCIES: List[int] = [32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]

# Standard Q factor for 1-octave constant bandwidth filter (1 / (2 * sinh(ln(2)/2 * 1.0)) ~= 1.4142)
DEFAULT_Q_FACTOR: float = 1.4142

# Allowed gain range in dB
MIN_GAIN_DB: float = -12.0
MAX_GAIN_DB: float = 12.0


class BiquadPeakingEQ:
    """Calculates second-order Biquad Peaking EQ coefficients and frequency response."""

    ISO_BANDS: List[int] = ISO_CENTER_FREQUENCIES

    PRESETS: Dict[str, List[float]] = {
        "Flat": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "Rock": [4.5, 3.0, 1.5, 0.0, -1.0, -0.5, 1.5, 3.0, 4.0, 4.5],
        "Pop": [-1.0, 1.5, 3.0, 3.5, 2.0, -0.5, 1.5, 2.5, 3.0, 2.0],
        "Jazz": [3.0, 2.0, 1.0, 1.5, -1.5, -1.5, 0.0, 1.5, 2.5, 3.0],
        "Classical": [4.0, 3.0, 2.0, 1.5, -1.0, -1.0, 0.0, 2.0, 3.0, 3.5],
        "Bass Boost": [7.0, 6.0, 4.5, 2.5, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "Vocal Boost": [-2.0, -1.5, -1.0, 1.5, 3.5, 4.0, 3.0, 1.5, 0.0, -1.0],
        "Electronic": [5.5, 4.5, 2.0, 0.0, -2.0, 2.0, 1.0, 2.5, 4.5, 5.0],
        "Acoustic": [3.5, 2.5, 1.5, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 2.0],
        "Studio Mastering": [1.0, 1.5, 0.0, -0.5, 0.5, 1.0, 1.5, 2.0, 2.5, 2.0],
        "Night Mode": [-4.0, -3.0, -1.5, 0.0, 1.0, 2.0, 2.0, 1.0, -1.0, -3.0],
    }

    @classmethod
    def clamp_gain(cls, gain_db: float) -> float:
        """Clamps gain to [-12.0, +12.0] dB."""
        return max(MIN_GAIN_DB, min(MAX_GAIN_DB, float(gain_db)))

    @classmethod
    def compute_coefficients(
        cls,
        f0: float,
        gain_db: float,
        fs: float = 44100.0,
        q: float = DEFAULT_Q_FACTOR,
    ) -> Tuple[float, float, float, float, float]:
        """
        Computes normalized Biquad coefficients (b0, b1, b2, a1, a2) for peaking EQ.
        Based on Robert Bristow-Johnson's Audio EQ Cookbook:
          A = 10^(gain_db / 40)
          w0 = 2 * pi * f0 / fs
          alpha = sin(w0) / (2 * Q)
          b0 = 1 + alpha * A
          b1 = -2 * cos(w0)
          b2 = 1 - alpha * A
          a0 = 1 + alpha / A
          a1 = -2 * cos(w0)
          a2 = 1 - alpha / A
        Returns: (b0/a0, b1/a0, b2/a0, a1/a0, a2/a0) where a0 is normalized to 1.0.
        """
        clamped_gain = cls.clamp_gain(gain_db)
        if abs(clamped_gain) < 1e-6:
            # Flat response: H(z) = 1.0
            return 1.0, 0.0, 0.0, 0.0, 0.0

        if fs <= 0.0:
            fs = 44100.0
        if f0 >= fs / 2.0:
            # Frequency above or at Nyquist cannot be filtered; return identity
            return 1.0, 0.0, 0.0, 0.0, 0.0

        a_gain = 10.0 ** (clamped_gain / 40.0)
        omega0 = 2.0 * math.pi * f0 / fs
        alpha = math.sin(omega0) / (2.0 * q)
        cos_omega0 = math.cos(omega0)

        b0 = 1.0 + alpha * a_gain
        b1 = -2.0 * cos_omega0
        b2 = 1.0 - alpha * a_gain
        a0 = 1.0 + alpha / a_gain
        a1 = -2.0 * cos_omega0
        a2 = 1.0 - alpha / a_gain

        if abs(a0) < 1e-12:
            return 1.0, 0.0, 0.0, 0.0, 0.0

        return b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0

    @classmethod
    def evaluate_response_db(
        cls,
        b0: float,
        b1: float,
        b2: float,
        a1: float,
        a2: float,
        eval_freq: float,
        fs: float = 44100.0,
    ) -> float:
        """
        Evaluates magnitude response |H(e^{j 2pi f / fs})| in dB at eval_freq.
        """
        if fs <= 0.0:
            fs = 44100.0
        omega = 2.0 * math.pi * eval_freq / fs
        z_inv = cmath.exp(-1j * omega)
        z_inv2 = z_inv * z_inv

        num = b0 + b1 * z_inv + b2 * z_inv2
        den = 1.0 + a1 * z_inv + a2 * z_inv2

        if abs(den) < 1e-12:
            return 0.0
        h = num / den
        mag = abs(h)
        if mag < 1e-12:
            return -120.0
        return 20.0 * math.log10(mag)

    @classmethod
    def evaluate_system_response_db(
        cls,
        gains_db: Sequence[float],
        eval_freq: float,
        fs: float = 44100.0,
        q: float = DEFAULT_Q_FACTOR,
    ) -> float:
        """Evaluates the composite magnitude response across all 10 cascaded bands in dB."""
        total_db = 0.0
        for f0, gain in zip(cls.ISO_BANDS, gains_db):
            if abs(gain) > 1e-6:
                b0, b1, b2, a1, a2 = cls.compute_coefficients(f0, gain, fs=fs, q=q)
                total_db += cls.evaluate_response_db(b0, b1, b2, a1, a2, eval_freq, fs=fs)
        return total_db

    @classmethod
    def generate_ffmpeg_filter_string(cls, gains_db: Sequence[float]) -> str:
        """Generates FFmpeg audio filter string representing the 10-band equalizer."""
        if len(gains_db) != 10:
            raise ValueError(f"Expected 10 band gains, got {len(gains_db)}")
        filters = []
        for f0, gain in zip(cls.ISO_BANDS, gains_db):
            clamped_gain = cls.clamp_gain(gain)
            filters.append(f"equalizer=f={f0}:t=q:w=1.4142:g={clamped_gain:.1f}")
        return ",".join(filters)

    @classmethod
    def generate_mpv_af_string(
        cls,
        gains_db: Sequence[float],
        night_mode: bool = False,
        extra_filters: Optional[List[str]] = None,
    ) -> str:
        """
        Generates full MPV `af` filter string combining 10-band EQ, night mode, and extras.
        """
        filter_list: List[str] = []

        # Check if EQ is non-flat
        is_flat = all(abs(g) < 1e-3 for g in gains_db)
        if not is_flat:
            eq_str = cls.generate_ffmpeg_filter_string(gains_db)
            if eq_str:
                filter_list.append(f"lavfi=[{eq_str}]")

        # Night mode dynamic dialogue normalizer
        if night_mode:
            filter_list.append("lavfi=[dynaudnorm=f=150:g=15:m=10:p=0.95:r=0.9]")

        if extra_filters:
            filter_list.extend(extra_filters)

        return ",".join(filter_list)

    @classmethod
    def process_buffer(
        cls,
        samples: Sequence[float],
        gains_db: Sequence[float],
        fs: float = 44100.0,
    ) -> List[float]:
        """
        Applies Direct Form II biquad filtering in cascade over audio samples.
        """
        out_samples = list(samples)
        for f0, gain in zip(cls.ISO_BANDS, gains_db):
            if abs(gain) < 1e-6:
                continue
            b0, b1, b2, a1, a2 = cls.compute_coefficients(f0, gain, fs=fs)
            # Direct Form II Transposed filtering state: (d1, d2)
            d1 = 0.0
            d2 = 0.0
            for i, x in enumerate(out_samples):
                y = b0 * x + d1
                d1 = b1 * x - a1 * y + d2
                d2 = b2 * x - a2 * y
                out_samples[i] = y
        return out_samples


class VUMeterDSP:
    """Stereo audio VU and Peak level meter DSP calculation engine."""

    MIN_DB: float = -60.0
    MAX_DB: float = 3.0

    @classmethod
    def calculate_peak_db(cls, samples: Sequence[float]) -> float:
        """Calculates peak level in dBFS."""
        if not samples:
            return cls.MIN_DB
        peak = max(abs(s) for s in samples)
        if peak <= 1e-6:
            return cls.MIN_DB
        db = 20.0 * math.log10(peak)
        return max(cls.MIN_DB, min(cls.MAX_DB, db))

    @classmethod
    def calculate_rms_db(cls, samples: Sequence[float]) -> float:
        """Calculates RMS level in dBFS."""
        if not samples:
            return cls.MIN_DB
        sum_sq = sum(s * s for s in samples)
        mean_sq = sum_sq / len(samples)
        rms = math.sqrt(mean_sq)
        if rms <= 1e-6:
            return cls.MIN_DB
        db = 20.0 * math.log10(rms)
        return max(cls.MIN_DB, min(cls.MAX_DB, db))

    @classmethod
    def classify_zone(cls, db_val: float) -> str:
        """Classifies level into nominal, headroom, or clipping zones."""
        if db_val > 0.0:
            return "clipping"
        elif db_val >= -3.0:
            return "headroom"
        else:
            return "nominal"

    @classmethod
    def simulate_decay(
        cls,
        current_hold_db: float,
        delta_time_sec: float,
        decay_rate_db_per_sec: float = 20.0,
    ) -> float:
        """Simulates peak hold meter decay over time."""
        decayed = current_hold_db - (decay_rate_db_per_sec * delta_time_sec)
        return max(cls.MIN_DB, decayed)

    @classmethod
    def db_to_normalized(cls, db_val: float, min_db: float = -60.0, max_db: float = 0.0) -> float:
        """Converts dBFS level to normalized [0.0, 1.0] scale."""
        if db_val <= min_db:
            return 0.0
        if db_val >= max_db:
            return 1.0
        return (db_val - min_db) / (max_db - min_db)

    @classmethod
    def normalized_to_db(cls, norm_val: float, min_db: float = -60.0, max_db: float = 0.0) -> float:
        """Converts normalized [0.0, 1.0] level to dBFS."""
        clamped = max(0.0, min(1.0, norm_val))
        return min_db + clamped * (max_db - min_db)


# Aliases for clean importing
EqualizerDSP = BiquadPeakingEQ
