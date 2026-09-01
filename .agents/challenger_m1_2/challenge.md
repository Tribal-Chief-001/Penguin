# Empirical Challenge Report — Milestone 1 (DSP & Subtitle Stress Verification)

**Challenger**: Challenger 2 (Empirical Challenger)  
**Target Milestone**: Milestone 1 (Core Playback Engine, Audio DSP, Timecode & Subtitles)  
**Date**: 2026-08-31  
**Overall Verdict**: **APPROVE**

---

## Challenge Summary

**Overall risk assessment**: **LOW**

An extensive empirical test suite (`tests/test_challenger_m1_2.cpp`, `tests/test_m1_core.cpp`, `tests/test_equalizer_dsp.py`, `tests/test_e2e_runner.py`) was constructed and executed to stress-test the mathematical transfer functions of `EqualizerDSP`, the ballistics and decay physics of `VUMeterDSP`, and the parser resilience of `SubtitleLoader` against malformed and extreme inputs.

All 24 empirical test cases in `test_challenger_m1_2`, all 34 unit tests in `test_m1_core`, and all 327 regression test cases in `test_e2e_runner.py` passed with **0 errors**.

---

## Challenges & Empirical Findings

### [Low] Challenge 1: Sample Rate Coefficient Decoupling in `EqualizerDSP::calculateMagnitudeResponse`
- **Assumption challenged**: Calling `calculateMagnitudeResponse(freqHz, sampleRate)` evaluates the magnitude response for arbitrary `sampleRate` dynamically on an arbitrary `EqualizerDSP` instance.
- **Attack scenario**: If `calculateMagnitudeResponse(250.0, 44100.0)` is called on an instance whose internal biquad coefficients were computed for 48,000 Hz, the digital angular frequency $\omega = 2\pi f / f_s$ is evaluated against 48 kHz coefficients, resulting in a slight frequency shift (e.g. 5.65 dB instead of 6.0 dB at 250 Hz).
- **Blast radius**: Low. In runtime audio playback, `processSample()` and `processInterleavedBuffer()` dynamically synchronize `m_lastSampleRate` and call `updateBiquadCoeffs(sampleRate)` whenever the sample rate changes. UI visualizers calling `calculateMagnitudeResponse` operate at the engine's active sample rate (typically 44.1kHz or 48kHz).
- **Mitigation**: When evaluating magnitude response across varying sample rates in tests or visualizers, ensure `processSample(0.0f, 0, sampleRate)` is called or provide an explicit `setSampleRate(double sampleRate)` method.

### [Low] Challenge 2: SRT 2-Part Timestamp Format Matching
- **Assumption challenged**: `SubtitleLoader::parseSrt` handles non-standard 2-part timestamps (`MM:SS,mmm`) without an hour field.
- **Attack scenario**: `SubtitleLoader::parseSrtTimestamp` supports 2-part splitting (`MM:SS.mmm`), but the regex `timeArrowRegex` in `parseSrt` (`(\\d{1,2}:\\d{2}:\\d{2}[,\\.]\\d{2,3})`) strictly requires 3 parts (`HH:MM:SS`). An SRT file using `01:20,000 --> 01:25,000` (missing `00:` hours) will not match `timeArrowRegex`.
- **Blast radius**: Low. Standard SubRip (.srt) specification mandates `HH:MM:SS,mmm`. WebVTT files that use 2-part timestamps are correctly matched by `parseVtt`'s regex `(?:(\\d{1,2}:)?\\d{2}:\\d{2}[\\.]\\d{2,3})`.
- **Mitigation**: Update `parseSrt` regex to `(?:(\\d{1,2}:)?\\d{2}:\\d{2}[,\\.]\\d{2,3})` if tolerance for non-standard 2-part SRT files is desired in future iterations.

---

## Stress Test Results

| Test ID | Scenario | Expected Behavior | Actual Behavior | Result |
|---|---|---|---|---|
| **EQ-01** | ISO Center Frequencies (31.25Hz–16kHz) at $\pm12\text{dB}$, $\pm6\text{dB}$, $0\text{dB}$ | $\|H(e^{j\omega_0})\| == \text{gain} \pm 0.05\text{dB}$ | Max delta $< 0.002\text{dB}$ across all 10 bands | **PASS** |
| **EQ-02** | Direct Biquad Coeffs at Extreme Gains ($+24\text{dB}$, $-24\text{dB}$, $+18\text{dB}$) | Finite coefficients, exact $\|H(z)\| = \pm 24\text{dB}$ | Numerically stable, exact magnitude response | **PASS** |
| **EQ-03** | Pole Locations & BIBO Stability across $[-30\text{dB}, +30\text{dB}]$ | All pole magnitudes $\|\rho\| < 1.0$ | All poles strictly inside unit circle ($\|\rho\| < 0.999$) | **PASS** |
| **EQ-04** | Time-Domain Steady State Sine Wave (1kHz, $+6\text{dB}$) | RMS amplification matches $+6.0\text{dB} \pm 0.1\text{dB}$ | Measured steady state gain $= 6.00\text{dB}$ | **PASS** |
| **EQ-05** | Cascaded 10-Band Long Stream (500k samples Dirac + Noise at max gain) | Zero NaN/Inf/denormal explosion | All 500k output samples finite and stable | **PASS** |
| **EQ-06** | Asymptotic DC ($0\text{Hz}$) & Nyquist ($f_s/2$) Response | $\|H(1)\| \approx 0\text{dB}$, $\|H(-1)\| \approx 0\text{dB}$ | DC gain $= 0.000\text{dB}$, Nyquist gain $= 0.000\text{dB}$ | **PASS** |
| **EQ-07** | Sample Rate Variations ($44.1\text{kHz}, 48\text{kHz}, 88.2\text{kHz}, 96\text{kHz}, 192\text{kHz}$) | Correct frequency response at band center | Accurate $6.00\text{dB}$ response across all sample rates | **PASS** |
| **VU-01** | Full-scale & Half-scale Pure Sine Waves ($1\text{kHz}$) | Peak $= 0.0\text{dBFS}$, RMS $= -3.01\text{dBFS}$; Peak $= -6.02\text{dBFS}$, RMS $= -9.03\text{dBFS}$ | Left Peak $= 0.00\text{dBFS}$, Left RMS $= -3.01\text{dBFS}$; Right Peak $= -6.02\text{dBFS}$, Right RMS $= -9.03\text{dBFS}$ | **PASS** |
| **VU-02** | Complete Silence Buffer ($0.0$) Underflow | Clamps to $-60\text{dBFS}$, normalized $= 0.0$, no clipping | All channels $-60.0\text{dBFS}$, norm $= 0.0$, no NaN/Inf | **PASS** |
| **VU-03** | Logarithmic Ballistic Decay ($20\text{dB/s}$) | $-2.0\text{dB}$ at $0.1\text{s}$, $-10.0\text{dB}$ at $0.5\text{s}$, $-60.0\text{dB}$ at $2.5\text{s}$ | Matches exact linear-in-dB decay formula | **PASS** |
| **VU-04** | Peak Hold Window ($1.0\text{s}$) & Hold Decay ($30\text{dB/s}$) | Hold at $0.0\text{dB}$ for $\le 1.0\text{s}$, $-9.0\text{dB}$ at $1.3\text{s}$ | Exact hold time and decay slope verified | **PASS** |
| **VU-05** | Clipping Flag & Hold Timer ($1.5\text{s}$) | Flag `true` for $\ge 0\text{dBFS}$, resets after $1.5\text{s}$ | Clipping triggers at $+2.0\text{dBFS}$, clears at $t = 1.6\text{s}$ | **PASS** |
| **VU-06** | 16-bit PCM Integer Buffer Normalization | Full scale $32767 \to 0\text{dBFS}$, half scale $16384 \to -6.02\text{dBFS}$ | Measured $0.00\text{dBFS}$ and $-6.02\text{dBFS}$ | **PASS** |
| **VU-07** | High-amplitude Noise Burst Stress ($2.0$ over-scale) | Output bounded, no NaN/Inf, normalized $\in [0, 1]$ | 1,000 iterations completed safely | **PASS** |
| **SUB-01** | Malformed SRT Timestamps (non-digits, bad arrow) | Corrupted cues dropped, valid cues preserved | 2 valid cues parsed, malformed dropped cleanly | **PASS** |
| **SUB-02** | Zero-Length & Inverted Time Cues in SRT | Cues with $end \le start$ discarded | Dropped zero-length and inverted cues safely | **PASS** |
| **SUB-03** | Unclosed HTML & ASS Tags in SRT (`<b><i>`, `{\b1...}`) | Formatting stripped, plain text extracted | Stripped tags correctly, clean text extracted | **PASS** |
| **SUB-04** | WebVTT with Cue Settings (`line:80% align:center`) | Cue settings ignored, text and timestamps parsed | Parsed $2000\text{ms} \to 5000\text{ms}$ accurately | **PASS** |
| **SUB-05** | ASS Malformed Events, Tags & Hard Linebreaks (`\N`, `\n`) | Tags stripped, linebreaks converted to `\n` | Extracted dialogue cleanly with correct line breaks | **PASS** |
| **SUB-06** | ASS Dialogue with Commas in Text Field | Commas in text preserved without field truncation | Entire dialogue string preserved with commas intact | **PASS** |
| **SUB-07** | Extreme Large File Benchmark ($5,000$ cues) | Rapid parsing, chronological sorting, zero memory leaks | 5,000 cues parsed and sorted in $< 15\text{ms}$ | **PASS** |
| **SUB-08** | Active Subtitle Lookup with Overlapping Cues | `activeCues(pos)` returns all concurrent subtitles | Accurately returns 1, 2, or 0 cues at target offsets | **PASS** |

---

## Unchallenged Areas

- **Hardware GPU GLX / VAAPI Video Decoding**: Headless container environment tests offscreen mpv software rendering; dedicated hardware accelerated VAAPI/VDPAU surface decoding requires physical GPU context and display server.
- **Hardware Audio Output (PipeWire / ALSA DAC)**: Verified through software buffer processing, DSP math, and headless mpv audio null sink; physical DAC output routing will be verified on target desktop system.

---

## Final Assessment

The Milestone 1 core implementation (`EqualizerDSP`, `VUMeterDSP`, `SubtitleLoader`, `LrcParser`, `TimecodeFormatter`, `MpvBackend`, `PlaybackEngine`) satisfies all technical requirements, passes all mathematical and empirical verification criteria, and demonstrates high robustness against corrupted inputs and extreme parameter configurations.

**Verdict: APPROVE**
