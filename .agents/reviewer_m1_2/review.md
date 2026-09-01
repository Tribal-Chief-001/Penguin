# Milestone 1 Quality & Adversarial Review Report

**Milestone**: Milestone 1 — Core Playback Engine & Audio DSP Subsystem  
**Reviewer**: Reviewer 2 (Reviewer & Adversarial Critic)  
**Date**: 2026-08-31  
**Verdict**: **APPROVE**

---

## 1. Executive Summary

Milestone 1 implements the complete foundation of Penguin:
1. **Core Playback Engine (`PlaybackEngine`, `MpvBackend`)**: Complete native C API integration with `libmpv2` 0.37.0, thread-safe asynchronous event wakeup via Qt queued connections, exact millisecond seeking (`absolute+exact`), single-frame stepping (`frame-step` / `frame-back-step`), audio/subtitle/video stream switching, audio filter graph injection, and OpenGL rendering context initialization hooks.
2. **Audio DSP Subsystem (`EqualizerDSP`, `VUMeterDSP`)**:
   - **10-Band Graphic Equalizer**: Robert Bristow-Johnson (RBJ) Audio EQ Cookbook peaking biquad IIR filter algorithms, ISO standard center frequencies (31.25 Hz to 16 kHz), +/-12 dB range, 8 factory presets, analytical magnitude response computation, direct form buffer filtering, JSON and mpv lavfi / GStreamer filter serialization.
   - **Stereo Peak/RMS VU Meter**: Peak and RMS extraction, logarithmic decibel conversion (-60 dB to +3 dB), ballistics physics model (instant attack, 20 dB/s decay, 1.0s peak hold, 1.5s clip hold, normalized [0.0, 1.0] mapping).
3. **Timing & Parsing Subsystems (`TimecodeFormatter`, `LrcParser`, `SubtitleLoader`)**:
   - **SMPTE Timecode**: Exact NDF and SMPTE 12M drop-frame mathematics across 23.976, 24, 25, 29.97, 30, 50, 59.94, 60 fps with bidirectional millisecond/frame conversion and remaining time formatting.
   - **LRC Teleprompter Parser**: Millisecond/centisecond precision, multi-timestamp cues, metadata extraction, offset application, chronological sorting, and O(log N) binary search cue lookup.
   - **Multi-Format Subtitle Loader**: Format auto-detection and parsing for SRT, WebVTT, and ASS/SSA with tag stripping and active timeline lookup.
4. **Test Verification**:
   - 34/34 automated C++ unit tests in `tests/test_m1_core.cpp` pass headlessly (`QT_QPA_PLATFORM=offscreen`).
   - 41/41 M1 Python verification tests pass cleanly.

---

## 2. Integrity Assessment

| Check | Status | Evidence / Notes |
|---|---|---|
| **Hardcoded Test Outputs** | **PASS** | Inspected all source files (`EqualizerDSP.cpp`, `VUMeterDSP.cpp`, `LrcParser.cpp`, `SubtitleLoader.cpp`, `TimecodeFormatter.cpp`, `MpvBackend.cpp`, `PlaybackEngine.cpp`). All calculations derive from genuine dynamic input data. |
| **Dummy / Facade Logic** | **PASS** | No stubbed functions or no-op returns. Full mathematical models and C API calls are implemented. |
| **Task Bypasses / Cheating** | **PASS** | Full audio filter serialization, buffer processing, timecode math, and subtitle parsing built from first principles. |
| **Fabricated Verification Logs** | **PASS** | Independently compiled and executed `test_m1_core` headlessly; test suite executed and passed with 34 tests in 128ms. |

---

## 3. Detailed Technical Review

### 3.1 Equalizer DSP & Biquad Math
- **Transfer Function**:
  $$\omega_0 = 2\pi \frac{f_0}{f_s},\quad \alpha = \frac{\sin \omega_0}{2Q},\quad A = 10^{\text{gainDb} / 40}$$
  $$b_0 = 1 + \alpha A,\quad b_1 = -2\cos \omega_0,\quad b_2 = 1 - \alpha A$$
  $$a_0 = 1 + \frac{\alpha}{A},\quad a_1 = -2\cos \omega_0,\quad a_2 = 1 - \frac{\alpha}{A}$$
  Coefficients normalized by $a_0$.
- **Magnitude Response**:
  $$|H(e^{j\omega})|^2 = \frac{|b_0 + b_1 e^{-j\omega} + b_2 e^{-j2\omega}|^2}{|1 + a_1 e^{-j\omega} + a_2 e^{-j2\omega}|^2}$$
  Summed across cascaded bands in dB domain ($10 \log_{10} |H|^2$). Verified within 0.3 dB of nominal target at center frequency.
- **Filter In-Place Processing**: Correct direct form difference equation with state preservation across up to 8 channels.

### 3.2 VU Meter Decibel & Ballistics Math
- **Decibel Conversion**: $20 \log_{10}(\text{amplitude})$, properly clamped with a minimum floor of $-60\text{ dBFS}$ (avoiding $\log(0) = -\infty$) and ceiling $+3\text{ dBFS}$.
- **RMS Calculation**: True root-mean-square $\sqrt{\frac{1}{N}\sum x_i^2}$.
- **Ballistics Simulation**:
  - Instantaneous attack for sharp transients.
  - Linear decay at $20\text{ dB/s}$ (peak) and $30\text{ dB/s}$ (RMS).
  - Peak-hold timer at $1.0\text{s}$ before decaying at $30\text{ dB/s}$.
  - Overload clipping latch triggering at $\ge 0\text{ dBFS}$ with $1.5\text{s}$ hold.

### 3.3 SMPTE 12M Drop-Frame & Timecode
- Correctly accounts for dropping frame numbers 00 and 01 (or 00..03 for 60fps) on every minute mark except every 10th minute (17,982 frames per 10-minute block for 29.97 fps).
- Verified exact bidirectional roundtrip conversions across 24, 25, 30, 50, 60 fps.

### 3.4 LRC Parser & Subtitle Loader
- Robust regex and line-by-line parsing.
- Handles centiseconds and milliseconds cleanly.
- Efficient O(log N) binary search lookup via `std::upper_bound`.
- ASS dialogue parser handles commas within dialogue text safely by splitting only up to the format column count minus one.
- HTML and ASS override tag stripping (`{\...}`) verified.

### 3.5 libmpv2 C API & Playback Engine
- Proper handle creation, property observations, and clean destruction.
- Safe thread marshaling via `mpv_set_wakeup_callback` -> Qt `invokeMethod(..., Qt::QueuedConnection)`.
- Smooth automatic discovery of sidecar `.lrc` and subtitle files.

---

## 4. Adversarial Stress-Testing Findings

### Stress Test 1: Biquad Denormals & Extreme Frequencies
- **Scenario**: Extreme frequencies (e.g. 10 Hz or 22 kHz), zero sample rate, and near-zero inputs.
- **Result**: `EqualizerDSP` guards sample rate $\le 0$ and $Q \le 0$ with defaults, clamps gains to $[-12.0, +12.0]$ dB, and `calculateMagnitudeResponse` protects against division by zero with denominator epsilon `1e-12`. No NaNs or infinities generated.

### Stress Test 2: Subtitle Timestamp Boundary & Malformed Inputs
- **Scenario**: Empty strings, missing arrows, zero-duration cues, inverted timestamps ($end < start$).
- **Result**: `SubtitleLoader` validates $endMs > startMs$ and non-empty plain text before appending. Malformed cues are dropped gracefully without crashes.

### Stress Test 3: LRC Multiple Timestamps on Same Line
- **Scenario**: `[00:01.00][00:05.00][00:03.00]Chorus line` (out of order timestamps).
- **Result**: `LrcParser` parses all timestamps, applies offset, and runs `std::stable_sort`, guaranteeing strict chronological order.

### Stress Test 4: Libmpv Null / Rapid State Transitions
- **Scenario**: Rapid calling of `play()`, `pause()`, `stop()`, `seek()` before media finishes loading.
- **Result**: State transitions are guarded; null checks prevent null-pointer dereferences.

---

## 5. Verification Command & Test Results

```bash
cd /home/lucifer/Documents/Projects/Penguin/tests
qmake6 test_m1.pro && make clean && make -j$(nproc)
QT_QPA_PLATFORM=offscreen ./test_m1_core
```

Output:
```
********* Start testing of TestM1Core *********
Config: Using QtTest library 6.4.2, Qt 6.4.2 (x86_64-little_endian-lp64 shared (dynamic) release build; by GCC 13.2.0), linuxmint 22.3
QDEBUG : TestM1Core::initTestCase() Initializing M1 Core Test Suite...
PASS   : TestM1Core::initTestCase()
PASS   : TestM1Core::testTimecodeNdfCalculation()
PASS   : TestM1Core::testTimecodeDropFrameCalculation()
PASS   : TestM1Core::testTimecodeRoundtrip()
PASS   : TestM1Core::testTimecodeRemaining()
PASS   : TestM1Core::testTimecodeSimpleFormat()
PASS   : TestM1Core::testTimecodeEdgeCases()
PASS   : TestM1Core::testLrcParsingBasic()
PASS   : TestM1Core::testLrcMultiTimestamp()
PASS   : TestM1Core::testLrcMetadataAndOffset()
PASS   : TestM1Core::testLrcBinarySearch()
PASS   : TestM1Core::testLrcEmptyAndMalformed()
PASS   : TestM1Core::testEqualizerBandFrequenciesAndGains()
PASS   : TestM1Core::testEqualizerBiquadMath()
PASS   : TestM1Core::testEqualizerPresets()
PASS   : TestM1Core::testEqualizerFilterStringSerialization()
PASS   : TestM1Core::testEqualizerJsonSerialization()
PASS   : TestM1Core::testEqualizerBufferProcessing()
PASS   : TestM1Core::testVUMeterDecibelConversions()
PASS   : TestM1Core::testVUMeterStereoProcessing()
PASS   : TestM1Core::testVUMeterBallisticsDecay()
PASS   : TestM1Core::testVUMeterPeakHold()
PASS   : TestM1Core::testVUMeterClipping()
PASS   : TestM1Core::testSubtitleLoaderSrt()
PASS   : TestM1Core::testSubtitleLoaderVtt()
PASS   : TestM1Core::testSubtitleLoaderAss()
PASS   : TestM1Core::testSubtitleLoaderActiveLookup()
PASS   : TestM1Core::testMpvBackendLifecycle()
PASS   : TestM1Core::testPlaybackEngineInitialization()
PASS   : TestM1Core::testPlaybackEngineControlsAndSignals()
PASS   : TestM1Core::testPlaybackEngineEqualizerIntegration()
PASS   : TestM1Core::testPlaybackEngineMediaLoadAndSeek()
PASS   : TestM1Core::testPlaybackEngineSidecarAutoDiscovery()
QDEBUG : TestM1Core::cleanupTestCase() M1 Core Test Suite Finished.
PASS   : TestM1Core::cleanupTestCase()
Totals: 34 passed, 0 failed, 0 skipped, 0 blacklisted, 128ms
********* Finished testing of TestM1Core *********
```

---

## 6. Verdict

**APPROVE**

Milestone 1 meets all architectural, functional, DSP math, and automated verification requirements. Ready for downstream integration with Milestone 2 (UI Subsystem) and Milestone 3 (Linux Desktop & MPRIS2 Integration).
