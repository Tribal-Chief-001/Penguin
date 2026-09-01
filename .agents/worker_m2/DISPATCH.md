## 2026-08-31T17:29:48Z

You are the Implementation Worker for Milestone 2 (Tactile Digital Brutalist UI Subsystem) and M1 Refinements for Penguin.
Working directory: /home/lucifer/Documents/Projects/Penguin/.agents/worker_m2
Project Root: /home/lucifer/Documents/Projects/Penguin

Input files to read:
- /home/lucifer/Documents/Projects/Penguin/.agents/ORIGINAL_REQUEST.md
- /home/lucifer/Documents/Projects/Penguin/PROJECT.md
- /home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md
- /home/lucifer/Documents/Projects/Penguin/.agents/explorer_survey_1/engine_ui_analysis.md
- /home/lucifer/Documents/Projects/Penguin/.agents/challenger_m1_1/challenge.md

MANDATORY INTEGRITY WARNING:
DO NOT CHEAT. All implementations must be genuine. DO NOT hardcode test results, create dummy/facade implementations, or circumvent the intended task. A auditor will independently verify your work. Integrity violations WILL be detected and your work WILL be rejected.

Objective:
1. Apply the M1 regex refinements identified by Challenger 1:
   - In `src/core/LrcParser.cpp`: Update `timeRegex` to `\\[(\\d{1,4}:\\d{2}(?:[\\.:]\\d{1,3})?)\\]` to support >= 100 min and 1-digit deciseconds/integer seconds.
   - In `src/core/TimecodeFormatter.cpp`: Update `isValidTimecode` regex to `^-?\\d{1,}:\\d{2}:\\d{2}[:;]\\d{2}$`.
2. Implement the Tactile Digital Brutalist UI Subsystem in `src/ui/`:
   - `src/ui/BrutalistTheme.h / .cpp`: Theme engine with palette tokens (`#070709`, `#0B0B0E`, `#1E1E24`, `#FF4400`, `#CCFF00`, `#00E5FF`), JetBrains Mono and Swiss sans typography, 1px structural borders, and QSS stylesheets.
   - `src/ui/TickScrubberWidget.h / .cpp`: Custom QPainter mechanical tick ruler scrubber with SMPTE timecodes, tick marks, chapter markers, remaining time display, and interactive scrubbing.
   - `src/ui/VUMeterWidget.h / .cpp`: Animated stereo CH_L / CH_R peak VU meter bars with logarithmic dB scale (-60dB to +3dB), color-coded LEDs (green, lime, yellow, red), peak hold, and clip flags.
   - `src/ui/EqualizerRackWidget.h / .cpp`: 10-band tactile slider rack (32Hz to 16kHz), preset dropdown (Flat, Rock, Pop, Jazz, Electronic, Vocal Boost, Bass Boost, Treble Boost, Classical, Acoustic), Flat reset button, and real-time EQ gain adjustments connected to `PlaybackEngine`.
   - `src/ui/TeleprompterWidget.h / .cpp`: Synchronized .lrc lyrics teleprompter widget with smooth scrolling, active line highlighting in safety orange/lime, and click-to-seek on any lyric line.
   - `src/ui/PlaylistMatrixWidget.h / .cpp`: Brutalist playlist queue table/matrix with track numbers, titles, artists, durations, active playback indicator, item selection, and reordering.
   - `src/ui/DiagnosticsHUDWidget.h / .cpp`: Customizable OSD telemetry overlay (FPS, dropped frames, bitrate, resolution, render time, codec info).
   - `src/ui/ViewfinderWidget.h / .cpp`: Video Viewfinder Mode container with video viewport, safe-area reticles (Action 90% / Title 80%), OSD telemetry overlay, tick scrubber, and bottom tactile control dock (frame step, speed toggles, stream selector, fullscreen toggle).
   - `src/ui/AudioDeckWidget.h / .cpp`: Hi-Fi Audio Deck Mode container with large typographic track & artist display, stereo VU meter rack, 10-band equalizer rack, synchronized LRC teleprompter, and playlist queue matrix.
   - `src/ui/MainWindow.h / .cpp`: Main application window managing dynamic seamless switching between Video Viewfinder Mode and Hi-Fi Audio Deck Mode, keyboard shortcuts (Space, Left/Right, F, M, <, >, numbers), and media loading.
3. Update `penguin.pro` and `CMakeLists.txt` to include `src/ui/` files.
4. Implement UI unit/headless tests in `tests/test_m2_ui.cpp` verifying offscreen rendering (`QT_QPA_PLATFORM=offscreen`), mode switching, scrubber math, and widget signals.
5. Build and execute unit tests for M1 and M2, verifying all tests pass with 0 errors.

Output requirements:
- Write implementation code to `src/ui/` and update `src/core/`.
- Compile and execute tests.
- Write handoff report to `/home/lucifer/Documents/Projects/Penguin/.agents/worker_m2/handoff.md`.
- Send completion message when finished.
