## 2026-08-31T17:05:18Z

<USER_REQUEST>
You are the Specification Miner for the "Penguin" desktop media player project.
Working directory: /home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey

Input files:
- Read /home/lucifer/Documents/Projects/Penguin/.agents/ORIGINAL_REQUEST.md

Objective:
Extract and document the comprehensive, precise technical requirements and specifications for Penguin media player across:
1. Playback & Engine: Supported container & codec matrix (MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV), seeking accuracy (millisecond & SMPTE HH:MM:SS:FF timecode), frame stepping, playback rate (0.5x - 2.0x), audio track discovery/switching, subtitle track discovery/switching and external subtitle loading (.srt, .ass, .vtt).
2. Tactile Digital Brutalist UI: Exact color palette (#070709, #0B0B0E, #1E1E24, #FF4400, #CCFF00), fonts (monospace JetBrains Mono, Swiss sans), Video Viewfinder mode specifications (safe-area reticles, telemetry OSD with FPS/dropped frames/bitrate/resolution/render time, tick ruler scrubber, control dock), Hi-Fi Audio Deck mode specifications (typographic display, stereo peak VU meters CH_L/CH_R, 10-band equalizer rack 32Hz-16kHz with presets, synchronized .lrc teleprompter, playlist matrix).
3. Linux Desktop & MPRIS2: D-Bus service `org.mpris.MediaPlayer2.penguin`, Root and Player interface methods/properties/signals, media keys, metadata dictionary, .desktop entry, icon, CLI arguments, PipeWire/PulseAudio integration.
4. Media Library & Persistence: File/dir loading, drag-and-drop, playlist queue matrix, persistent config/db schema for history, volume, window geometry.
5. Verification & Test Suite: Unit tests, headless verification script, acceptance criteria matrix.

Scope boundaries:
- Exploration and specification extraction ONLY. Do NOT write production source code.

Output requirements:
- Write your comprehensive specification report to `/home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md`
- Write your handoff report to `/home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/handoff.md`
- Send a completion message when finished.
</USER_REQUEST>
