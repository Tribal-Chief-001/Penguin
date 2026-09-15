# Original User Request

## 2026-09-01T07:16:26Z

# Teamwork Project Prompt

Build "Penguin" — a next-generation, universal Linux desktop media player featuring a Tactile Digital Brutalist UI (Studio Precision & Viewfinder Cinema aesthetics), dual-mode playback engine (Viewfinder Video & Hi-Fi Audio Deck), high-performance media decoding, and deep Linux desktop integration.

Working directory: /home/lucifer/Documents/Projects/Penguin
Integrity mode: development

## Requirements

### R1. Architecture & Core Playback Engine
- Implement a robust media engine capable of playing modern audio and video formats (MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV, etc.).
- Provide seamless playback controls: play, pause, stop, seek (millisecond & SMPTE timecode accuracy), single-frame stepping (< 1F / 1F >), +/- 10s jumps, volume, mute, and speed control (0.5x to 2.0x).
- Support audio track switching, subtitle track switching, and external subtitle loading (.srt, .ass, .vtt).
- Support A-B repeat looping, dynamic night mode dialogue compressor, deband dithering, forensic screenshot export, and network stream URL ingestion.

### R2. Tactile Digital Brutalist UI (Qt 6 / Modern Native GUI)
- Implement the "Tactile Digital Brutalism" design language:
  - Color Palette & Geometry: Deep obsidian base (#070709, #0B0B0E), razor-sharp 1px structural grid lines (#1E1E24), high-contrast monospace (JetBrains Mono) and Swiss sans typography, industrial safety orange (#FF4400) and signal lime (#CCFF00) tactile active indicators.
  - Mode 1: Video Viewfinder Mode:
    - Borderless, clean video viewport with technical safe-area reticles and customizable OSD hardware/diagnostics telemetry (FPS, dropped frames, codec bitrate, resolution, render time).
    - Mechanical timecode tick ruler progress scrubber with chapter markers and remaining time calculation.
    - Tactile bottom control dock with frame-step buttons, speed toggles, stream switchers, and fullscreen/PiP toggles.
  - Mode 2: Hi-Fi Audio Deck Mode:
    - Large typographic track & artist display.
    - Animated stereo peak VU meters (CH_L & CH_R) responding to playback.
    - 10-band tactile graphic equalizer rack (32Hz to 16kHz) with presets and flat reset.
    - Synchronized lyrics teleprompter supporting standard .lrc timestamps and real-time active lyric tracking.
    - Playlist queue matrix with drag-and-drop / track reordering.

### R3. Linux Desktop System Integration
- MPRIS2 D-Bus Interface: Register Penguin on D-Bus (org.mpris.MediaPlayer2.penguin) with standard Player and Root interfaces, supporting system media keys, lock screen widgets, GNOME/KDE media controllers, and metadata broadcasting.
- Desktop Packaging & Entry: Provide .desktop file, scalable icon, and command-line arguments (e.g. penguin [file/url], --audio, --video, --fullscreen, --help).
- Audio Routing: Integrate smoothly with PipeWire and PulseAudio.

### R4. Media Library & State Persistence
- Support open file, open folder/directory, drag-and-drop files directly onto the window.
- Persist playlist queue, volume level, recent playback history, and window geometry across sessions in a local config/database (SQLite WAL).

### R5. Comprehensive Automated Test Suite & Verification
- Unit test suite covering the playback pipeline, audio/video file parsing, metadata extraction, .lrc lyric parsing, equalizer DSP/filter logic, and MPRIS2 D-Bus interface.
- Headless verification script or CLI test mode to validate all core functionality programmatically.

## Acceptance Criteria

### Playback & Engine
- [ ] Successfully plays test audio files (e.g. MP3, FLAC, WAV) and video files (e.g. MP4, MKV, WebM) with correct duration and position tracking.
- [ ] Seeking is accurate and responsive; frame stepping and speed adjustments operate without audio/video desync.
- [ ] Subtitle and audio track selectors discover all embedded tracks and switch between them dynamically.

### User Interface & Experience
- [ ] Application launches smoothly with the complete Brutalist dark theme styling matching the design specifications.
- [ ] Dynamic switching between Video Viewfinder Mode and Audio Hi-Fi Deck Mode works seamlessly.
- [ ] Scrubber tick ruler updates in real time with formatted SMPTE timecode (HH:MM:SS:FF).
- [ ] Audio mode displays real-time VU meter animations and updates synchronized .lrc lyrics line-by-line during playback.
- [ ] Graphic equalizer sliders adjust audio frequency gain.

### Desktop Integration & CLI
- [ ] MPRIS2 interface responds to Play, Pause, Next, Previous, and updates metadata (title, artist, duration, position).
- [ ] Launching from terminal with a media file path (e.g. penguin /path/to/media.mp4) opens and begins playback immediately.
- [ ] Keyboard shortcuts work reliably (Space for play/pause, Left/Right for seek, F for fullscreen, M for mute).

### Code Quality & Testing
- [ ] All automated tests pass with 0 errors.
- [ ] Code is well-structured, modular, documented, and includes instructions in README.md on building, running, and installing Penguin.
