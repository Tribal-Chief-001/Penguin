# Project: Penguin — Tactile Digital Brutalist Linux Desktop Media Player

## Architecture
Penguin is built as a high-performance Linux desktop media player adhering to the Tactile Digital Brutalism design language with dual playback modes (Video Viewfinder & Hi-Fi Audio Deck), libmpv core decoding, audio DSP (10-band equalizer + real-time stereo VU meters), MPRIS2 D-Bus integration, SQLite state persistence, and a headless-capable test architecture.

```
+-------------------------------------------------------------------------------+
|                                Penguin CLI / App                              |
|                          (main.cpp / CommandLineParser)                       |
+---------------------------------------+---------------------------------------+
                                        |
        +-------------------------------+-------------------------------+
        |                               |                               |
+-------v---------------+       +-------v---------------+       +-------v---------------+
|    UI Subsystem       |       | Playback Engine Core  |       | System Integration    |
| (Video Viewfinder &   |<----->| (MpvBackend / Audio   |<----->| (MPRIS2 D-Bus, XDG,   |
|  Hi-Fi Audio Deck)    |       |  DSP / TrackManager)  |       |  Desktop, CLI)        |
+-------+---------------+       +-------+---------------+       +-------+---------------+
        |                               |                               |
        +-------------------------------+-------------------------------+
                                        |
                        +---------------v---------------+
                        | Library & State Persistence   |
                        | (SQLite WAL / Playlist / Hist)|
                        +-------------------------------+
```

## Feature Inventory
| # | Feature | Description | Milestone | Source |
|---|---------|-------------|-----------|--------|
| F01 | Multi-format Media Playback | Play MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV with accurate duration/position | M1 | ORIGINAL_REQUEST §R1 |
| F02 | Millisecond & SMPTE Seeking | Exact seek to ms and SMPTE timecode (HH:MM:SS:FF) | M1 | ORIGINAL_REQUEST §R1 |
| F03 | Frame Stepping & Jumps | Single-frame step (< 1F / 1F >) and +/- 10s instant jumps | M1 | ORIGINAL_REQUEST §R1 |
| F04 | Speed Control & Pitch Correction | Variable rate 0.5x to 2.0x with pitch preservation | M1 | ORIGINAL_REQUEST §R1 |
| F05 | Track Switching & External Subs | Dynamic audio stream switching, subtitle stream switching, external .srt/.ass/.vtt | M1 | ORIGINAL_REQUEST §R1 |
| F06 | 10-Band Graphic Equalizer DSP | 10-band peaking filter rack (32Hz-16kHz, +/-12dB) with factory presets & flat reset | M1 | ORIGINAL_REQUEST §R1, §R2 |
| F07 | Stereo Peak VU Meter DSP | Real-time stereo channel peak/RMS level extraction (-60dB to +3dB) with decay | M1 | ORIGINAL_REQUEST §R2 |
| F08 | Tactile Brutalist Design System | Deep obsidian base (#070709, #0B0B0E), 1px grid (#1E1E24), JetBrains Mono, safety orange (#FF4400) & signal lime (#CCFF00) | M2 | ORIGINAL_REQUEST §R2 |
| F09 | Video Viewfinder Mode | Borderless video viewport, safe-area reticles, technical diagnostics HUD (FPS, drops, bitrate, res, render time) | M2 | ORIGINAL_REQUEST §R2 |
| F10 | Mechanical Tick Ruler Scrubber | Mechanical tick ruler scrubber with SMPTE timecodes, chapter marks, remaining time | M2 | ORIGINAL_REQUEST §R2 |
| F11 | Tactile Video Control Dock | Bottom dock with frame-step, speed toggles, stream switchers, fullscreen/PiP | M2 | ORIGINAL_REQUEST §R2 |
| F12 | Hi-Fi Audio Deck Mode | Typographic metadata board, animated stereo VU meters, 10-band slider rack | M2 | ORIGINAL_REQUEST §R2 |
| F13 | Synchronized LRC Teleprompter | Real-time .lrc lyric parser with active line highlighting and click-to-seek | M2 | ORIGINAL_REQUEST §R2 |
| F14 | Playlist Queue Matrix | Interactive queue matrix with track reordering, status indicators, and durations | M2 | ORIGINAL_REQUEST §R2, §R4 |
| F15 | MPRIS2 D-Bus Interface | Register org.mpris.MediaPlayer2.penguin on session bus, Root & Player interfaces, media keys | M3 | ORIGINAL_REQUEST §R3 |
| F16 | Desktop Packaging & Icon | FreeDesktop .desktop entry with media action shortcuts, scalable SVG icon | M3 | ORIGINAL_REQUEST §R3 |
| F17 | CLI Options & Arguments | Command-line parsing: files/URLs, --audio, --video, --fullscreen, --eq, --test, --help | M3 | ORIGINAL_REQUEST §R3 |
| F18 | Audio Subsystem Routing | PipeWire & PulseAudio integration with graceful fallback | M3 | ORIGINAL_REQUEST §R3 |
| F19 | File & Directory Loading | Open file, recursive folder scan, drag-and-drop (text/uri-list) | M4 | ORIGINAL_REQUEST §R4 |
| F20 | State & History Persistence | SQLite WAL database storing playback history, volume, window geometry, playlist | M4 | ORIGINAL_REQUEST §R4 |
| F21 | Synthetic Test Media Generator | Zero-dependency test media generator for multi-track video/audio/subtitles | M5 | ORIGINAL_REQUEST §R5 |
| F22 | Comprehensive Automated Unit Tests | Unit tests for playback pipeline, Biquad EQ math, SMPTE, LRC parsing, MPRIS2 | M5 | ORIGINAL_REQUEST §R5 |
| F23 | Headless Verification Runner | CLI test mode and headless verification script (QT_QPA_PLATFORM=offscreen) | M5 | ORIGINAL_REQUEST §R5 |

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Core Playback Engine & Audio DSP | `src/core/` — MpvBackend, Timecode, EqualizerDSP, VUMeterDSP, TrackManager | none | DONE |
| M2 | Tactile Digital Brutalist UI | `src/ui/` — Viewfinder, AudioDeck, Scrubber, VU Meter, EQ Rack, Teleprompter, Theme | M1 | DONE |
| M3 | Linux Desktop Integration, MPRIS2 & CLI | `src/desktop/` — MPRIS2 D-Bus adaptor, .desktop, Icon, CLI parser, Hotkeys | M1 | DONE |
| M4 | Media Library & State Persistence | `src/library/` — SQLite WAL DB, History, Playlist matrix, Drag&Drop | M1, M2 | DONE |
| M5 | E2E Test Suite & Full Verification | `tests/` — Tiers 1-4 test suite, Tier 5 adversarial tests, headless runner | M1, M2, M3, M4 | DONE |

## Interface Contracts

### 1. Core Engine ↔ UI Subsystem (`src/core/` ↔ `src/ui/`)
- `PlaybackEngine`:
  - `loadMedia(const QString &uri, bool autoPlay = true)` -> `bool`
  - `play()`, `pause()`, `togglePlayPause()`, `stop()`
  - `seek(qint64 positionMs)`, `seekRelative(qint64 offsetMs)`, `frameStep(int direction)`
  - `setSpeed(double rate)` (0.5 to 2.0), `setVolume(int volume)` (0 to 100), `setMuted(bool mute)`
  - `setEqualizerBand(int bandIndex, double gainDb)`, `setEqualizerPreset(const QString &presetName)`
  - Signals:
    - `positionChanged(qint64 positionMs, const QString &smpteTimecode)`
    - `durationChanged(qint64 durationMs, const QString &smpteTimecode)`
    - `playbackStateChanged(PlaybackState state)`
    - `vuLevelsChanged(double leftPeakDb, double rightPeakDb, double leftRmsDb, double rightRmsDb)`
    - `tracksChanged(const QList<TrackInfo> &audioTracks, const QList<TrackInfo> &subTracks)`
    - `telemetryUpdated(const DiagnosticsData &telemetry)`

### 2. Core Engine ↔ MPRIS2 D-Bus (`src/core/` ↔ `src/desktop/`)
- `MPRIS2Adaptor` maps D-Bus calls:
  - `Play()`, `Pause()`, `PlayPause()`, `Stop()`, `Next()`, `Previous()`, `Seek(qint64 offsetMicroseconds)`, `SetPosition(const QDBusObjectPath &trackId, qint64 positionMicroseconds)`
  - Properties: `PlaybackStatus` ("Playing", "Paused", "Stopped"), `LoopStatus`, `Rate`, `Shuffle`, `Metadata` (`mpris:trackid`, `mpris:length`, `xesam:title`, `xesam:artist`, `xesam:album`, `xesam:url`), `Volume`, `Position`, `CanControl`, `CanSeek`, `CanPlay`, `CanPause`.
  - Signal: `PropertiesChanged` on `org.freedesktop.DBus.Properties`.

### 3. Core Engine & UI ↔ State Persistence (`src/library/`)
- `DatabaseManager`:
  - `savePlaybackHistory(const QString &uri, const QString &title, qint64 positionMs, qint64 durationMs)`
  - `getRecentHistory(int limit = 50)` -> `QList<HistoryEntry>`
  - `savePlaylist(const QString &name, const QList<PlaylistItem> &items)`
  - `loadPlaylist(const QString &name)` -> `QList<PlaylistItem>`
  - `saveSetting(const QString &key, const QVariant &value)`
  - `getSetting(const QString &key, const QVariant &defaultValue = QVariant())` -> `QVariant`

## Code Layout
```
/home/lucifer/Documents/Projects/Penguin/
├── CMakeLists.txt                # Root CMake / QMake build configuration
├── penguin.pro                   # Qt6 project file
├── README.md                     # Build, install, and usage documentation
├── penguin.desktop               # FreeDesktop XDG Desktop Entry
├── icons/                        # Scalable and pixel icons
│   ├── hicolor/
│   │   ├── scalable/apps/penguin.svg
│   │   ├── 48x48/apps/penguin.png
│   │   └── 256x256/apps/penguin.png
├── src/
│   ├── main.cpp                  # Application entry point & CLI routing
│   ├── core/                     # Playback engine, DSP, timecode, tracks
│   │   ├── PlaybackEngine.h / .cpp
│   │   ├── MpvBackend.h / .cpp
│   │   ├── TimecodeFormatter.h / .cpp
│   │   ├── EqualizerDSP.h / .cpp
│   │   ├── VUMeterDSP.h / .cpp
│   │   ├── SubtitleLoader.h / .cpp
│   │   └── LrcParser.h / .cpp
│   ├── ui/                       # Tactile Digital Brutalist UI
│   │   ├── MainWindow.h / .cpp
│   │   ├── BrutalistTheme.h / .cpp
│   │   ├── ViewfinderWidget.h / .cpp
│   │   ├── AudioDeckWidget.h / .cpp
│   │   ├── TickScrubberWidget.h / .cpp
│   │   ├── VUMeterWidget.h / .cpp
│   │   ├── EqualizerRackWidget.h / .cpp
│   │   ├── TeleprompterWidget.h / .cpp
│   │   ├── PlaylistMatrixWidget.h / .cpp
│   │   └── DiagnosticsHUDWidget.h / .cpp
│   ├── desktop/                  # MPRIS2 D-Bus & system integration
│   │   ├── DBusService.h / .cpp
│   │   ├── MPRIS2Adaptor.h / .cpp
│   │   └── CommandLineParser.h / .cpp
│   └── library/                  # Database, persistence & file management
│       ├── DatabaseManager.h / .cpp
│       ├── StatePersistence.h / .cpp
│       └── PlaylistManager.h / .cpp
├── tests/                        # Comprehensive test suite
│   ├── test_synthetic_media.py   # Test media synthesis engine
│   ├── test_timecode.cpp / .py   # SMPTE timecode calculation tests
│   ├── test_equalizer_dsp.cpp    # Analytical Biquad filter verification
│   ├── test_lrc_parser.cpp       # Synchronized LRC cue parsing tests
│   ├── test_mpris2_dbus.py       # MPRIS2 D-Bus protocol validation tests
│   ├── test_persistence.cpp      # SQLite WAL persistence unit tests
│   ├── test_playback_engine.cpp  # Multi-format playback engine test
│   ├── test_ui_headless.cpp      # Offscreen QPA UI rendering tests
│   └── test_e2e_runner.py        # Comprehensive headless verification harness
└── scripts/
    ├── build.sh                  # One-step compilation script
    ├── run_tests.sh              # Headless test runner script
    └── package.sh                # Desktop packaging & installation script
```
