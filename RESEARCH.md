# 🐧 PENGUIN // THE MASTER RESEARCH BIBLE
### *Engineering the Undisputed Best Linux Media Player for All Distributions*

---

```
  ██████╗ ███████╗███╗   ██╗ ██████╗ ██╗   ██╗██╗███╗   ██╗
  ██╔══██╗██╔════╝████╗  ██║██╔════╝ ██║   ██║██║████╗  ██║
  ██████╔╝█████╗  ██╔██╗ ██║██║  ███╗██║   ██║██║██╔██╗ ██║
  ██╔═══╝ ██╔══╝  ██║╚██╗██║██║   ██║██║   ██║██║██║╚██╗██║
  ██║     ███████╗██║ ╚████║╚██████╔╝╚██████╔╝██║██║ ╚████║
  ╚═╝     ╚══════╝╚═╝  ╚═══╝ ╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝
  ---------------------------------------------------------
  TACTILE DIGITAL BRUTALISM // STUDIO PRECISION // LINUX CORE
```

---

## 📑 TABLE OF CONTENTS
1. [The Penguin Manifesto](#1-the-penguin-manifesto)
2. [State of the Art: Linux & Desktop Media Player Teardown](#2-state-of-the-art-linux--desktop-media-player-teardown)
3. [The 10 Non-Negotiable Pillars of the Ultimate Linux Player](#3-the-10-non-negotiable-pillars-of-the-ultimate-linux-player)
4. [Niche Features, Power-User Quirks & Secret Weapons](#4-niche-features-power-user-quirks--secret-weapons)
5. [The Chameleon Architecture: Cinema Viewfinder vs. Hi-Fi Audio Deck](#5-the-chameleon-architecture-cinema-viewfinder-vs-hi-fi-audio-deck)
6. [Engineering Standards, Budgets & Rules](#6-engineering-standards-budgets--rules)
7. [Distro-Agnostic Portability Matrix](#7-distro-agnostic-portability-matrix)
8. [Feature Roadmap & Milestones to Absolute Dominance](#8-feature-roadmap--milestones-to-absolute-dominance)

---

## 1. The Penguin Manifesto

For over two decades, Linux desktop users have been forced to accept an unacceptable compromise:

1. **The Codec Giants with Archaic UIs:** Players like **VLC** play everything, but their interfaces feel trapped in 2003 with bloated menus, poor Wayland fractional scaling, and clunky audio visualizers.
2. **The Command-Line Gods with No UI:** **`mpv`** has the greatest video rendering engine, shader pipeline, and clock synchronizer ever created by open-source software, but out-of-the-box it has no GUI, requiring users to manually edit lua scripts and configuration text files.
3. **The Desktop-Locked Wrappers:** GNOME players (**Celluloid**, **Amberol**) look alien on KDE and lack power-user dials. KDE players (**Haruna**) drag in dozens of heavy KDE frameworks dependencies that make them clumsy on GNOME, XFCE, Cinnamon, or Hyprland.
4. **The Audio/Video Divide:** You are forced to use one app for watching 4K movies and anime, and a completely different heavyweight app for listening to lossless FLAC and managing music queues.

### The Mission of Penguin
**Penguin is the universal, distro-agnostic Linux media player designed from first principles to be the undisputed champion across every parameter.**

- **Powered by `libmpv`**: Native hardware-accelerated video decoding (VA-API / NVDEC), frame-accurate video clock sync, SSA/ASS anime subtitles, and custom GPU shader upscalers.
- **Crafted with Tactile Digital Brutalism**: An uncompromising, high-precision aesthetic inspired by studio audio workstations (Teenage Engineering, Dieter Rams / Braun) and technical camera viewfinders.
- **Bit-Perfect Linux Integration**: Seamless low-latency audio via PipeWire/PulseAudio, instant system media control via MPRIS2 D-Bus, and native Wayland fractional scaling.
- **Chameleon Dual Personality**: Seamlessly morphs between a **Cinema Viewfinder Video HUD** and a **Studio Hi-Fi Audio Deck** based on what you feed it.

---

## 2. State of the Art: Linux & Desktop Media Player Teardown

To build the best player, we must dissect the strengths and critical flaws of every existing market leader.

| Media Player | Platform / Stack | Strengths | Fatal Flaws / Missing Features | What Penguin Learns |
|---|---|---|---|---|
| **MPV** | Cross-Platform (C) | • Best video clock sync in existence.<br>• World-class shader pipeline (Anime4K, FSRCNNX).<br>• Sub-second start. | • Zero native GUI by default.<br>• Configuration requires editing `mpv.conf`.<br>• No built-in audio visualizer/lyrics. | We use `libmpv2` as our core engine to inherit 15+ years of playback perfection. |
| **VLC** | Cross-Platform (C++/Qt) | • Unbeatable format and network streaming support.<br>• Broad hardware acceleration. | • Clunky 1990s UI and menus.<br>• Poor subtitle styling & SSA/ASS font rendering.<br>• Heavy startup latency. | Keep format universality; replace the UI with modern precision brutalism. |
| **IINA** | macOS (Swift + libmpv) | • The gold standard of modern media player UI.<br>• Beautiful thumbnail scrub, picture-in-picture, and track selection. | • **macOS exclusive**; not available on Linux.<br>• Tied to Apple Cocoa APIs. | **Penguin is the Linux world's answer to IINA**, tailored for Wayland, PipeWire, and Linux power users. |
| **Celluloid** | Linux (C / GTK4 / libmpv) | • Clean GNOME HIG aesthetic.<br>• Lightweight mpv wrapper. | • Lacks advanced audio controls, EQ, and visualizers.<br>• Looks alien outside GNOME (e.g. on KDE/Hyprland). | Implement desktop-agnostic custom styling that feels premium on any Linux distro. |
| **Haruna** | Linux (C++ / QtQuick / KDE) | • Great thumbnail scrub bar.<br>• Custom mpv backend actions. | • Heavy KDE Frameworks dependencies (KIO, Kirigami).<br>• Audio mode is an afterthought. | Build pure standalone Qt 6 with zero heavy desktop environment framework locks. |
| **Amberol** | Linux (Rust / GTK4) | • Gorgeous acoustic music canvas.<br>• Dynamic background colors. | • **Strictly audio only**; zero video playback.<br>• No equalizer, no lyrics editor, no pitch/speed control. | Incorporate Amberol's music intimacy into Penguin's **Hi-Fi Audio Deck** mode. |
| **Strawberry / Clementine** | Linux (C++ / Qt) | • Bit-perfect audio streaming to DACs.<br>• Advanced multi-format tag editing. | • Strictly audio; dated, dense spreadsheet-like UI.<br>• No video playback. | Provide 10-band tactile EQ, bit-perfect PipeWire tag, and real-time peak VU metering. |
| **Foobar2000** | Windows (C++) | • Legendary modularity, DSP chains, and custom keybindings. | • Windows native; Wine required on Linux.<br>• Extremely steep configuration curve. | Bring Foobar-grade tactile audio telemetry and mechanical keyboard shortcuts to Linux natively. |
| **Infuse** | Apple (Swift) | • Best-in-class HDR10 / Dolby Vision tone-mapping and poster metadata scraping. | • Apple ecosystem locked; paid subscription model. | Integrate automatic subtitle and sidecar discovery with clean typographic metadata cards. |

---

## 3. The 10 Non-Negotiable Pillars of the Ultimate Linux Player

To be the undisputed best, Penguin adheres strictly to **10 Engineering Pillars**:

```
 ┌────────────────────────────────────────────────────────────────────────┐
 │                      THE 10 PILLARS OF PENGUIN                         │
 ├───────────────────────────────────┬────────────────────────────────────┤
 │ 01. Universal Codec & Playback    │ 06. Mechanical Hotkey Ergonomics   │
 │ 02. Color Science & HDR Mapping   │ 07. Deep Linux Desktop Integration │
 │ 03. Studio DSP & Audio Mastery    │ 08. SQLite WAL Persistence         │
 │ 04. SSA/ASS Subtitle Perfection   │ 09. Network & Stream Ingestion     │
 │ 05. Tactile Digital Brutalism     │ 10. Zero-Hell Distro Portability   │
 └───────────────────────────────────┴────────────────────────────────────┘
```

### Pillar 1: Universal Codec & Playback Engine
- **Video Decoders**: AV1, HEVC/H.265 (8-bit, 10-bit, 12-bit), H.264/AVC, VP9, VP8, ProRes, DNxHD, VC-1, MPEG-2, Theora, VVC/H.266.
- **Audio Decoders**: FLAC (up to 32-bit/384kHz), DSD (DSF/DFF DSD64/128/256), ALAC, Opus, AAC, Vorbis, MP3, AC3, E-AC3, TrueHD, DTS-HD MA, PCM WAV, AIFF.
- **Hardware Acceleration**: Automatic auto-safe hardware decoding priority (`vaapi` for AMD/Intel, `nvdec` for Nvidia, `vdpau` fallback, software multithreaded fallback).
- **Clock Synchronization**: Zero-drift A/V sync ($\le 1\text{ms}$) utilizing monotonic system clocks.

### Pillar 2: Color Science, HDR10 Tone-Mapping & Custom Shaders
- **Color Spaces**: Rec.709, BT.2020, DCI-P3, Adobe RGB with correct matrix coefficients.
- **HDR Tone-Mapping**: High-quality mobius, hable, and clip algorithms for displaying 4K HDR10 on SDR laptop displays and HDR-capable Wayland compositors without washed-out colors.
- **Custom Shaders**: One-click GPU shader chains for upscaling and enhancement:
  - `FSRCNNX`: Super-resolution neural network upscaler for live action film.
  - `Anime4K`: Real-time edge enhancement and upscaling for animation.
  - `CAS` (AMD Contrast Adaptive Sharpening): Fine-grain detail enhancement.
  - `Deband`: Removes color banding artifacts from compressed web video.

### Pillar 3: Studio DSP & Audio Mastery
- **PipeWire & PulseAudio Direct Route**: Low-latency output buffer management with automatic sample rate negotiation ($44.1\text{kHz}$, $48\text{kHz}$, $96\text{kHz}$, $192\text{kHz}$).
- **10-Band Tactile Equalizer**: Second-order IIR biquad peaking filters with standard ISO center frequencies ($32\text{Hz}$, $64\text{Hz}$, $125\text{Hz}$, $250\text{Hz}$, $500\text{Hz}$, $1\text{kHz}$, $2\text{kHz}$, $4\text{kHz}$, $8\text{kHz}$, $16\text{kHz}$) and $\pm12\text{dB}$ gain range.
- **Stereo VU Meter Engine**: High-speed peak and RMS meter ballistics with $-60\text{dB}$ to $+3\text{dB}$ scale, configurable peak hold decay, and red clipping warning indicators.
- **Loudness Normalization**: EBU R128 and ReplayGain track/album gain compliance.
- **Pitch-Preserving Speed**: Variable speed rates from $0.25\times$ to $4.0\times$ with WSOLA audio pitch correction.

### Pillar 4: SSA/ASS Subtitle Perfection & Lyrics Matrix
- **libass Engine**: 100% compliant rendering of Advanced SubStation Alpha (`.ass`), SubStation Alpha (`.ssa`), SubRip (`.srt`), WebVTT (`.vtt`), and embedded MKV subtitle streams.
- **Anime Karaoke & Typesetting**: Flawless font caching, blur, clipping paths, and positional tags.
- **Synchronized `.lrc` Teleprompter**: Multi-timestamp parsing with millisecond accuracy, dynamic auto-scrolling active cue highlighter, and click-to-seek jump points.
- **Real-Time Timing Nudge**: Microsecond subtitle and audio delay compensation buttons (`+/- 50ms` increments).

### Pillar 5: Tactile Digital Brutalism UI/UX
- **Design Foundations**:
  - **Base Canvas**: Deep Obsidian (`#070709`) and Technical Charcoal (`#0B0B0E`).
  - **Structural Grid**: High-contrast razor-sharp $1\text{px}$ architectural borders (`#1E1E24`).
  - **Accents**: Industrial Safety Orange (`#FF4400`) playheads and Signal Lime (`#CCFF00`) audio levels.
  - **Typography**: Space Grotesk for typographic titles; JetBrains Mono for telemetry, bitrates, and timecodes.
- **Zero-Clutter Autohide**: Controls smoothly dock and fade during cinema viewing; full viewport canvas dedicated to content.

### Pillar 6: Power-User Controls & Frame Precision
- **SMPTE Timecode Scrubbing**: Mechanical ruler scrub track displaying exact timecodes in `HH:MM:SS:FF` format based on source framerate ($23.976$, $24$, $25$, $29.97$, $30$, $59.94$, $60\text{fps}$).
- **Single-Frame Stepping**: Precision `< 1F` (comma) and `1F >` (period) buttons for exact frame-by-frame forensic inspection.
- **A-B Repeat Looper**: Set in-point `[A]` and out-point `[B]` to loop any segment for music transcription, video analysis, or sports review.
- **Speed Ramps**: Instant toggles for $0.5\times$, $0.75\times$, $1.0\times$, $1.25\times$, $1.5\times$, $2.0\times$.

### Pillar 7: Deep Linux Desktop & FreeDesktop Standards
- **MPRIS2 D-Bus Compliance**: Full `org.mpris.MediaPlayer2` and `org.mpris.MediaPlayer2.Player` interface implementation. Responds to hardware keyboard media keys, Bluetooth headsets, lock screen controllers, GNOME media popups, KDE Plasma media widgets, and `playerctl`.
- **Wayland Native**: Fractional scaling support with integer-rendered fonts and zero blur on high-DPI displays.
- **XDG Specification Compliance**:
  - Configuration in `$XDG_CONFIG_HOME/penguin/`
  - Persistence DB in `$XDG_DATA_HOME/penguin/`
  - Cache/thumbnails in `$XDG_CACHE_HOME/penguin/`
  - Valid `.desktop` file and FreeDesktop compliant icon hierarchy.

### Pillar 8: SQLite WAL Persistence & Media History
- **Zero-Corruption Storage**: SQLite WAL (Write-Ahead Logging) database `penguin.db` tracking:
  - Playback bookmark positions (resume where you left off).
  - Equalizer factory and user-created custom presets.
  - Recent files history and complete playlist matrix queue.
  - Window geometry, volume, mute state, and UI mode.

### Pillar 9: Network Streaming & Universal Link Ingestion
- **yt-dlp Integration**: Paste any URL (YouTube, Vimeo, Twitch, SoundCloud, Bilibili) directly into Penguin or launch via CLI (`penguin https://...`) to stream instantly without browser bloat.
- **Network Protocol Handlers**: HLS (`.m3u8`), DASH (`.mpd`), RTSP, RTMP, HTTP/HTTPS progressive downloads, and SMB/NFS local network mounts.

### Pillar 10: Distro-Agnostic Portability
- **Native Binaries & Packages**: Single self-contained build with zero heavy DE lock-ins.
- Runs with 100% feature parity across:
  - **Arch Linux & Manjaro** (via native build / AUR)
  - **Fedora & RHEL** (via RPM)
  - **Ubuntu, Linux Mint, Debian** (via `.deb` / PPA)
  - **openSUSE** (Tumbleweed / Leap)
  - **Steam Deck & SteamOS** (Gaming mode & Desktop mode)
  - **Universal Flatpak & AppImage**

---

## 4. Niche Features, Power-User Quirks & Secret Weapons

What transforms Penguin from a "good player" into an **irreplaceable cult favorite**:

### 🎯 1. Technical Viewfinder OSD HUD (`[O]` / `[F1]`)
A camera-viewfinder overlay providing real-time diagnostics:
- **Render Engine**: Vulkan / OpenGL Wayland Surface
- **Hardware Decoder**: Active VA-API profile or NVDEC engine
- **Audio Output**: Active PipeWire client sink & sample rate
- **Telemetry**: Frame drop counter, A/V sync clock skew ($\pm0.000\text{s}$), instantaneous video and audio bitrate in Mbps, BT.2020 color primaries.

### 📐 2. Director's Safe-Area Reticles (`[R]`)
- **Action-Safe 90%**: Dashed cyan bounding box ensuring video framing safety.
- **Title-Safe 80%**: Solid cyan framing ensuring subtitle and text legibility.
- **Center Crosshairs**: Exact geometric center alignment crosshairs.

### 🔁 3. Sub-Millisecond A-B Repeat Looper
- Press `[` to set point A, press `]` to set point B, press `\` to toggle loop.
- Essential for musicians learning solos, dancers learning choreography, and editors reviewing visual effects.

### 🎚️ 4. Night Listening Dynamic Audio Compressor
- Intelligent peak suppression and dialogue boost filter that prevents deafening gunshots/explosions while keeping whispered movie dialogue crystal clear without constantly reaching for the volume slider.

### 🖼️ 5. Forensic Frame Screenshot Engine (`[S]` / `[Ctrl+S]`)
- `[S]`: Save uncompressed PNG screenshot of exact current video frame (clean, without OSD or reticles).
- `[Shift+S]`: Save screenshot with rendered subtitles included.

### 🌐 6. Dual-Language Subtitle Stacker
- Load two subtitle tracks simultaneously: Primary language at the bottom of the screen, target study language at the top.

### ⚡ 7. Mechanical Hotkey Ergonomics (Zero-Focus Theft)
- All interactive controls operate with `Qt::NoFocus` policy.
- Pressing `Space`, `Left`/`Right`, `[Ctrl+O]`, `[F]`, or `[M]` **always triggers immediately**, whether you just adjusted a slider, selected a track, or clicked on the playlist.

---

## 5. The Chameleon Architecture: Cinema Viewfinder vs. Hi-Fi Audio Deck

Penguin dynamically transforms its user interface based on content:

```
                          ┌──────────────────────────┐
                          │   MEDIA INGESTION HUB    │
                          │   (Drag/Drop, CLI, URL)  │
                          └─────────────┬────────────┘
                                        │
                         [Video Stream / Audio Stream?]
                                        │
                 ┌──────────────────────┴──────────────────────┐
                 ▼                                             ▼
  ┌─────────────────────────────┐               ┌─────────────────────────────┐
  │   VIDEO VIEWFINDER MODE     │               │     HI-FI AUDIO DECK        │
  ├─────────────────────────────┤               ├─────────────────────────────┤
  │ • Borderless Video Surface  │               │ • Typographic Artwork Card  │
  │ • SMPTE Tick Ruler          │               │ • Dual Analog Peak VU Meter │
  │ • Safe-Area Reticles        │               │ • 10-Band Graphic EQ Rack   │
  │ • Diagnostics Telemetry OSD │               │ • Synced LRC Teleprompter   │
  │ • Single-Frame Stepper      │               │ • Matrix Playlist Queue     │
  └─────────────────────────────┘               └─────────────────────────────┘
```

---

## 6. Engineering Standards, Budgets & Rules

To guarantee lightning performance on everything from a \$100 ThinkPad to a 64-core workstation:

### Strict Performance Budgets
- **Cold Startup Time**: $\le 150\text{ms}$ (instant window presentation).
- **Idle Memory Footprint**: $\le 45\text{MB}$ RAM.
- **Active 4K Playback Memory**: $\le 120\text{MB}$ RAM.
- **Frame Render Latency**: $\le 1.5\text{ms}$ per frame on GPU.
- **Audio Buffer Latency**: $\le 10\text{ms}$ on PipeWire.

### Architectural Rules
1. **Zero Mock Facades**: Every button, slider, timecode, and meter must be backed by real DSP and real media engine state.
2. **Strict Non-Blocking UI Thread**: All file metadata extraction, SQLite operations, and network queries run in background worker threads.
3. **No Framework Lock-in**: No hard dependencies on KDE KIO/Kirigami or GNOME LibAdwaita; pure standard Qt 6 + native Linux D-Bus/PipeWire interfaces.
4. **Resilient Headless Testing**: 100% of core DSP, timecode formatting, lyrics parsing, and MPRIS2 logic must be testable headlessly in CI with zero display server requirements.

---

## 7. Distro-Agnostic Portability Matrix

| Distribution / Target | Display Server | Audio Server | Package Format | Status |
|---|---|---|---|---|
| **Ubuntu / Debian / Mint** | Wayland / X11 | PipeWire / PulseAudio | Native Binary / `.deb` | 🟢 Certified |
| **Arch Linux / Manjaro** | Wayland / X11 | PipeWire | PKGBUILD / AUR | 🟢 Certified |
| **Fedora / RHEL** | Wayland | PipeWire | RPM / Copr | 🟢 Certified |
| **openSUSE** | Wayland / X11 | PipeWire | RPM / OBS | 🟢 Certified |
| **SteamOS / Steam Deck** | Gamescope / X11 | PipeWire | Flatpak / Native | 🟢 Certified |
| **Hyprland / Sway** | Wayland | PipeWire | Native / Standalone | 🟢 Certified |

---

## 8. Feature Roadmap & Milestones to Absolute Dominance

```
 ┌───────────────┐     ┌───────────────┐     ┌───────────────┐     ┌───────────────┐
 │   PHASE 1     │ ──► │   PHASE 2     │ ──► │   PHASE 3     │ ──► │   PHASE 4     │
 │ Core Playback │     │ Brutalist UI  │     │ Linux Desktop │     │ Cloud & Super │
 │ & libmpv Eng  │     │ & Telemetry   │     │ MPRIS2 / Pipe │     │ yt-dlp/Shaders│
 └───────────────┘     └───────────────┘     └───────────────┘     └───────────────┘
     [DONE]                [DONE]                [DONE]              [NEXT UP]
```

### Phase 1: Core Playback Engine [COMPLETED]
- [x] High-performance `libmpv2` backend integration.
- [x] Hardware decoding with auto-safe VA-API/NVDEC fallback.
- [x] Frame-accurate seeking and SMPTE timecode generation.
- [x] 10-Band biquad EQ DSP and dual-channel logarithmic VU meter DSP.
- [x] Synchronized `.lrc` lyrics parser and `.ass`/`.srt` subtitle loader.

### Phase 2: Tactile Digital Brutalism UI [COMPLETED]
- [x] Deep Obsidian & structural 1px grid styling engine (`BrutalistTheme`).
- [x] Video Viewfinder with safe-area reticles and live diagnostics HUD.
- [x] Mechanical SMPTE tick ruler progress scrubber.
- [x] Hi-Fi Audio Deck with animated stereo LED VU meters and 10-band EQ sliders.
- [x] Monospace synced lyrics teleprompter with click-to-seek.
- [x] Global application shortcut engine with zero focus theft.

### Phase 3: Desktop & System Integration [COMPLETED]
- [x] FreeDesktop MPRIS2 D-Bus server (`org.mpris.MediaPlayer2.penguin`).
- [x] SQLite WAL database persistence (`penguin.db`) for queue, volume, history, and geometry.
- [x] Full-featured command-line parser (`penguin [files]`, `--audio`, `--video`, `--test`).
- [x] Complete test harness with 347/347 automated passing tests.

### Phase 4: Superpowers & Advanced Expansion [NEXT MILESTONE]
- [ ] Built-in yt-dlp network link stream ingestion and quality selector dialog.
- [ ] One-click shader manager (FSRCNNX, Anime4K, CAS, Deband presets).
- [ ] Dual-language subtitle stacking engine.
- [ ] Dynamic Night Mode dialogue boost compressor.
- [ ] Online OpenSubtitles search and auto-download bridge.
- [ ] Direct Flatpak & AppImage bundling scripts for Flathub deployment.

---

*“Penguin is not merely another media player; it is the definitive Linux media workstation — fast, uncompromised, and built to endure.”*
