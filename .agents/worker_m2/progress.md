# Progress Log

Last visited: 2026-08-31T17:40:00Z

## Completed Tasks
- [x] M1 Regex Refinements:
  - Updated `timeRegex` in `src/core/LrcParser.cpp` to `\\[(\\d{1,4}:\\d{2}(?:[\\.:]\\d{1,3})?)\\]`.
  - Updated `isValidTimecode` in `src/core/TimecodeFormatter.cpp` to `^-?\\d{1,}:\\d{2}:\\d{2}[:;]\\d{2}$`.
  - Verified with `test_challenger_m1` (21/21 tests passing).
- [x] Tactile Digital Brutalist UI Subsystem (`src/ui/`):
  - `BrutalistTheme.h / .cpp`: Theme engine with palette tokens (`#070709`, `#0B0B0E`, `#141418`, `#1E1E24`, `#333342`, `#FFFFFF`, `#777788`, `#444455`, `#FF4400`, `#CCFF00`, `#00E5FF`, `#FF2200`), JetBrains Mono/Swiss sans typography, 0px border-radius, 1px structural borders, and complete QSS styling.
  - `TickScrubberWidget.h / .cpp`: Custom QPainter mechanical tick ruler progress scrubber with SMPTE timecodes, major/minor tick marks, chapter markers, remaining time calculation, and interactive scrubbing.
  - `VUMeterWidget.h / .cpp`: Stereo CH_L / CH_R peak VU meter bars with logarithmic dB scale (-60dB to +3dB), color-coded segmented LEDs, peak hold with decay, and clip indicators.
  - `EqualizerRackWidget.h / .cpp`: 10-band tactile vertical slider rack (32Hz to 16kHz), preset dropdown (Flat, Rock, Pop, Jazz, Electronic, Vocal Boost, Bass Boost, Treble Boost, Classical, Acoustic), Flat reset button, and real-time gain control.
  - `TeleprompterWidget.h / .cpp`: Synchronized .lrc lyrics teleprompter widget with smooth scrolling, active line centering, lime/orange highlighting, and click-to-seek support.
  - `PlaylistMatrixWidget.h / .cpp`: Brutalist playlist queue table/matrix with track indices, titles, artists, albums, durations, formats, active track highlighting, row reordering, filtering, and shuffle.
  - `DiagnosticsHUDWidget.h / .cpp`: OSD telemetry overlay (FPS, dropped frames, video/audio bitrate, resolution, color space, render time, A/V skew).
  - `ViewfinderWidget.h / .cpp`: Video Viewfinder Mode container with video viewport, action-safe (90%) and title-safe (80%) reticle overlays, center crosshair, OSD HUD overlay, tick scrubber, and bottom tactile control dock (frame step, speed selector, audio/sub stream selectors, volume/mute).
  - `AudioDeckWidget.h / .cpp`: Hi-Fi Audio Deck Mode container with typographic metadata masthead, stereo VU meter rack, 10-band equalizer rack, synchronized LRC teleprompter, playlist queue matrix, and transport controls.
  - `MainWindow.h / .cpp`: Main application window managing dynamic seamless switching between Video Viewfinder Mode and Hi-Fi Audio Deck Mode, global keyboard shortcuts (Space, Left/Right, Comma/Period, F, M, Tab, etc.), and drag-and-drop media loading.
- [x] Build System Updates:
  - Updated `CMakeLists.txt` with `src/ui/` sources/headers and `test_m2_ui` test target.
  - Updated `penguin.pro` with `src/ui/` sources/headers.
- [x] Automated Verification Suite:
  - Created `tests/test_m2_ui.cpp` and `tests/test_m2_ui.pro`.
  - Executed `./tests/test_m2_ui` (12/12 passing).
  - Executed `./tests/test_challenger_m1` (21/21 passing).
  - Executed `./scripts/run_tests.sh` (327/327 passing).
  - Executed `python3 -m unittest discover` (658/658 passing).
