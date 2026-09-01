# Handoff Report — Challenger 1 (Milestone 1 Core Engine Verification)

## 1. Observation
- **Binary Executable Tested**: `tests/test_challenger_m1` (built via `qmake6 tests/test_challenger_m1.pro && make -f Makefile.test_challenger`).
- **Execution Command & Verbatim Output**:
  ```
  ./tests/test_challenger_m1
  ```
  Output:
  ```
  ********* Start testing of TestChallengerM1 *********
  Config: Using QtTest library 6.4.2, Qt 6.4.2 (x86_64-little_endian-lp64 shared (dynamic) release build; by GCC 13.2.0), linuxmint 22.3
  QDEBUG : TestChallengerM1::initTestCase() === Starting Challenger 1 Empirical Stress Test Suite ===
  PASS   : TestChallengerM1::initTestCase()
  PASS   : TestChallengerM1::testFractionalFrameRatesNominalAndDropDetection()
  PASS   : TestChallengerM1::testDropFrameMath2997FullHourScan()
  PASS   : TestChallengerM1::testDropFrameMath5994FullHourScan()
  PASS   : TestChallengerM1::testDropFrame10MinuteBoundaries()
  PASS   : TestChallengerM1::testNonDropFrameFractionalRates()
  PASS   : TestChallengerM1::testExtremeMillisecondValues()
  PASS   : TestChallengerM1::testNegativeMillisecondValues()
  PASS   : TestChallengerM1::testTimecodeParserStressAndFuzz()
  PASS   : TestChallengerM1::testTimecodeRemainingEdgeCases()
  PASS   : TestChallengerM1::testFrameSteppingForwardBackwardMath()
  PASS   : TestChallengerM1::testFrameSteppingAcrossDropPoints()
  FAIL!  : TestChallengerM1::testLrcTimestampFormatVariants() Compared values are not the same
     Actual   (parser.count()): 4
     Expected (6)             : 6
     Loc: [test_challenger_m1.cpp(427)]
  PASS   : TestChallengerM1::testLrcMultiTimestampPerLine()
  PASS   : TestChallengerM1::testLrcOutOfOrderSorting()
  PASS   : TestChallengerM1::testLrcMetadataAndOffsets()
  PASS   : TestChallengerM1::testLrcBinarySearchStress()
  PASS   : TestChallengerM1::testLrcMalformedAndFuzzing()
  PASS   : TestChallengerM1::testLrcUnicodeAndSpecialChars()
  FAIL!  : TestChallengerM1::testLrcLargeFilePerformance() Compared values are not the same
     Actual   (parser.count()): 6000
     Expected (10000)         : 10000
     Loc: [test_challenger_m1.cpp(659)]
  QDEBUG : TestChallengerM1::cleanupTestCase() === Challenger 1 Stress Test Suite Completed ===
  PASS   : TestChallengerM1::cleanupTestCase()
  Totals: 19 passed, 2 failed, 0 skipped, 0 blacklisted, 9008ms
  ********* Finished testing of TestChallengerM1 *********
  ```
- **Code Inspection Observations**:
  - `src/core/LrcParser.cpp:85`:
    `static const QRegularExpression timeRegex("\\[(\\d{1,2}:\\d{2}[\\.:]\\d{2,3})\\]");`
    - `\\d{1,2}` restricts the minute component to at most 2 digits (max 99 minutes). Any timestamp with 3+ minute digits (e.g. `[100:00.00]`, `[120:30.00]`) fails the regex match.
    - `[\\.:]\\d{2,3}` requires 2 or 3 fractional digits. Single-digit fractions (e.g. `[00:03.4]`) and whole-second timestamps (`[00:03]`) fail the regex match, despite `parseTimestamp` having logic for 1-digit fractions at line 63.
  - `src/core/TimecodeFormatter.cpp:175`:
    `static const QRegularExpression regex("^-?\\d{1,2}:\\d{2}:\\d{2}[:;]\\d{2}$");`
    - `\\d{1,2}` causes `isValidTimecode` to return `false` for timestamps >= 100 hours (`100:00:00:00`).

## 2. Logic Chain
1. **Observation**: `testLrcTimestampFormatVariants` failed because `[00:03.4]` and `[120:30.00]` were not counted (`parser.count()` was 4 instead of 6).
2. **Observation**: `testLrcLargeFilePerformance` with 10,000 cues generated cues from minute 0 to minute 166. Exactly 4,000 cues (all cues from minute 100 to 166) were missing, resulting in `parser.count()` of 6,000.
3. **Inference**: In `LrcParser.cpp:85`, `timeRegex` specifies `\\d{1,2}` for minutes and `\\d{2,3}` for fractions.
4. **Conclusion**: Any media file with duration >= 100 minutes (concerts, podcasts, movies, audiobooks) will suffer silent loss of all lyric/subtitle cues after minute 99. Also, LRC files with decisecond or whole-second timestamps will be discarded.
5. **Observation**: In `TimecodeFormatter.cpp`, 108,000 frames (29.97 DF) and 216,000 frames (59.94 DF) continuous simulation verified that drop-frame math strictly satisfies SMPTE 12M specifications: zero illegal frame numbers were produced, 10-minute boundaries were preserved, and bidirectional roundtrip error was bounded within 1 frame. Stepping math and negative millisecond conversions are fully verified.

## 3. Caveats
- `MpvBackend` and `PlaybackEngine` live hardware rendering was tested in headless offscreen mode (`vo=null`). Full GPU OpenGL render contexts will be verified in Milestone 2 UI testing.
- The 96kHz FLAC boundary test failure in `test_tiers.py:446` is part of the synthetic fixture generator and does not affect the C++ core engine.

## 4. Conclusion
- **Verdict**: **REQUEST_CHANGES**
- **Actionable Fixes for Worker**:
  1. In `src/core/LrcParser.cpp:85`, change:
     ```cpp
     static const QRegularExpression timeRegex("\\[(\\d{1,4}:\\d{2}(?:[\\.:]\\d{1,3})?)\\]");
     ```
  2. In `src/core/TimecodeFormatter.cpp:175`, change:
     ```cpp
     static const QRegularExpression regex("^-?\\d{1,}:\\d{2}:\\d{2}[:;]\\d{2}$");
     ```

## 5. Verification Method
1. Apply the recommended fixes to `src/core/LrcParser.cpp` and `src/core/TimecodeFormatter.cpp`.
2. Recompile and run the challenger test binary:
   ```bash
   cd /home/lucifer/Documents/Projects/Penguin/tests
   qmake6 test_challenger_m1.pro -o Makefile.test_challenger
   make -f Makefile.test_challenger
   ./test_challenger_m1
   ```
3. Verify that all 21 test slots in `TestChallengerM1` pass with 0 failures:
   `Totals: 21 passed, 0 failed, 0 skipped, 0 blacklisted`.
