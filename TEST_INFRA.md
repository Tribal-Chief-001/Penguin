# E2E Test Infra: Penguin Media Player

## Test Philosophy
- Opaque-box, requirement-driven testing independent of internal implementation hacks.
- Zero external network dependencies (deterministic synthetic test media generation via ffmpeg/wave).
- Full headless CI testability using `QT_QPA_PLATFORM=offscreen` and `dbus-run-session`.
- Multi-tier methodology: Category-Partition (Tier 1) + Boundary Value Analysis (Tier 2) + Pairwise Combinatorial (Tier 3) + Real-World Workloads (Tier 4) + Adversarial White-Box Hardening (Tier 5).

## Feature Inventory
| # | Feature | Source | Tier 1 (Count) | Tier 2 (Count) | Tier 3 (Pairwise) | Tier 4 (Real-World) |
|---|---------|--------|:--------------:|:--------------:|:-----------------:|:-------------------:|
| F01 | Multi-format Media Playback (MP4/MKV/WebM/AVI/MP3/FLAC/Opus/AAC/WAV) | ORIGINAL_REQUEST §R1 | 5 | 5 | ✓ | ✓ |
| F02 | Millisecond & SMPTE Seeking Accuracy | ORIGINAL_REQUEST §R1 | 5 | 5 | ✓ | ✓ |
| F03 | Frame Stepping & Jumps (< 1F / 1F >, +/- 10s) | ORIGINAL_REQUEST §R1 | 5 | 5 | ✓ | ✓ |
| F04 | Speed Control & Pitch Correction (0.5x - 2.0x) | ORIGINAL_REQUEST §R1 | 5 | 5 | ✓ | ✓ |
| F05 | Audio/Subtitle Track Discovery & External Subs (.srt, .ass, .vtt) | ORIGINAL_REQUEST §R1 | 5 | 5 | ✓ | ✓ |
| F06 | 10-Band Graphic Equalizer DSP & Presets | ORIGINAL_REQUEST §R1, §R2 | 5 | 5 | ✓ | ✓ |
| F07 | Stereo Peak VU Meter DSP (-60dB to +3dB) | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F08 | Tactile Brutalist Design System & Theme | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F09 | Video Viewfinder Mode (Reticles & Telemetry HUD) | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F10 | Mechanical Tick Ruler Scrubber | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F11 | Tactile Video Control Dock | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F12 | Hi-Fi Audio Deck Mode | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F13 | Synchronized LRC Teleprompter & Seek Sync | ORIGINAL_REQUEST §R2 | 5 | 5 | ✓ | ✓ |
| F14 | Playlist Queue Matrix & Reordering | ORIGINAL_REQUEST §R2, §R4 | 5 | 5 | ✓ | ✓ |
| F15 | MPRIS2 D-Bus Interface (Root & Player) | ORIGINAL_REQUEST §R3 | 5 | 5 | ✓ | ✓ |
| F16 | Desktop Packaging, XDG Entry & SVG Icon | ORIGINAL_REQUEST §R3 | 5 | 5 | ✓ | ✓ |
| F17 | CLI Arguments & Mode Routing | ORIGINAL_REQUEST §R3 | 5 | 5 | ✓ | ✓ |
| F18 | Audio System Routing (PipeWire/PulseAudio) | ORIGINAL_REQUEST §R3 | 5 | 5 | ✓ | ✓ |
| F19 | File & Recursive Directory Loading | ORIGINAL_REQUEST §R4 | 5 | 5 | ✓ | ✓ |
| F20 | State & History Persistence (SQLite WAL) | ORIGINAL_REQUEST §R4 | 5 | 5 | ✓ | ✓ |
| F21 | Synthetic Test Media Generator | ORIGINAL_REQUEST §R5 | 5 | 5 | ✓ | ✓ |
| F22 | Comprehensive Automated Unit Tests | ORIGINAL_REQUEST §R5 | 5 | 5 | ✓ | ✓ |
| F23 | Headless Verification Runner | ORIGINAL_REQUEST §R5 | 5 | 5 | ✓ | ✓ |

## Test Architecture
- **Test Runner Location**: `scripts/run_tests.sh` and `tests/test_e2e_runner.py` / `./penguin --test`
- **Invocation**:
  ```bash
  QT_QPA_PLATFORM=offscreen dbus-run-session python3 tests/test_e2e_runner.py
  ```
- **Pass/Fail Semantics**: Clean exit 0 on all tests passing, non-zero on any failure.
- **Directory Layout**:
  - `tests/fixtures/`: Synthetic generated multi-track audio/video/subtitles and .lrc files.
  - `tests/unit/`: Component-level unit test suites.
  - `tests/e2e/`: End-to-end integration and CLI test cases.

## Real-World Application Scenarios (Tier 4)
| # | Scenario | Features Exercised | Complexity |
|---|----------|--------------------|------------|
| 1 | Full Cinema Playback Workflow: Open 4K MKV, switch audio to Japanese, load external ASS subtitle, toggle Viewfinder telemetry OSD, seek by SMPTE timecode, step 5 frames backward. | F01, F02, F03, F05, F08, F09, F10, F11 | High |
| 2 | Audiophile Hi-Fi Session: Queue 5 FLAC tracks, engage Rock EQ preset (+4.5dB 64Hz, +3.0dB 16kHz), verify real-time VU meter response, follow synchronized .lrc lyrics, reorder queue. | F01, F06, F07, F08, F12, F13, F14 | High |
| 3 | Linux Desktop MPRIS2 Remote Control: Launch media from CLI, control playback via D-Bus player interface (Next/Previous/Pause/Seek), verify metadata broadcast (`xesam:title`, `xesam:artist`). | F01, F15, F16, F17, F18 | High |
| 4 | State Recovery & Session Persistence: Play track to 45s with custom volume (78%) and EQ setting, close application, restart, verify history resume, volume restoration, and window geometry. | F01, F06, F19, F20 | Medium |
| 5 | Video Study / Sports Analysis Workflow: Load 60fps MP4, set speed to 0.5x, frame-step frame-by-frame through action sequence, verify exact timecode accuracy and 0 audio/video desync. | F01, F02, F03, F04, F09, F10 | Medium |

## Coverage Thresholds
- **Tier 1 (Feature Coverage)**: ≥5 test cases per feature (23 × 5 = 115 test cases).
- **Tier 2 (Boundary & Corner Cases)**: ≥5 test cases per feature (23 × 5 = 115 test cases).
- **Tier 3 (Cross-Feature Combinations)**: ≥23 pairwise interaction tests.
- **Tier 4 (Real-World Scenarios)**: ≥12 real-world workload scenarios.
- **Total Minimum Threshold**: >265 test cases.
