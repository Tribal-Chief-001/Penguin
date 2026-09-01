# Project Execution Plan: Penguin Desktop Media Player

## 1. Objectives & Quality Standards
- Deliver Penguin: high-performance Linux desktop media player with Tactile Digital Brutalist UI (Video Viewfinder & Hi-Fi Audio Deck modes).
- Full compliance with Requirements R1-R5 and all acceptance criteria.
- 100% genuine implementation (Zero tolerance for stubbing, mocks in production code, or hardcoded test values).
- Dual track: Implementation Track + Opaque-box E2E Testing Track.

## 2. Execution Phases

### Phase 0: Survey & Technical Exploration
- Spawn 3 parallel survey agents (Spec Miner / Explorers) to analyze system capabilities, available multimedia frameworks (e.g. PySide6 / PyQt6 / GStreamer / libmpv / ffmpeg), D-Bus bindings, audio equalizer DSP techniques, SMPTE timecode handling, and UI layout.
- Aggregate survey reports and synthesize `PROJECT.md` and `TEST_INFRA.md`.

### Phase 1: Architecture & Milestone Decomposition
- Define module boundaries, data flow, and interface contracts.
- Establish milestones:
  - M1: Media Playback Core & Audio DSP Engine (Format playback, accurate seeking, frame stepping, speed, audio/subtitle track switching, 10-band graphic equalizer, peak VU calculation, SMPTE timecode).
  - M2: Tactile Digital Brutalist UI (Video Viewfinder Mode, Hi-Fi Audio Deck Mode, OSD diagnostics, tick scrubber, teleprompter lyrics sync, peak VU meters, equalizer sliders, brutalist styling).
  - M3: Linux Desktop Integration, MPRIS2 & CLI (D-Bus `org.mpris.MediaPlayer2.penguin`, desktop entry, scalable icon, CLI arg parsing, keyboard shortcuts).
  - M4: Media Library, File I/O & State Persistence (File/Folder opening, drag-and-drop, playlist matrix, SQLite/JSON state persistence for history/volume/window geometry).
  - M5: E2E Test Suite & Test Runner (Tiers 1-4 opaque-box tests, headless test runner script, programmatic test mode).
- Establish E2E Testing Track concurrently.

### Phase 2: Implementation & Iteration Loops
- For each milestone:
  - Explorer -> Worker -> Reviewers (2) -> Challengers (2) -> Forensic Auditor -> Gate.
  - Strict AND gate: passes only if clean build, passing tests, 2 APPROVE reviews, 2 challenger confirmations, and CLEAN forensic audit.

### Phase 3: Final Integration & E2E Validation
- Pass 100% of E2E tests (Tiers 1-4).
- Phase 2 Adversarial Hardening (Tier 5 white-box challenger loop).
- Final forensic audit across entire codebase.

### Phase 4: Verification, Documentation & Completion Handoff
- Verify README.md, build/run instructions, .desktop packaging, CLI capabilities.
- Generate final status report for user.
