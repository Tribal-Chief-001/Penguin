# Handoff Report: Specification Mining for Penguin Desktop Media Player

**Author**: Specification Miner (`spec_miner_survey`)  
**Target Agent / Recipient**: Parent Orchestrator (`6821d798-6cdd-4eea-875d-d2e977529e8b`)  
**Type**: Hard Handoff (Task Complete)  
**Date**: 2026-08-31T22:38:10+05:30  

---

## 1. Observation

1. **User Request & Requirements Specification**:
   - Inspected `/home/lucifer/Documents/Projects/Penguin/.agents/ORIGINAL_REQUEST.md`.
   - Lines 14–45 define 5 core requirement areas:
     - `R1. Architecture & Core Playback Engine`: Format matrix (MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV), seeking (millisecond & SMPTE timecode), single-frame stepping (`< 1F / 1F >`), $\pm 10\text{s}$ jumps, volume, mute, speed control ($0.5\times$ to $2.0\times$), audio track switching, subtitle track switching, external subtitles (`.srt`, `.ass`, `.vtt`).
     - `R2. Tactile Digital Brutalist UI`: Deep obsidian base (`#070709`, `#0B0B0E`), 1px structural grid lines (`#1E1E24`), typography (JetBrains Mono, Swiss sans), safety orange (`#FF4400`), signal lime (`#CCFF00`). Video Viewfinder mode (safe-area reticles, OSD telemetry HUD with FPS/drops/bitrate/resolution/render time, tick ruler scrubber, control dock). Hi-Fi Audio Deck mode (typographic metadata display, stereo peak VU meters CH_L/CH_R, 10-band equalizer rack 32Hz–16kHz with presets/flat reset, synchronized `.lrc` teleprompter, playlist matrix).
     - `R3. Linux Desktop System Integration`: MPRIS2 D-Bus service `org.mpris.MediaPlayer2.penguin`, Root & Player interfaces, media keys, metadata dictionary, `.desktop` entry, scalable icon, CLI arguments, PipeWire/PulseAudio integration.
     - `R4. Media Library & State Persistence`: Open file/folder, drag-and-drop (`text/uri-list`), persistent config (`config.json`), SQLite database (`history.db`) for recent history and playlist queues, volume and window geometry restoration.
     - `R5. Automated Test Suite & Verification`: Unit test suites, headless verification script (`--test` mode), offscreen test harness.

2. **System Environment Capabilities & Probed Toolchains**:
   - `python3 --version` returned `Python 3.12.3`.
   - `pkg-config --list-all` confirmed available libraries:
     - Qt 6: `Qt6Core`, `Qt6Gui`, `Qt6Widgets`, `Qt6DBus`, `Qt6OpenGL`, `Qt6OpenGLWidgets`, `Qt6Sql`, `Qt6Svg`, `Qt6Test`, `Qt6Concurrent`.
     - FFmpeg: `libavcodec`, `libavformat`, `libavutil`, `libavfilter`, `libswscale`, `libswresample`.
     - Audio: `libpulse`, `libpulse-simple`, `libpulse-mainloop-glib`.
     - Matroska / Demuxing / Video: `libmpv2` (`libmpv.so.2` in `/usr/lib/x86_64-linux-gnu/libmpv.so.2`).
     - D-Bus: `dbus-1`, `libqt6dbus6t64`, `gdbus`, `dbus-send`, `dbus-run-session`, `dbus-daemon`.
     - Build tools: `g++`, `gcc`, `qmake6` (`QMake version 3.1, Using Qt version 6.4.2`), `make`.
     - Qt Platform Plugins: `libqoffscreen.so`, `libqxcb.so`, `libqwayland-egl.so`, `libqlinuxfb.so`.

3. **Specification Output Artifact**:
   - Created `/home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md` (386 lines, full technical specification covering all 5 domains, math formulas, schemas, D-Bus interfaces, UI design tokens, 23 discovered features, 22 edge cases, and 18 acceptance criteria).

---

## 2. Logic Chain

1. **From Requirements Analysis to Component Decomposition**:
   - Based on Observations 1 and 2, Penguin requires low-latency multimedia decoding, frame-accurate seeking, and hardware-accelerated rendering alongside complex DSP (10-band IIR biquad equalizer, stereo peak/RMS VU calculation) and desktop integration (MPRIS2 D-Bus, XDG persistence).
   - Qt 6 C++ with `libmpv2` / FFmpeg provides direct access to OpenGL rendering surfaces, native D-Bus via `Qt6DBus`, SQLite via `Qt6Sql`, and headless testing via `QT_QPA_PLATFORM=offscreen`.

2. **From Video & Audio Standards to Mathematical Specifications**:
   - **SMPTE Timecode Calculation**: Formulated non-drop frame timecode math $N = \lfloor t \times FPS + 0.5 \rfloor$ yielding `HH:MM:SS:FF`, with support for fractional rates ($23.976, 29.97, 59.94$).
   - **Stereo VU Meters**: Formulated logarithmic peak calculation $\text{dBFS} = 20 \log_{10}(\max |x| + 10^{-6})$ mapped across $[-60.0, +3.0]\text{ dB}$ with $500\text{ms}$ decay hold.
   - **10-Band Equalizer**: Standardized ISO center frequencies ($32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000\text{ Hz}$) with gain range $[-12.0, +12.0]\text{ dB}$ and preset matrices.
   - **LRC Teleprompter**: Defined cue parsing `[mm:ss.xx]`, chronological sort, binary search active lyric matching, and click-to-seek synchronization.

3. **From Desktop Protocols to D-Bus & File Schemas**:
   - Specified complete MPRIS2 interfaces `org.mpris.MediaPlayer2` and `org.mpris.MediaPlayer2.Player` including method signatures, properties, signals, and `PropertiesChanged` notifications.
   - Specified `config.json` JSON schema and `history.db` SQLite schema for state persistence.
   - Specified command-line interface arguments (`--audio`, `--video`, `--fullscreen`, `--volume`, `--test`, etc.) and `.desktop` / icon specifications.

4. **From Testing Requirements to Verification Strategy**:
   - Formulated headless verification harness using `QT_QPA_PLATFORM=offscreen` and `dbus-run-session`, ensuring all unit and integration tests run deterministically in CI / headless container environments with zero display server dependencies.

---

## 3. Caveats

1. **Display Server Variance**: In environments without a running X11 or Wayland display server, the test runner must specify `QT_QPA_PLATFORM=offscreen` or execute via `dbus-run-session`. This is explicitly documented in the specification and test fixtures.
2. **Dynamic Audio Sinks**: On some Linux installations without PipeWire/PulseAudio daemons running, the audio subsystem must fallback gracefully to ALSA or null sink during headless test execution.

---

## 4. Conclusion

The specification mining phase is complete. The resulting technical specification report `/home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md` establishes an authoritative, comprehensive blueprint for the Penguin media player across all 5 architectural domains, with concrete formulas, schemas, D-Bus interfaces, UI design tokens, and verification matrices.

---

## 5. Verification Method

To verify the deliverables:
1. View the specification report:
   ```bash
   view_file /home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md
   ```
2. Verify completeness of all 5 domains, Features Discovered table (23 items), Edge Cases table (22 items), and Acceptance Criteria matrix (18 items).
