# Quality Review & Adversarial Challenge Report: Milestone 1

**Reviewer**: Reviewer 1 (reviewer, critic)  
**Milestone**: Milestone 1 — Core Playback Engine & Audio DSP  
**Target Path**: `src/core/`, `tests/test_m1_core.cpp`  
**Date**: 2026-08-31  

---

## 1. Review Summary

**Verdict**: **APPROVE**

Milestone 1 successfully delivers the entire foundation of the Penguin media player core architecture. All components specified in `ORIGINAL_REQUEST.md` (§R1) and `PROJECT.md` have been implemented with genuine, rigorous algorithms, robust memory safety, and thread-safe Qt/libmpv integration.

---

## 2. Review Dimensions

### 2.1 Integrity Assessment
- **Hardcoded test returns**: **None**. All mathematical models, parsing engines, and audio processing algorithms execute full logic.
- **Facade implementations**: **None**. `PlaybackEngine` directly manages underlying `MpvBackend`, `EqualizerDSP`, `VUMeterDSP`, `SubtitleLoader`, and `LrcParser` instances.
- **Shortcuts & external delegation**: **None**. Peaking Biquad filters, SMPTE timecodes, VU ballistics, and subtitle parsers are natively implemented in C++17.
- **Attestation**: Test results independently reproduced and verified headlessly (`QT_QPA_PLATFORM=offscreen`).

### 2.2 Correctness & Algorithmic Rigor
1. **TimecodeFormatter**:
   - Implements both Non-Drop Frame (NDF) and SMPTE 12M Drop-Frame (DF) calculations for 23.976, 24, 25, 29.97, 30, 50, 59.94, and 60 fps.
   - Accurately handles 10-minute non-dropping boundaries and fractional seconds.
   - Full bi-directional roundtrip (`formatTimecode` <-> `timecodeToMs`) is exact within $\le 1$ frame duration across all standard framerates.
2. **LrcParser**:
   - Parses standard `[mm:ss.xx]`, `[mm:ss.xxx]`, multi-timestamp cues (`[00:01.00][00:10.00]Lyric`), and metadata tags (`[ti:]`, `[ar:]`, `[al:]`, `[by:]`, `[offset:]`).
   - Applies global millisecond offsets and maintains chronological ordering via `std::stable_sort`.
   - Employs $O(\log N)$ binary search (`std::upper_bound`) for real-time cue lookup during playback.
3. **EqualizerDSP**:
   - Implements 10 ISO standard center frequencies (31.25 Hz to 16.0 kHz) with gain clamping $[-12.0\text{ dB}, +12.0\text{ dB}]$.
   - Calculates Robert Bristow-Johnson Audio EQ Cookbook Peaking Biquad IIR filter coefficients ($b_0, b_1, b_2, a_1, a_2$) with direct form difference equations.
   - Computes analytical frequency responses via complex transfer function evaluation.
   - Serializes to FFmpeg `lavfi=[equalizer=...]` filter graphs for transparent injection into `libmpv`.
4. **VUMeterDSP**:
   - Computes linear-to-dBFS transformations spanning $[-60.0\text{ dBFS}, +3.0\text{ dBFS}]$ and handles zero/sub-epsilon inputs without numerical singularity.
   - Integrates stereo channel peak and RMS power ($\sqrt{\frac{1}{N}\sum x_i^2}$).
   - Accurately models physical VU decay ballistics ($20\text{ dB/s}$), peak-hold timers ($1.0\text{s}$ hold with $30\text{ dB/s}$ decay), and clipping hold timers ($1.5\text{s}$).
5. **SubtitleLoader**:
   - Supports SRT, WebVTT, and ASS/SSA subtitle formats with automated header/extension detection.
   - Strips formatting tags (ASS override tags `{\...}`, HTML tags `<b>...</b>`) while preserving text and timestamps.
   - Implements active cue lookup supporting overlapping subtitle streams.
6. **MpvBackend & PlaybackEngine**:
   - Wraps `libmpv2` C API with RAII resource management.
   - Connects `mpv_set_wakeup_callback` to Qt's event loop via `QMetaObject::invokeMethod(backend, "processEvents", Qt::QueuedConnection)`, ensuring thread-safe main-thread event dispatching.
   - Discovers sidecar subtitle and lyrics files automatically during media loading.

### 2.3 Architecture & Memory Safety
- **Ownership & Lifecycles**: Strict RAII encapsulation using `std::unique_ptr` and Qt parent-child hierarchy.
- **Zero dynamic allocations in audio path**: Fixed-size buffers and `std::array` state arrays are pre-allocated for up to 8 audio channels.
- **Thread Safety**: `libmpv` background events are queued into Qt's event loop; GUI-thread state reads are race-free.

---

## 3. Adversarial Stress-Testing & Attack Surface Analysis

| # | Assumption / Scenario | Stress-Test / Attack Construction | Blast Radius | Result |
|---|---|---|---|---|
| 1 | SMPTE 12M Drop-Frame Minute Boundary | At $t = 59.97\text{s}$ at 29.97 fps, verify frame indices skip `00:01:00;00` and `00:01:00;01` to land on `00:01:00;02`, but do NOT skip at 10-minute marks. | Timecode desynchronization in video viewfinder | **PASS**: Exact match with SMPTE 12M standard. |
| 2 | Biquad IIR Peaking Filter Stability | Apply extreme gain ($\pm 12\text{ dB}$) across Nyquist boundary ($16\text{ kHz}$ at $44.1\text{ kHz}$ sample rate). Evaluate pole locations. | Filter oscillation or overflow (NaN/Inf in audio buffer) | **PASS**: Pole magnitudes strictly $< 1$ by Jury stability criterion. Processed samples verified finite. |
| 3 | Malformed / Corrupted Subtitles & LRC | Inject empty strings, missing timestamps, invalid tags, and multi-line breaks. | Application crash, unhandled exceptions, or hung loops | **PASS**: Handled gracefully without crash; returns clean fallback empty states. |
| 4 | Rapid Media Load / Unload / Stop | Rapid sequence of `loadMedia()`, `seek()`, `frameStep()`, and `stop()` calls on synthetic video container. | libmpv race condition, deadlock, or heap corruption | **PASS**: State transitions remain consistent; destructors cleanly release `mpv_handle`. |
| 5 | Audio VU Meter Sub-Epsilon Zero Level | Feed continuous array of `0.0f` floating point samples. | $\log_{10}(0)$ returning `-inf` or `NaN` polluting UI bindings | **PASS**: Clamped cleanly to minimum $-60.0\text{ dBFS}$ / $0.0$ normalized. |

---

## 4. Test Verification Results

The M1 automated test suite (`tests/test_m1_core`) was built and executed headlessly:

```
********* Start testing of TestM1Core *********
Config: Using QtTest library 6.4.2, Qt 6.4.2
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
PASS   : TestM1Core::cleanupTestCase()
Totals: 34 passed, 0 failed, 0 skipped, 0 blacklisted, 113ms
********* Finished testing of TestM1Core *********
```

---

## 5. Findings & Recommendations

### Minor Note 1: Linking in `test_m1.pro`
- When compiling `tests/test_m1.pro` with aggressive parallel jobs (`make -j$(nproc)`), building directly against individual `.o` files occasionally triggers a linker race before object files are flushed.
- **Recommendation**: Link tests against `libpenguin_core.a` (as configured in `CMakeLists.txt` and `penguin.pro`) in subsequent milestone test pro files.

### Minor Note 2: Fixture Sample Rate in `test_tiers.py`
- In `tests/fixtures/test_audio.flac`, the sample rate on disk is 48000 Hz, whereas `test_tiers.py:446` (M5 boundary test) checks for 96000 Hz.
- **Recommendation**: When executing M5 end-to-end verification, ensure synthetic media generation scripts generate 96kHz fixtures or dynamic temporary files.

---

## 6. Conclusion
Milestone 1 is **APPROVED**. The code is mathematically sound, cleanly modularized, memory safe, and fully ready for UI subsystem integration (Milestone 2) and system/desktop integration (Milestone 3).
