# Adversarial Challenge Report — Milestone 1 (Core Engine)

**Target**: Core Playback Engine, SMPTE Timecode Formatter, and Synchronized LRC Parser  
**Challenger**: Challenger 1 (Empirical Challenger / Critic Specialist)  
**Date**: 2026-08-31  
**Overall Risk Assessment**: **MEDIUM**  
**Verdict**: **REQUEST_CHANGES**

---

## Executive Summary
An extensive adversarial and empirical stress-test suite was executed against the Milestone 1 core components:
1. **SMPTE Timecode Conversions (`TimecodeFormatter`)**:
   - Tested non-drop frame conversions across standard and fractional frame rates (23.976, 24.0, 25.0, 29.97 NDF, 30.0, 50.0, 59.94 NDF, 60.0).
   - Executed continuous 1-hour full frame scans (108,000 frames for 29.97 DF; 216,000 frames for 59.94 DF) to verify SMPTE 12M drop-frame mathematical invariants.
   - Tested extreme values (up to 10,000+ hours and 10^12 ms), negative milliseconds, stepping forward/backward (+/- 1F), and relative seeking jumps.
   - **Result**: SMPTE 12M drop-frame math, nominal FPS calculation, bidirectional roundtrips, and stepping math are mathematically sound and robust. One minor non-blocking edge case noted in `isValidTimecode` for > 99 hours.

2. **Synchronized Lyric Parser (`LrcParser`)**:
   - Tested formatting variants, multi-timestamp lines, out-of-order lines, metadata ID tags, global offset arithmetic, binary search lookup over 100,000 randomized queries, and malformed/fuzzed inputs (including Unicode, emojis, RTL text, unclosed tags).
   - **Result**: Identified **2 concrete bugs** causing silent data loss in `LrcParser.cpp`:
     - **Bug 1 (High Severity)**: Timestamp regex `\\[(\\d{1,2}:\\d{2}[\\.:]\\d{2,3})\\]` restricts minutes to 1-2 digits (`\\d{1,2}`). Any LRC cues at or beyond 100 minutes (`>= 100:00.00`) are rejected and silently dropped (e.g. in a 10,000-cue benchmark spanning 166 minutes, 4,000 cues were dropped).
     - **Bug 2 (Medium Severity)**: Regex fraction specification `[\\.:]\\d{2,3}` enforces at least 2 fractional digits, causing single-digit decisecond timestamps (e.g. `[00:03.4]`) and integer-second timestamps (`[00:03]`) to be silently discarded, even though `parseTimestamp` has handling logic for 1-digit fractions.

---

## Detailed Challenges & Vulnerabilities

### [High] Challenge 1: `LrcParser` Drops All Timestamps >= 100 Minutes (Minute Overflow in Regex)
- **Assumption Challenged**: Media duration with lyrics/subtitles will never exceed 99 minutes (5940 seconds).
- **Attack Scenario**: An audio file, podcast, concert recording, or movie with duration > 100 minutes has synchronized cues formatted as `[100:15.00]`, `[120:30.00]`, etc.
- **Empirical Observation**:
  - `src/core/LrcParser.cpp` line 85:
    ```cpp
    static const QRegularExpression timeRegex("\\[(\\d{1,2}:\\d{2}[\\.:]\\d{2,3})\\]");
    ```
  - When parsing a 10,000-line dataset (`testLrcLargeFilePerformance`), exactly 4,000 cues between minute 100 and minute 166 failed the regex match and were dropped (`parser.count()` returned 6,000 instead of 10,000).
- **Blast Radius**: Complete loss of teleprompter/lyrics synchronization for long audio tracks, concerts, audiobooks, and feature films.
- **Mitigation**: Update `timeRegex` in `src/core/LrcParser.cpp` to support 1 to 4 minute digits:
  ```cpp
  static const QRegularExpression timeRegex("\\[(\\d{1,4}:\\d{2}(?:[\\.:]\\d{1,3})?)\\]");
  ```

---

### [Medium] Challenge 2: `LrcParser` Rejects Valid 1-Digit Fractional and Integer-Second Timestamps
- **Assumption Challenged**: All LRC generators output exactly 2 or 3 fractional digits.
- **Attack Scenario**: An LRC file contains decisecond timestamps (e.g. `[00:01.5]`) or standard integer-second timestamps (e.g. `[00:05]`).
- **Empirical Observation**:
  - `timeRegex` in `LrcParser.cpp` requires `[\\.:]\\d{2,3}`.
  - Single-digit fraction `[00:03.4]` in `testLrcTimestampFormatVariants` failed to match `timeRegex`, despite `parseTimestamp` explicitly containing `else if (fracStr.length() == 1) ms = fracStr.toLongLong() * 100;`.
  - Integer-second timestamp `[00:05]` without fraction also fails.
- **Blast Radius**: Valid LRC files created by tools emitting 1-digit decimals or whole seconds fail to parse.
- **Mitigation**: Make the decimal part optional with 1 to 3 digits: `(?:\.[0-9]{1,3}|:[0-9]{1,3})?` or `(?:[\\.:]\\d{1,3})?`.

---

### [Low] Challenge 3: `TimecodeFormatter::isValidTimecode` Rejects Durations >= 100 Hours
- **Assumption Challenged**: Timecode hours never exceed 99.
- **Attack Scenario**: Long stream/playback position formatted as `100:00:00:00` is validated via `isValidTimecode("100:00:00:00")`.
- **Empirical Observation**:
  - `TimecodeFormatter.cpp` line 175:
    ```cpp
    static const QRegularExpression regex("^-?\\d{1,2}:\\d{2}:\\d{2}[:;]\\d{2}$");
    ```
  - Returns `false` for `100:00:00:00` even though `formatTimecode` outputs 3+ digits for hours when `positionMs >= 360000000` and `timecodeToMs` successfully parses it.
- **Blast Radius**: False negative validation for extreme continuous streams.
- **Mitigation**: Update regex to `^-?\\d{1,}:\\d{2}:\\d{2}[:;]\\d{2}$`.

---

## Empirical Stress Test Results

| Category | Test Scenario | Expected Behavior | Actual Behavior | Result |
|---|---|---|---|---|
| **SMPTE Rate** | Fractional 23.976, 29.97 NDF, 59.94 NDF | Correct nominal FPS & roundtrip accuracy | Nominal FPS 24/30/60, error <= 1 frame | **PASS** |
| **SMPTE 12M** | 29.97 DF 108,000 frame scan (1 hr) | Monotonic progression, skip ;00/;01 on non-10m | 0 invalid frames, 100% compliant | **PASS** |
| **SMPTE 12M** | 59.94 DF 216,000 frame scan (1 hr) | Monotonic progression, skip ;00-;03 on non-10m | 0 invalid frames, 100% compliant | **PASS** |
| **SMPTE 12M** | 10-Minute Boundary Check | Minute 10 retains frames ;00 and ;01 | Exact match: 00:10:00;00 / 00:10:00;01 | **PASS** |
| **SMPTE Extremes**| 24h, 100h, 1000h, 10,000h, 10^12 ms | Correct timecode string and parsing | Exact hour formatting without overflow | **PASS** |
| **SMPTE Negative**| -1s, -1h, -24h negative ms | Leading negative sign, exact roundtrip | `-00:00:01:00`, `-01:00:00:00`, `-24:00:00:00` | **PASS** |
| **SMPTE Stepping**| Forward (+1F) / Backward (-1F) | Step by frame duration, clamp at 0 & dur | Accurate step delta, 0 drift | **PASS** |
| **SMPTE Stepping**| Step across 29.97 DF drop points | Step from 00:00:59;29 to 00:01:00;02 | Continuous step without under/overflow | **PASS** |
| **LRC Multi-TS**  | Multiple timestamps per line & OOO | All instances extracted and sorted | Sorted chronologically | **PASS** |
| **LRC Sorting**   | Out-of-order lines (reverse order) | Chronological sorting of all cues | Strict monotonic cue order | **PASS** |
| **LRC Offset**    | `[offset:+1500]` & `[offset:-2000]` | Shift timestamps, clamp negative to 0 | Accurate ms shift, clamped to 0 | **PASS** |
| **LRC Search**    | 100,000 random binary search queries | Correct active cue index at all points | 100,000 / 100,000 exact matches | **PASS** |
| **LRC Fuzzing**   | Malformed brackets, garbage, Unicode | No crash, graceful cue extraction | Handled safely, 0 memory faults | **PASS** |
| **LRC Timestamp** | Deciseconds `[00:03.4]` & `[120:30.00]` | Both timestamps parsed | Regex rejected both | **FAIL** |
| **LRC Scale**     | 10,000 cues spanning 166 minutes | All 10,000 cues parsed | Only 6,000 parsed (4,000 dropped) | **FAIL** |

---

## Recommendations & Action Items for Worker
1. **Fix `timeRegex` in `src/core/LrcParser.cpp`**:
   Replace:
   ```cpp
   static const QRegularExpression timeRegex("\\[(\\d{1,2}:\\d{2}[\\.:]\\d{2,3})\\]");
   ```
   with:
   ```cpp
   static const QRegularExpression timeRegex("\\[(\\d{1,4}:\\d{2}(?:[\\.:]\\d{1,3})?)\\]");
   ```
2. **Update `isValidTimecode` in `src/core/TimecodeFormatter.cpp`**:
   Replace:
   ```cpp
   static const QRegularExpression regex("^-?\\d{1,2}:\\d{2}:\\d{2}[:;]\\d{2}$");
   ```
   with:
   ```cpp
   static const QRegularExpression regex("^-?\\d{1,}:\\d{2}:\\d{2}[:;]\\d{2}$");
   ```
3. Re-run `./tests/test_challenger_m1` to verify 100% pass across all 21 stress-test suites.
