# Penguin — Tactile Digital Brutalist Linux Desktop Media Player

**Penguin** is a next-generation, high-performance Linux desktop media player featuring a **Tactile Digital Brutalist UI** (Studio Precision & Viewfinder Cinema aesthetics), dual-mode playback engine (Viewfinder Video & Hi-Fi Audio Deck), hardware-accelerated media decoding via `libmpv2`, and deep Linux desktop integration via MPRIS2 and SQLite WAL persistence.

---

## Key Features

- **Tactile Digital Brutalism Design Language:** Deep obsidian base (`#070709`, `#0B0B0E`), razor-sharp 1px structural grid lines (`#1E1E24`), high-contrast typography, industrial safety orange (`#FF4400`) playheads, and signal lime (`#CCFF00`) audio levels.
- **Mode 1: Video Viewfinder Mode:**
  - Borderless video viewport with technical safe-area reticles (Action 90%, Title 80%).
  - Telemetry diagnostics OSD HUD (FPS, dropped frames, bitrate, resolution, render time).
  - Mechanical SMPTE timecode tick ruler progress scrubber (`HH:MM:SS:FF`).
  - Tactile bottom control dock with single-frame stepping (`< 1F` / `1F >`), 10s jumps, speed toggles, stream switchers, and fullscreen/PiP toggles.
- **Mode 2: Hi-Fi Audio Deck Mode:**
  - Typographic metadata masthead and embedded album artwork display.
  - Animated stereo peak VU meters (CH_L & CH_R, -60dB to +3dB).
  - 10-band tactile graphic equalizer rack (32Hz to 16kHz) with studio factory presets and flat reset.
  - Synchronized `.lrc` lyrics teleprompter with auto-scroll and click-to-seek.
  - Playlist queue matrix with track reordering, shuffle, and repeat modes.
- **MPRIS2 D-Bus Integration:**
  - Full `org.mpris.MediaPlayer2` and `org.mpris.MediaPlayer2.Player` interface compliance.
  - Native integration with desktop controllers (GNOME, KDE Plasma, playerctl, waybar).
- **Media Library & State Persistence:**
  - SQLite WAL persistence (`penguin.db`) in `$XDG_DATA_HOME`.
  - Automatic saving and restoration of window geometry, volume, equalizer presets, and playlist queue across sessions.
- **Rich Command-Line Interface (CLI):**
  - Launch media files or directories, force audio/video mode, fullscreen, volume, speed, and remote IPC actions.

---

## Build Requirements

- C++17 compiler (`g++` or `clang++`)
- Qt 6 (Core, Gui, Widgets, DBus, Sql, OpenGL, OpenGLWidgets, Test)
- `libmpv2` (`libmpv-dev`)
- `qmake6` or `cmake`

On Ubuntu/Debian/Linux Mint:
```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-base-dev-tools libmpv-dev libmpv2
```

---

## Building and Running

### 1. Build the Binary
```bash
./scripts/build.sh
```
Or directly with `qmake6`:
```bash
qmake6 penguin.pro
make -j$(nproc)
```

### 2. Run Penguin
```bash
# Launch player
./penguin

# Open a video file in Video Viewfinder mode
./penguin /path/to/video.mp4

# Open audio files in Hi-Fi Audio Deck mode
./penguin --audio /path/to/album/*.flac

# Set initial volume and playback speed
./penguin --volume 80 --speed 1.25 /path/to/media.mp4
```

### 3. Run Automated Self-Diagnostics & Tests
```bash
# Headless self-test report
./penguin --test

# Run full master test suite
./scripts/run_tests.sh

# Run C++ unit test suites
./tests/test_m1_core
./tests/test_m2_ui
./tests/test_m3_m4
```

---

## Installation & Desktop Packaging

To install Penguin to `~/.local` (or custom prefix):
```bash
./scripts/package.sh ~/.local
```
This installs:
- `~/.local/bin/penguin` (executable)
- `~/.local/share/applications/penguin.desktop` (FreeDesktop XDG desktop entry)
- `~/.local/share/icons/hicolor/...` (Scalable SVG & PNG icons)

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Space` / `K` | Toggle Play / Pause |
| `Left` / `Right` / `J` / `L` | Seek -10s / +10s |
| `Shift + Left` / `Shift + Right` | Seek -30s / +30s |
| `,` (Comma) / `.` (Period) | Frame Step Backward / Forward (`< 1F` / `1F >`) |
| `Up` / `Down` | Volume Up (+5%) / Down (-5%) |
| `M` | Toggle Mute / Unmute |
| `[` / `]` | Speed Rate -0.1x / +0.1x |
| `Backspace` | Reset Speed to 1.0x |
| `Tab` | Switch UI Mode (Viewfinder Video <-> Hi-Fi Audio Deck) |
| `F` / `F11` | Toggle Fullscreen |
| `O` | Toggle Video Telemetry OSD HUD |
| `R` | Toggle Video Safe-Area Reticles |
| `Ctrl + O` | Open Media File |
| `Ctrl + Shift + O` | Open Media Directory / Folder |
| `Ctrl + Q` | Quit Application |

---

## License

Penguin Media Player is released under the MIT License.
