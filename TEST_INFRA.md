# E2E Test Infra: Penguin Desktop Media Player

## 1. Test Philosophy & Principles
- **Opaque-Box & Requirement-Driven**: Tests derive directly from `ORIGINAL_REQUEST.md` (R1-R5) and user-facing specifications, exercising the public interfaces, CLI, MPRIS2 D-Bus, and engine pipelines without depending on internal implementation details.
- **Complete Feature Coverage**: Every feature inventoried in `PROJECT.md § Feature Inventory` is mapped to tests across all four tiers.
- **Progressive Testability**: Verification harnesses use standard synthetic media generators (via ffmpeg or raw PCM/WAV/MP4 synthesis) and headless D-Bus sessions (`dbus-run-session`) to test with zero display/sound hardware dependencies.
- **Zero-Tolerance Integrity**: Hardcoded mock bypasses and fake facades are strictly prohibited and verified by forensic auditors.

---

## 2. Feature Inventory Mapping

| # | Feature | Requirement Source | Tier 1 (Count) | Tier 2 (Count) | Tier 3 | Tier 4 |
|---|---|---|:---:|:---:|:---:|:---:|
| 1 | libmpv FFI Core Binding | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 2 | Multi-Format A/V Playback (MP4, MKV, MP3, FLAC, WAV) | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 3 | Transport & Exact ms Seeking | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 4 | SMPTE 12M Timecode Calculation (NDF & 29.97 DF) | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 5 | Bidirectional Frame Stepping (< 1F / 1F >) | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 6 | Pitch-Preserved Speed Scaling (0.5x to 2.0x) | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 7 | Dynamic Track Introspection & Audio/Sub Switching | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 8 | External Subtitle Loading (.srt, .ass, .vtt) | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 9 | A-B Repeat Looping | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 10 | Dynamic Night Mode Dialogue Compressor | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 11 | Deband & Video Dithering | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 12 | Lossless Forensic Screenshot Export | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 13 | Network Stream URL Ingestion | R1 § Engine | ≥5 | ≥5 | ✓ | ✓ |
| 14 | Tactile Brutalist Design Tokens & Geometry | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 15 | Video Viewfinder Mode (Reticles & OSD HUD) | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 16 | Hi-Fi Audio Deck Mode (Typographic Masthead) | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 17 | Stereo Peak VU Meters (CH_L & CH_R Ballistics) | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 18 | 10-Band Graphic Equalizer Rack & Presets | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 19 | Synchronized LRC Teleprompter & Word Tokens | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 20 | Playlist Queue Matrix & Search Filter | R2 § UI | ≥5 | ≥5 | ✓ | ✓ |
| 21 | MPRIS2 D-Bus Root & Player Interfaces | R3 § Desktop | ≥5 | ≥5 | ✓ | ✓ |
| 22 | Single-Instance IPC & CLI Arguments | R3 § Desktop | ≥5 | ≥5 | ✓ | ✓ |
| 23 | Audio Routing (PipeWire/PulseAudio) | R3 § Desktop | ≥5 | ≥5 | ✓ | ✓ |
| 24 | Desktop Packaging (.desktop, SVG/PNG Icons) | R3 § Desktop | ≥5 | ≥5 | ✓ | ✓ |
| 25 | SQLite WAL Database & Schema Migrations | R4 § Persistence | ≥5 | ≥5 | ✓ | ✓ |
| 26 | History & Playlist Queue State Persistence | R4 § Persistence | ≥5 | ≥5 | ✓ | ✓ |
| 27 | Window Geometry & DSP State Restore | R4 § Persistence | ≥5 | ≥5 | ✓ | ✓ |

---

## 3. Test Methodology & Tier Breakdown

### Tier 1 — Category-Partition Feature Tests (≥135 tests)
- Verifies every feature in isolation under nominal happy-path inputs:
  - Formats: MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV media loading and duration verification.
  - Transport: Play, Pause, Stop, Seek, Speed, Volume, Mute state changes.
  - Timecodes: NDF 24, 25, 30, 50, 60 fps standard conversions.
  - DSP: 10 individual equalizer bands, 10 factory presets, flat reset.
  - Lyrics: Standard `.lrc` centisecond parsing, metadata tags (`ti`, `ar`, `al`, `offset`).
  - MPRIS2: Calling `Play()`, `Pause()`, `Next()`, `Previous()`, `Seek()`, property inspection.
  - Persistence: Record insertion, schema verification, table indexing.

### Tier 2 — Boundary & Corner Cases (≥135 tests)
- Verifies edge cases, extremes, and error handling:
  - SMPTE: 0ms timestamp, negative milliseconds clamping, 29.97 DF 1st minute skip (`00:01:00;02`), 10th minute exception (`00:10:00;00`), 100+ hour timestamps.
  - LRC: Out-of-order timestamps, negative global offsets, unclosed brackets, word-level A2 tags (`<mm:ss.xx>`), instrumental pauses.
  - Audio DSP: EQ gains clamped at ±12dB / ±15dB limits, extreme frequency biquad stability, audio clipping at >0dBFS.
  - Subtitles: Malformed subtitle files, UTF-8 BOM, missing track IDs, zero-length files.
  - MPRIS2: `SetPosition` with invalid `TrackId`, rapid seek signal throttling, D-Bus session disconnect resilience.
  - Persistence: Malformed database recovery, WAL checkpointing under rapid writes, foreign key constraints.

### Tier 3 — Cross-Feature Combinations & State Transitions (≥27 tests)
- Verifies feature interactions and state matrix coverage:
  - Speed scaling (2.0x) + 10-Band EQ + Night Mode compression concurrently.
  - Mode switching (Video Viewfinder ↔ Audio Deck) during active playback without audio/video dropout.
  - A-B repeat looping while adjusting speed and volume.
  - MPRIS2 remote commands modifying active UI state and database history simultaneously.
  - Drag-and-drop playlist reordering while actively seeking.

### Tier 4 — Real-World Workload Scenarios (≥14 tests)
- Simulates realistic end-user media playback workflows:
  - Scenario 1: Feature-length 4K movie playback with subtitle switching, audio track selection, chapter navigation, and forensic screenshot export.
  - Scenario 2: High-res FLAC album listening session with 10-band EQ preset tuning, real-time VU meter dynamics, and synchronized `.lrc` lyric teleprompter scrolling.
  - Scenario 3: Continuous audio queue playback with MPRIS2 desktop media key control and SQLite history updates.
  - Scenario 4: Fast-forward / slow-motion forensic frame analysis with bidirectional frame stepping and SMPTE timecode readout.

### Tier 5 — Adversarial Hardening (White-box, ≥20 tests)
- Stress-tests concurrency, resource exhaustion, memory leaks, rapid state spamming, invalid media headers, and database lock contention.

---

## 4. Test Runner & CLI Architecture
- Main Test Runner: `run_tests.py`
  - Command: `python3 run_tests.py` or `pytest -v tests/`
  - Supports flags: `--tier 1`, `--tier 2`, `--tier 3`, `--tier 4`, `--tier 5`, `--headless`, `--verbose`.
  - Automatically wraps tests in `dbus-run-session` when testing D-Bus MPRIS2 interfaces in headless environments.
  - Guaranteed zero external GUI window requirements under `QT_QPA_PLATFORM=offscreen`.
