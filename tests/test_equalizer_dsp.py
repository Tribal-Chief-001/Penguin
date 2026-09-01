"""
test_equalizer_dsp.py - Analytical Biquad Equalizer & VU Meter DSP Verification.

Verifies the 10-band peaking EQ IIR filter math (Audio EQ Cookbook by Robert Bristow-Johnson),
analytical transfer function evaluation |H(z)| across all 10 center frequencies,
preset gain configurations, flat reset, FFmpeg filter string generation, and stereo VU meter DSP.
"""

import cmath
import math
import unittest
from typing import Dict, List, Tuple


class BiquadPeakingEQ:
    """Calculates second-order Biquad Peaking EQ coefficients and frequency response."""

    ISO_BANDS = [32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000]

    PRESETS = {
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
    def compute_coefficients(
        cls,
        f0: float,
        gain_db: float,
        fs: float = 44100.0,
        q: float = 1.4142,
    ) -> Tuple[float, float, float, float, float]:
        """
        Computes normalized Biquad coefficients (B0, B1, B2, A1, A2) for peaking EQ.
        Returns: (B0, B1, B2, A1, A2) where a0 is normalized to 1.0.
        """
        gain_db = max(-12.0, min(12.0, gain_db))
        if abs(gain_db) < 1e-6:
            # Flat response: H(z) = 1
            return 1.0, 0.0, 0.0, 0.0, 0.0

        a_gain = 10.0 ** (gain_db / 40.0)
        omega0 = 2.0 * math.pi * f0 / fs
        alpha = math.sin(omega0) / (2.0 * q)
        cos_omega0 = math.cos(omega0)

        b0 = 1.0 + alpha * a_gain
        b1 = -2.0 * cos_omega0
        b2 = 1.0 - alpha * a_gain
        a0 = 1.0 + alpha / a_gain
        a1 = -2.0 * cos_omega0
        a2 = 1.0 - alpha / a_gain

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
        """Evaluates magnitude response |H(e^{j 2pi f / fs})| in dB at eval_freq."""
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
    def generate_ffmpeg_filter_string(cls, gains_db: List[float]) -> str:
        """Generates FFmpeg audio filter string representing the 10-band equalizer."""
        if len(gains_db) != 10:
            raise ValueError(f"Expected 10 band gains, got {len(gains_db)}")
        filters = []
        for f0, gain in zip(cls.ISO_BANDS, gains_db):
            clamped_gain = max(-12.0, min(12.0, gain))
            filters.append(f"equalizer=f={f0}:t=q:w=1.4142:g={clamped_gain:.1f}")
        return ",".join(filters)


class VUMeterDSP:
    """Stereo audio VU and Peak level meter DSP calculation engine."""

    MIN_DB = -60.0
    MAX_DB = 3.0

    @classmethod
    def calculate_peak_db(cls, samples: List[float]) -> float:
        """Calculates peak level in dBFS."""
        if not samples:
            return cls.MIN_DB
        peak = max(abs(s) for s in samples)
        if peak <= 1e-6:
            return cls.MIN_DB
        db = 20.0 * math.log10(peak)
        return max(cls.MIN_DB, min(cls.MAX_DB, db))

    @classmethod
    def calculate_rms_db(cls, samples: List[float]) -> float:
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
    def simulate_decay(cls, current_hold_db: float, delta_time_sec: float, decay_rate_db_per_sec: float = 20.0) -> float:
        """Simulates peak hold meter decay over time."""
        decayed = current_hold_db - (decay_rate_db_per_sec * delta_time_sec)
        return max(cls.MIN_DB, decayed)


class TestEqualizerDSP(unittest.TestCase):
    """Test suite verifying Biquad transfer function analytical properties across all bands and presets."""

    def test_iso_10_bands_center_frequency_response(self):
        """Analytical verification that |H(f0)| == gain_db within +-0.001 dB."""
        fs = 48000.0
        test_gains = [-12.0, -6.0, -3.0, 0.0, 3.0, 6.0, 12.0]

        for f0 in BiquadPeakingEQ.ISO_BANDS:
            # Skip bands approaching Nyquist for 48kHz (16kHz is fine since 16k < 24k)
            if f0 >= fs / 2:
                continue
            for gain in test_gains:
                b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(f0, gain, fs=fs)
                measured_gain = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, f0, fs=fs)
                self.assertAlmostEqual(
                    measured_gain,
                    gain,
                    delta=0.001,
                    msg=f"Center frequency response failed for band {f0}Hz with target {gain}dB (got {measured_gain:.4f}dB)",
                )

    def test_asymptotic_dc_and_nyquist_flat_response(self):
        """Peaking EQ response at DC (0Hz) and Nyquist (fs/2) must be 0dB (flat)."""
        fs = 48000.0
        for f0 in [250, 500, 1000, 2000, 4000]:
            b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(f0, gain_db=6.0, fs=fs)
            gain_dc = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, eval_freq=0.0, fs=fs)
            gain_nyq = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, eval_freq=fs / 2, fs=fs)
            self.assertAlmostEqual(gain_dc, 0.0, delta=0.001)
            self.assertAlmostEqual(gain_nyq, 0.0, delta=0.001)

    def test_all_factory_presets_structure(self):
        self.assertIn("Flat", BiquadPeakingEQ.PRESETS)
        self.assertIn("Rock", BiquadPeakingEQ.PRESETS)
        self.assertIn("Bass Boost", BiquadPeakingEQ.PRESETS)
        self.assertIn("Vocal Boost", BiquadPeakingEQ.PRESETS)
        self.assertIn("Studio Mastering", BiquadPeakingEQ.PRESETS)

        for name, gains in BiquadPeakingEQ.PRESETS.items():
            self.assertEqual(len(gains), 10, f"Preset '{name}' must have exactly 10 band gains")
            for g in gains:
                self.assertGreaterEqual(g, -12.0)
                self.assertLessEqual(g, 12.0)

    def test_flat_reset_response(self):
        flat_gains = [0.0] * 10
        for f0 in BiquadPeakingEQ.ISO_BANDS:
            b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(f0, 0.0)
            self.assertEqual((b0, b1, b2, a1, a2), (1.0, 0.0, 0.0, 0.0, 0.0))
            gain = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, f0)
            self.assertEqual(gain, 0.0)

    def test_ffmpeg_filter_string_generation(self):
        rock_gains = BiquadPeakingEQ.PRESETS["Rock"]
        filter_str = BiquadPeakingEQ.generate_ffmpeg_filter_string(rock_gains)
        self.assertTrue(filter_str.startswith("equalizer=f=32:t=q:w=1.4142:g=4.5"))
        self.assertIn("equalizer=f=1000:t=q:w=1.4142:g=-0.5", filter_str)
        self.assertTrue(filter_str.endswith("equalizer=f=16000:t=q:w=1.4142:g=4.5"))

    def test_gain_clamping(self):
        # Value > 12 clamped to 12
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, gain_db=20.0)
        gain_measured = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, 1000)
        self.assertAlmostEqual(gain_measured, 12.0, delta=0.001)

        # Value < -12 clamped to -12
        b0, b1, b2, a1, a2 = BiquadPeakingEQ.compute_coefficients(1000, gain_db=-30.0)
        gain_measured = BiquadPeakingEQ.evaluate_response_db(b0, b1, b2, a1, a2, 1000)
        self.assertAlmostEqual(gain_measured, -12.0, delta=0.001)

    def test_vu_meter_dsp_full_scale_sine(self):
        # 1.0 peak sine wave
        n_samples = 1000
        samples = [math.sin(2.0 * math.pi * i / 100) for i in range(n_samples)]
        peak_db = VUMeterDSP.calculate_peak_db(samples)
        rms_db = VUMeterDSP.calculate_rms_db(samples)

        # Peak of unit sine is 1.0 -> 0.0 dBFS
        self.assertAlmostEqual(peak_db, 0.0, delta=0.01)
        # RMS of unit sine is 1/sqrt(2) = 0.7071 -> -3.01 dBFS
        self.assertAlmostEqual(rms_db, -3.0103, delta=0.05)

    def test_vu_meter_silence(self):
        silence = [0.0] * 500
        self.assertEqual(VUMeterDSP.calculate_peak_db(silence), -60.0)
        self.assertEqual(VUMeterDSP.calculate_rms_db(silence), -60.0)

    def test_vu_meter_zones_and_decay(self):
        self.assertEqual(VUMeterDSP.classify_zone(-10.0), "nominal")
        self.assertEqual(VUMeterDSP.classify_zone(-1.5), "headroom")
        self.assertEqual(VUMeterDSP.classify_zone(1.2), "clipping")

        # Peak decay over 0.5s at 20dB/s -> drops by 10dB
        decayed = VUMeterDSP.simulate_decay(0.0, delta_time_sec=0.5, decay_rate_db_per_sec=20.0)
        self.assertAlmostEqual(decayed, -10.0, delta=0.01)


if __name__ == "__main__":
    unittest.main()
