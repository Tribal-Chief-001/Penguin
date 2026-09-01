## 2026-08-31T17:10:28Z

You are the Implementation Worker for Milestone 1 (Core Playback Engine & Audio DSP Subsystem) of Penguin Media Player.
Working directory: /home/lucifer/Documents/Projects/Penguin/.agents/worker_m1
Project Root: /home/lucifer/Documents/Projects/Penguin

Input files to read:
- /home/lucifer/Documents/Projects/Penguin/.agents/ORIGINAL_REQUEST.md
- /home/lucifer/Documents/Projects/Penguin/PROJECT.md
- /home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md
- /home/lucifer/Documents/Projects/Penguin/.agents/explorer_survey_1/engine_ui_analysis.md

MANDATORY INTEGRITY WARNING:
DO NOT CHEAT. All implementations must be genuine. DO NOT hardcode test results, create dummy/facade implementations, or circumvent the intended task. A auditor will independently verify your work. Integrity violations WILL be detected and your work WILL be rejected.

Objective:
Implement Milestone 1 in \`src/core/\` and root build files:
1. \`CMakeLists.txt\` and \`penguin.pro\` targeting Qt6 with Core, Gui, Widgets, DBus, Sql, Svg, OpenGL, libmpv2.
2. \`src/core/TimecodeFormatter.h / .cpp\`: Exact SMPTE timecode (HH:MM:SS:FF) calculation math, support 24, 25, 30, 50, 60, 23.976, 29.97, 59.94 FPS, millisecond conversions.
3. \`src/core/LrcParser.h / .cpp\`: Standard [mm:ss.xx] timestamp parser, chronological sorting, binary search for active lyric cue at current playback position, click-to-seek timestamp mapping.
4. \`src/core/EqualizerDSP.h / .cpp\`: 10-band ISO graphic equalizer rack (31.25Hz to 16kHz, +/-12dB gain), peaking biquad IIR filter transfer functions, 8 factory presets (Flat, Rock, Pop, Jazz, Electronic, Vocal Boost, Bass Boost, Treble Boost) + custom, and filter graph serialization for mpv/gstreamer.
5. \`src/core/VUMeterDSP.h / .cpp\`: Stereo CH_L & CH_R audio level processing, RMS & Peak dBFS conversion (range -60dB to +3dB), decay/ballistics physics model (60Hz tick).
6. \`src/core/SubtitleLoader.h / .cpp\`: Parsing and validation for external .srt, .ass, and .vtt subtitle files.
7. \`src/core/PlaybackEngine.h / .cpp\` & \`src/core/MpvBackend.h / .cpp\`: Complete media engine using \`libmpv2\` C API with Qt signal/slot architecture:
   - Formats: MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV.
   - Controls: load, play, pause, togglePlayPause, stop, exact seek (ms), relative seek (+/-10s), frame stepping (< 1F / 1F >), speed rate (0.5x to 2.0x) with pitch correction, volume (0-100), mute.
   - Stream Management: Audio track discovery & switching, subtitle track discovery & switching, external subtitle loading.
   - Telemetry & Metadata: Extraction of duration, position, FPS, dropped frames, bitrate, video resolution, codec name, title, artist, album.
8. Unit tests in \`tests/test_m1_core.cpp\` (or equivalent test runner) verifying all M1 components. Compile and run the tests, and verify they pass with 0 errors.
