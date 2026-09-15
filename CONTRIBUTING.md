# Contributing to Penguin

Thank you for your interest in contributing to **Penguin**! Penguin is built on the philosophy of **Tactile Digital Brutalism & Studio Precision** — uncompromising engineering, sub-millisecond precision, and industrial design.

---

## 🏗️ Architecture Guidelines

- **Language Standard**: C++20 / Qt 6.4+
- **Media Engine**: `libmpv2` (`libmpv-dev`) with hardware acceleration (`vaapi`, `nvdec`, `gpu-next`).
- **Audio DSP**: Biquad IIR filters, EBU R128 loudness normalization, and BS2B crossfeed.
- **Desktop Standards**: FreeDesktop MPRIS2 D-Bus interfaces, XDG Base Directory specification, and SQLite WAL persistence.

---

## 🛠️ Development Setup

### 1. Install Prerequisites (Debian/Ubuntu/Mint)
```bash
sudo apt update
sudo apt install -y build-essential qt6-base-dev qt6-base-dev-tools \
    libgl1-mesa-dev libmpv-dev libmpv2 ffmpeg libsqlite3-dev dbus-x11
```

### 2. Build from Source
```bash
qmake6 penguin.pro
make -j$(nproc)
```

### 3. Run Headless Self-Diagnostics
```bash
./penguin --test
```

### 4. Run the Full Test Harness (432+ Tests)
```bash
./scripts/run_tests.sh
python3 run_tests.py
```

---

## 🎨 UI & Design Principles: Tactile Digital Brutalism

When designing or touching UI components, follow our core design tokens:
- **Base Obsidian**: `#070709` (canvas), `#0B0B0E` (panels), `#121217` (docks)
- **Structural Grid**: 1px sharp borders using `#1E1E24` and `#2A2A35`
- **Safety Orange (`#FF4400`)**: Active playheads, seek markers, primary transport indicators
- **Signal Lime (`#CCFF00`)**: Nominal audio levels, synchronized lyric teleprompter highlights
- **Telemetry Cyan (`#00E5FF`)**: Viewfinder safe-area reticles, SMPTE timecode readout, HUD telemetry
- **Monospace Readouts**: Numerical figures must use tabular monospace (`JetBrains Mono`, `Fira Code`, or native monospace) to guarantee zero jitter during playback.

---

## 📬 Submitting Changes

1. Fork the repository and create a feature branch (`git checkout -b feat/your-feature`).
2. Adhere to **Conventional Commits** format (`feat:`, `fix:`, `refactor:`, `docs:`, `test:`).
3. Ensure all tests pass (`python3 run_tests.py`) with 0 failures before opening a pull request.
4. Open a Pull Request detailing your changes, context, and verification output.
