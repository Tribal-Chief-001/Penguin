# Project: Penguin Desktop Media Player

## 1. Architecture Overview
Penguin is a next-generation Linux desktop media player built with a Tactile Digital Brutalist UI design language, a dual-mode playback engine (Viewfinder Video Mode & Hi-Fi Audio Deck Mode), high-performance media decoding via `libmpv` and FFmpeg, standard MPRIS2 D-Bus integration, SQLite WAL state persistence, and a multi-tiered automated test suite.

```
+---------------------------------------------------------------------------------------+
|                                    PENGUIN CORE GUI                                    |
|  +-------------------------------------+   +---------------------------------------+  |
|  |      VIEWFINDER VIDEO VIEWPORT      |   |            HI-FI AUDIO DECK           |  |
|  | - Technical Safe-Area Reticles      |   | - Typographic Masthead & Artwork      |  |
|  | - Mechanical SMPTE Tick Scrubber    |   | - Stereo Peak VU Meters (CH_L & CH_R) |  |
|  | - Real-time Diagnostics HUD (OSD)   |   | - 10-Band Graphic Equalizer Rack      |  |
|  | - Tactile Bottom Video Control Dock |   | - Synchronized .lrc Teleprompter      |  |
|  | - Native Video Surface (wid/X11)    |   | - Playlist Queue Matrix               |  |
|  +-------------------------------------+   +---------------------------------------+  |
+-------------------------------------------+-------------------------------------------+
                                            |
                               Qt Signals / Direct Calls
                                            v
+---------------------------------------------------------------------------------------+
|                                PENGUIN PLAYBACK ENGINE                                |
|  - Transport: Play, Pause, Stop, Seek (Exact ms & SMPTE), Frame-Step (< 1F / 1F >)    |
|  - Speed Scaling (0.5x to 2.0x pitch-preserved) & Audio/Subtitle Track Switchers      |
|  - Audio Filters: 10-Band Graphic EQ DSP + Night Mode Compressor (`dynaudnorm`)       |
|  - Video Processing: Deband Dithering + Lossless Forensic PNG Screenshot Export       |
|  - A-B Looper & Stream URL Ingestion (yt-dlp / direct HTTP)                           |
+-------------------------------------+-------------------------------------------------+
                                      |
                    +-----------------+-----------------+
                    |                                   |
                    v                                   v
+---------------------------------------+   +-------------------------------------------+
|          LIBMPV BACKEND CORE          |   |       AUDIO ANALYSIS & VU PIPELINE        |
|  - HW Decode (VAAPI / NVDEC / SW)     |   |  - Dual-Path Waveform Peak/RMS Cache      |
|  - Audio Output: PipeWire / PulseAudio|   |  - 60 FPS Ballistic Needle Dynamics       |
|  - Subtitles Engine (libass)          |   |  - Peak Hold & Overload Clip Alerts       |
+---------------------------------------+   +-------------------------------------------+
                    |                                   |
                    +-----------------+-----------------+
                                      |
                                      v
+---------------------------------------------------------------------------------------+
|                        DESKTOP INTEGRATION & PERSISTENCE                              |
|  - MPRIS2 D-Bus Service (`org.mpris.MediaPlayer2.penguin` - Root & Player interfaces) |
|  - CLI Argument Parser (`penguin [file] --audio --video --fullscreen ...`)            |
|  - Desktop Packaging (`penguin.desktop`, scalable SVG icon, XDG directory compliance) |
|  - SQLite WAL Database (`penguin.db` - playlists, history, settings, geometry)       |
+---------------------------------------------------------------------------------------+
```

---

## 2. Feature Inventory
Every feature from the survey phase is assigned to a milestone:

| # | Category | Feature | Description | Milestone | Status |
|---|---|---|---|---|---|
| 1 | Core Engine | libmpv FFI Core Binding | Direct ctypes/C-API binding to `libmpv.so.2` | M1 | **DONE** |
| 2 | Core Engine | Multi-Format A/V Playback | MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV | M1 | **DONE** |
| 3 | Core Engine | Transport & Exact ms Seeking | Play, pause, stop, volume, mute, absolute & relative ms seek | M1 | **DONE** |
| 4 | Core Engine | SMPTE 12M Timecode Engine | Accurate SMPTE timecode (NDF & 29.97 DF) | M1 | **DONE** |
| 5 | Core Engine | Bidirectional Frame Stepping | Single-frame forward (`1F >`) and backward (`< 1F`) | M1 | **DONE** |
| 6 | Core Engine | Pitch-Preserved Speed Control | Speed scaling from 0.5x to 2.0x via scaletempo2 | M1 | **DONE** |
| 7 | Core Engine | Track Introspection & Switch | Dynamic audio and subtitle track discovery and selection | M1 | **DONE** |
| 8 | Core Engine | External Subtitle Loading | Loading `.srt`, `.ass`, `.vtt` subtitle files | M1 | **DONE** |
| 9 | Core Engine | A-B Repeat Looping | Setting and looping between time A and time B | M1 | **DONE** |
| 10 | Core Engine | Night Mode Dialogue Compressor | Dynamic dialogue boost audio filter (`dynaudnorm` / `acompressor`)| M1 | **DONE** |
| 11 | Core Engine | Deband & Video Dithering | Gradient debanding and dither processing | M1 | **DONE** |
| 12 | Core Engine | Forensic Screenshot Export | Frame-accurate lossless PNG screenshot export with metadata | M1 | **DONE** |
| 13 | Core Engine | Network Stream URL Ingestion | Direct extraction and streaming of web video URLs | M1 | **DONE** |
| 14 | UI | Tactile Brutalist Design Tokens | Obsidian base (#070709), 1px grid (#1E1E24), Swiss/Monospace fonts, Safety Orange & Lime accents | M2 | IN_PROGRESS |
| 15 | UI | Video Viewfinder Mode | Native video surface, safe-area reticles, telemetry OSD HUD, SMPTE tick scrubber, bottom dock | M2 | IN_PROGRESS |
| 16 | UI | Hi-Fi Audio Deck Mode | Typographic masthead, stereo peak VU meters, 10-band EQ rack, synced .lrc teleprompter, playlist matrix | M2 | IN_PROGRESS |
| 17 | UI | Stereo Peak VU Meter Rack | Animated 30-segment LED ballistics (CH_L & CH_R) with peak-hold & clip alerts | M2 | IN_PROGRESS |
| 18 | UI | 10-Band Graphic Equalizer Rack | ISO center frequencies (32Hz-16kHz), ±12dB sliders, presets, flat reset | M2 | IN_PROGRESS |
| 19 | UI | Synchronized LRC Teleprompter | Line and word-level .lrc parsing, active highlight, smooth auto-scroll, click-to-seek | M2 | IN_PROGRESS |
| 20 | UI | Playlist Queue Matrix | Grid display, search filter, track reordering, shuffle | M2 | IN_PROGRESS |
| 21 | Desktop | MPRIS2 D-Bus Service | `org.mpris.MediaPlayer2.penguin` Root and Player interfaces, properties & signals | M3 | IN_PROGRESS |
| 22 | Desktop | Single-Instance IPC & CLI | Command-line argument parsing and remote command forwarding | M3 | IN_PROGRESS |
| 23 | Desktop | Audio Routing (PipeWire/Pulse) | Low-latency audio sink configuration via PipeWire/PulseAudio | M3 | IN_PROGRESS |
| 24 | Desktop | Desktop Packaging & Assets | `.desktop` launcher, scalable SVG icon, hicolor icon hierarchy | M3 | IN_PROGRESS |
| 25 | Persistence | SQLite WAL State Database | Schema creation, WAL mode, crash recovery, settings storage | M4 | IN_PROGRESS |
| 26 | Persistence | History & Playlist Persistence | Track playback history, saved playlists, resume bookmarks | M4 | IN_PROGRESS |
| 27 | Persistence | Window & DSP State Restore | Window geometry, volume, mute, mode, EQ presets persistence | M4 | IN_PROGRESS |
| 28 | Verification | Multi-Tier E2E Test Suite | Tiers 1-4 requirement-driven opaque-box test suite (399 tests) | E2E/M5 | **DONE** |
| 29 | Verification | Adversarial Hardening (Tier 5) | White-box adversarial stress testing and edge-case validation | M5 | PLANNED |
| 30 | Verification | Headless CLI Verification | Headless runner executing 100% of test suites with exit code 0 | M5 | **DONE** |

---

## 3. Milestones & Dependencies

| # | Name | Scope | Dependencies | Status |
|---|---|---|---|---|
| **M1** | Core Playback Engine & Audio DSP | `src/engine/` - MPV backend, transport, SMPTE, stepping, EQ DSP, night mode, deband, screenshots, subtitle engine | none | **DONE** |
| **M2** | Tactile Digital Brutalist GUI | `src/ui/` - Theme tokens, Viewfinder mode, Audio Deck mode, VU meter widget, EQ rack, LRC teleprompter, tick scrubber, playlist matrix | M1 | IN_PROGRESS |
| **M3** | Linux Desktop Integration | `src/desktop/` - MPRIS2 D-Bus service, CLI parser, single-instance IPC, PipeWire/Pulse routing, `.desktop` & icons | M1 | IN_PROGRESS |
| **M4** | Media Library & State Persistence | `src/library/` - SQLite WAL database, schema migrations, history, playlists, settings persistence | M1 | IN_PROGRESS |
| **E2E** | E2E Test Track (Parallel) | `tests/` & `run_tests.py` - Test harness, test runners, Tier 1-4 test suites (Category-Partition, BVA, Pairwise, Real-World) | none | **DONE** |
| **M5** | Final Integration & E2E 100% Pass | Full system integration, Phase 1 (100% Tier 1-4 Pass) & Phase 2 (Adversarial Tier 5 Hardening) | M1, M2, M3, M4, E2E | PLANNED |
