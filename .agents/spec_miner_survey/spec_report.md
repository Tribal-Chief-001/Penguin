# Penguin Desktop Media Player — Comprehensive Technical Specification Report

**Document Version**: 1.0.0  
**Status**: Authoritative Technical Specification  
**Integrity Mode**: Production / Development  
**Target Platform**: Linux Desktop (X11 / Wayland, PipeWire / PulseAudio, D-Bus, Qt 6 C++)  

---

## 1. Executive Summary & Architecture Overview

### 1.1 Mission & Architectural Vision
**Penguin** is an advanced, universal Linux desktop media player engineered with a **Tactile Digital Brutalist UI** (Studio Precision & Viewfinder Cinema aesthetics) and powered by a dual-mode playback engine:
1. **Video Viewfinder Mode**: Precision broadcast-style monitor with hardware safe-area reticles, telemetry diagnostics HUD (FPS, dropped frames, bitrate, resolution, render time), mechanical SMPTE timecode ruler scrubber, and stream selector docks.
2. **Hi-Fi Audio Deck Mode**: Typographic metadata masthead, responsive stereo peak VU meters (CH_L & CH_R), 10-band tactile graphic equalizer rack (32Hz–16kHz), real-time synchronized `.lrc` teleprompter, and playlist queue matrix.

### 1.2 High-Level Component Architecture

```
+-----------------------------------------------------------------------------------+
|                                  PENGUIN ARCHITECTURE                             |
+-----------------------------------------------------------------------------------+
|                                                                                   |
|  +-----------------------------------+     +-----------------------------------+  |
|  |       CLI & Argument Parser       |     |        Desktop & Window Layer     |  |
|  |  (CLI Flags, URIs, --test mode)   |     |  (Qt6 QMainWindow, QOpenGLWidget) |  |
|  +-----------------+-----------------+     +-----------------+-----------------+  |
|                    |                                         |                    |
|  +-----------------v-----------------------------------------v-----------------+  |
|  |                         TACTILE DIGITAL BRUTALIST UI                        |  |
|  | +------------------------------------+   +--------------------------------+ |  |
|  | |      Video Viewfinder Mode         |   |       Hi-Fi Audio Deck Mode    | |  |
|  | |  - Safe-Area Reticle Overlays      |   |  - Typographic Metadata Board  | |  |
|  | |  - Telemetry Diagnostics HUD       |   |  - Stereo Peak VU Meters       | |  |
|  | |  - SMPTE Tick Ruler Scrubber       |   |  - 10-Band Graphic Equalizer   | |  |
|  | |  - Control Dock & Stream Selectors |   |  - Sync .lrc Teleprompter      | |  |
|  | +------------------------------------+   +--------------------------------+ |  |
|  +-----------------+-----------------------------------------+-----------------+  |
|                    |                                         |                    |
|  +-----------------v-----------------------------------------v-----------------+  |
|  |                               CONTROLLER LAYER                              |  |
|  |   PlaybackController   |   PlaylistManager   |   StatePersistenceController |  |
|  +-----------------+--------------------+--------------------+-----------------+  |
|                    |                    |                    |                    |
|  +-----------------v-----------------+  |  +-----------------v-----------------+  |
|  |       MPRIS2 D-Bus Service        |  |  |      SQLite & Config Store        |  |
|  |   org.mpris.MediaPlayer2.penguin  |  |  |  ($XDG_CONFIG_HOME / config.json) |  |
|  |   (Root & Player Interfaces)      |  |  |  ($XDG_DATA_HOME / history.db)    |  |
|  +-----------------+-----------------+  |  +-----------------------------------+  |
|                    |                    |                                         |
|  +-----------------v--------------------v--------------------------------------+  |
|  |                         CORE PLAYBACK & DSP ENGINE                          |  |
|  |   - Multi-format Demuxer/Decoder Pipeline (libmpv2 / FFmpeg AVFormat/Codec)  |  |
|  |   - Video Surface Renderer (OpenGL / QOpenGLWidget hardware blit)           |  |
|  |   - 10-Band Biquad IIR Equalizer Filter Rack (32 Hz - 16 kHz)               |  |
|  |   - Stereo Peak & RMS Audio Level Analyzer (CH_L & CH_R)                    |  |
|  |   - Frame Stepping & SMPTE Timecode Engine (HH:MM:SS:FF math)               |  |
|  |   - Subtitle Pipeline (SRT, ASS/SSA, VTT) & Audio Stream Switcher           |  |
|  |   - Audio Sink Integration (PipeWire / PulseAudio / ALSA)                   |  |
|  +-----------------------------------------------------------------------------+  |
+-----------------------------------------------------------------------------------+
```

---

## 2. Playback & Core Engine Specifications

### 2.1 Container & Codec Compatibility Matrix

The media engine must support the following container formats, video codecs, audio codecs, and subtitle formats:

| Format Category | Format / Codec | MIME Type / File Extension | Specification / Implementation Details |
|---|---|---|---|
| **Video Container** | MP4 / M4V / MOV | `video/mp4`, `video/quicktime` (.mp4, .m4v, .mov) | ISO/IEC 14496-14 / QuickTime File Format. Supports multi-audio & multi-subtitle track parsing. |
| **Video Container** | Matroska (MKV) | `video/x-matroska` (.mkv) | Matroska v1–v4. Supports embedded SSA/ASS, VobSub, PGS subtitles, multi-track audio. |
| **Video Container** | WebM | `video/webm` (.webm) | Open WebM container (VP8/VP9/AV1 video, Opus/Vorbis audio, WebVTT subtitles). |
| **Video Container** | AVI | `video/x-msvideo` (.avi) | RIFF AVI 1.0 and OpenDML 2.0. Supports interleaved audio/video streams. |
| **Audio Container** | MP3 | `audio/mpeg` (.mp3) | MPEG-1/2 Audio Layer III. Full ID3v1, ID3v2.3, ID3v2.4 tag parsing and embedded cover art extraction. |
| **Audio Container** | FLAC | `audio/flac` (.flac) | Free Lossless Audio Codec (16-bit, 24-bit, 32-bit up to 192kHz). Vorbis Comment metadata. |
| **Audio Container** | Opus | `audio/ogg`, `audio/opus` (.opus, .ogg) | Ogg Opus encapsulated audio. Dynamic sample rate conversion (48kHz internal clock). |
| **Audio Container** | AAC | `audio/aac`, `audio/mp4` (.aac, .m4a) | Advanced Audio Coding (ADTS / MPEG-4 Audio). AAC-LC, HE-AAC v1/v2. |
| **Audio Container** | WAV | `audio/wav`, `audio/x-wav` (.wav) | RIFF WAVE (PCM 8/16/24/32-bit integer, 32/64-bit IEEE float). |
| **Video Codec** | H.264 / AVC | Codec ID: `h264`, `avc1` | Baseline, Main, High, High 10 profiles up to 4K@60fps. Hardware acceleration via VA-API / VDPAU. |
| **Video Codec** | H.265 / HEVC | Codec ID: `hevc`, `hvc1` | Main, Main 10 profiles up to 8K@60fps. HDR10 / BT.2020 color metadata pass-through. |
| **Video Codec** | VP8 / VP9 | Codec ID: `vp8`, `vp9` | Google VP8 and VP9 Profile 0 / Profile 2 (10-bit HDR). |
| **Video Codec** | AV1 | Codec ID: `av1` | AOMedia Video 1 Main Profile. |
| **Video Codec** | MPEG-4 / DivX / XviD | Codec ID: `mpeg4` | MPEG-4 Part 2 Simple & Advanced Simple Profile. |
| **Audio Codec** | PCM | Codec ID: `pcm_s16le`, `pcm_s24le`, `pcm_f32le` | Raw uncompressed PCM stereo / multichannel. |
| **Audio Codec** | Vorbis | Codec ID: `vorbis` | Ogg Vorbis audio. |
| **Audio Codec** | ALAC | Codec ID: `alac` | Apple Lossless Audio Codec (16/24-bit). |
| **Audio Codec** | AC-3 / E-AC-3 | Codec ID: `ac3`, `eac3` | Dolby Digital / Dolby Digital Plus stereo / 5.1 downmixing. |
| **Subtitles** | SRT | `application/x-subrip` (.srt) | SubRip text formatting, UTF-8/UTF-16/Latin1 auto-detection, HTML color/italic tags. |
| **Subtitles** | ASS / SSA | `text/x-ssa` (.ass, .ssa) | Advanced SubStation Alpha v4.00+ script formatting, custom fonts, styling, positions. |
| **Subtitles** | WebVTT | `text/vtt` (.vtt) | W3C WebVTT standard, cue settings, styling tags. |

---

### 2.2 Seeking Accuracy & SMPTE Timecode Engine

#### 2.2.1 Seeking Modes
1. **Exact Seek (Precision Mode)**:
   - Seeks directly to the exact target millisecond timestamp $t_{\text{target}}$.
   - Decoding pipeline seeks to the preceding IDR keyframe and decodes forward to $t_{\text{target}}$, presenting the exact target frame.
   - Required for frame stepping, SMPTE scrubber navigation, and `.lrc` lyric click-to-seek.
2. **Fast / Keyframe Seek (Scrubbing Mode)**:
   - Seeks to the nearest keyframe $\le t_{\text{target}}$ for low-latency interactive slider dragging.
3. **Jump Seeks**:
   - Discrete relative seek jumps: $-10\text{s}$, $+10\text{s}$ (keyboard Left/Right arrows or dedicated UI buttons), $-30\text{s}$, $+30\text{s}$ (Shift + Left/Right).

#### 2.2.2 SMPTE Timecode Mathematical Specification
Video media position must be formatted and displayed in real time as **SMPTE timecode**: `HH:MM:SS:FF`.

- Let $t$ be current playback position in seconds ($t = \text{position\_ms} / 1000.0$).
- Let $FPS$ be the video stream frame rate (nominal values: $23.976, 24.0, 25.0, 29.97, 30.0, 50.0, 59.94, 60.0$). If audio-only or unknown video frame rate, default $FPS = 30.0$.
- **Calculation (Non-Drop Frame)**:
  $$\text{Total Frame Count } N = \lfloor t \times FPS + 0.5 \rfloor$$
  $$\text{Hours } HH = \lfloor N / (3600 \times FPS) \rfloor$$
  $$\text{Minutes } MM = \lfloor (N \bmod (3600 \times FPS)) / (60 \times FPS) \rfloor$$
  $$\text{Seconds } SS = \lfloor (N \bmod (60 \times FPS)) / FPS \rfloor$$
  $$\text{Frames } FF = N \bmod \text{round}(FPS)$$
- **Output Format**: `HH:MM:SS:FF` (e.g. `00:14:22:18`).
- **Remaining Time Calculation**:
  $$\text{Remaining Time } t_{\text{rem}} = \max(0.0, \text{duration} - t)$$
  Formatted as `-HH:MM:SS:FF` (e.g. `-01:30:37:12`).

#### 2.2.3 Single-Frame Stepping (`< 1F` / `1F >`)
- **Forward Step (`1F >`)**:
  - Increments playback position by exactly $\Delta t = +1.0 / FPS$ seconds.
  - Automatically pauses playback if currently playing.
  - Renders the exact next decoded video frame without audio glitch or pipeline stutter.
- **Backward Step (`< 1F`)**:
  - Decrements playback position by exactly $\Delta t = -1.0 / FPS$ seconds.
  - Automatically pauses playback if currently playing.
  - Performs backward keyframe seek + forward decode to render the exact previous frame.

---

### 2.3 Playback Rate & Pitch Preservation
- **Rate Range**: $0.5\times$ to $2.0\times$ in continuous steps ($0.05\times$ resolution) or discrete presets ($0.5\times, 0.75\times, 1.0\times, 1.25\times, 1.5\times, 1.75\times, 2.0\times$).
- **Pitch Preservation (Time-Stretching DSP)**:
  - Audio time-stretching must use **WSOLA (Waveform Similarity Overlap-Add)** or `scaletempo2` / `rubberband` DSP.
  - Pitch shift ratio must remain strictly $1.0$ (no chipmunk / demon vocal distortion) when playback rate $\ne 1.0$.

---

### 2.4 Stream Discovery & Dynamic Switching

#### 2.4.1 Audio Stream Structure
Each audio track is represented by a structured record:
```cpp
struct AudioTrackInfo {
    int id;               // Stream index (0, 1, 2, ...)
    QString title;        // e.g. "Director's Commentary", "Surround 5.1"
    QString language;     // ISO 639-1 / 639-2 code (e.g. "eng", "jpn", "ger")
    QString codec;        // e.g. "aac", "flac", "ac3", "opus"
    int channels;         // 1 (Mono), 2 (Stereo), 6 (5.1), 8 (7.1)
    int sampleRate;       // e.g. 44100, 48000, 96000 Hz
    int bitRate;          // in bps (e.g. 320000)
    bool isDefault;
};
```
- **Track Selection**: Dynamic switching (`setAudioTrack(int id)`) must seamlessly transition audio output without restarting the video decoder.

#### 2.4.2 Subtitle Stream Structure
```cpp
struct SubtitleTrackInfo {
    int id;               // Stream index (-1 = Disabled, 0, 1, 2, ...)
    QString title;        // e.g. "English Full", "Signs & Songs"
    QString language;     // ISO 639 code (e.g. "eng", "spa", "fra")
    QString format;       // "srt", "ass", "vtt", "subrip", "pgs"
    bool isExternal;      // true if loaded from sidecar file (.srt/.ass/.vtt)
    QString sourcePath;   // File path if external
};
```
- **External Subtitle Loading**:
  - `loadExternalSubtitle(const QString &filePath)`: Inspects file existence, parses format extension, auto-detects text encoding (UTF-8, UTF-16LE, UTF-16BE, Windows-1252/Latin-1), and registers as an active subtitle track.
  - Auto-discovery: When opening `/path/movie.mp4`, scan directory for `/path/movie*.srt`, `/path/movie*.ass`, `/path/movie*.vtt` and automatically append to available subtitle list.

---

## 3. Tactile Digital Brutalist UI Specifications

### 3.1 Visual Design Language & Color Palette

The user interface follows the **Tactile Digital Brutalism** design standard: studio precision, high-contrast structural hierarchy, razor-sharp 1px boundary lines, mechanical industrial controls, and absolute zero rounded corners (`border-radius: 0px`).

| Token Name | Hex Code | Semantic Role & UI Placement |
|---|---|---|
| `BG_DEEP_OBSIDIAN` | `#070709` | Main application canvas, video background canvas, window root background. |
| `SURFACE_PANEL_BASE` | `#0B0B0E` | Secondary panels, control dock background, sidebar panels, modal cards. |
| `GRID_STRUCTURAL_BORDER` | `#1E1E24` | 1px razor grid divider lines, cell boundaries, rack chassis edges, button borders. |
| `TEXT_HIGH_CONTRAST` | `#FFFFFF` | Primary headings, active track title, active SMPTE timecode, button labels. |
| `TEXT_SECONDARY_DIM` | `#777788` | Metadata labels (Artist, Album, Codec), unselected queue items, tick ruler markings. |
| `TEXT_MUTED` | `#444455` | Inactive controls, disabled buttons, subtle background labels. |
| `ACCENT_SAFETY_ORANGE` | `#FF4400` | Playhead scrubber needle, active play state, recording indicator, error/warning badges, clipping zone on VU meters. |
| `ACCENT_SIGNAL_LIME` | `#CCFF00` | VU meter nominal peak bars ($-60\text{ dB}$ to $-3\text{ dB}$), active lyric line highlight, live lock indicators, EQ boost bars. |
| `ACCENT_TELEMETRY_CYAN` | `#00E5FF` | Viewfinder safe-area reticles, telemetry OSD technical labels, frame time readout. |

### 3.2 Typography Hierarchy

- **Monospace Font**: `JetBrains Mono`, `DejaVu Sans Mono`, `Liberation Mono`, `monospace`
  - Used for: SMPTE timecodes (`00:14:22:15`), OSD telemetry readouts (`FPS: 59.94`), graphic equalizer frequency labels (`32Hz`, `1kHz`), VU meter decibel scale (`-60dB`, `0dB`), bitrates, sample rates.
  - Metrics: Tabular numeric figures (`font-variant-numeric: tabular-nums;`), letter-spacing: `0.05em`.
- **Swiss Sans Font**: `Inter`, `Liberation Sans`, `DejaVu Sans`, `Helvetica Neue`, `sans-serif`
  - Used for: Track titles, artist/album headings, playlist matrix rows, modal dialogs, button tooltips.
  - Metrics: High x-height, geometric clarity, weights 400 (Regular) and 700 (Bold).

---

### 3.3 Mode 1: Video Viewfinder Mode

```
+-------------------------------------------------------------------------------+
| [MODE: VIDEO]  file_name.mp4               [OSD: ON] [RETICLE: ON] [FS] [X]   |
+-------------------------------------------------------------------------------+
| + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - + |
| | [TELEMETRY OSD]                                    [ACTION/TITLE SAFE]    | |
| | FPS: 59.94 / 60.00 | DROP: 0                                              | |
| | V_CODEC: H264 4,820 kbps | RES: 3840x2160@60Hz                            | |
| | A_CODEC: AAC  256 kbps   | COLOR: BT.709 8-bit                            | |
| | RENDER: 1.42ms           | SKEW: +0.002ms                                 | |
| |                                                                           | |
| |                                 +                                         | |
| |                           (Center Cross)                                  | |
| |                                                                           | |
| + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - + |
+-------------------------------------------------------------------------------+
| 00:14:22:18  |...|...|...|...|...|...[#]...|...|...|...|...|...| -01:30:37:12 |
|               (Mechanical Tick Ruler Scrubber with Chapter Flags)             |
+-------------------------------------------------------------------------------+
| [|< 1F] [-10s] [ > PLAY ] [+10s] [1F >|] [ [] STOP ] | [1.0x v] | [AUD: Eng 5.1 v] | [SUB: Eng v] |
+-------------------------------------------------------------------------------+
```

#### 3.3.1 Video Viewport & Technical Safe-Area Reticles
- **Viewport**: Hardware-accelerated OpenGL surface (`QOpenGLWidget`) with letterboxing / pillarboxing to maintain source aspect ratio.
- **Safe-Area Reticle Overlays** (Toggleable via `RETICLE` button or shortcut `R`):
  - **Action-Safe Boundary**: 90% rectangle border from video frame edges (1px subtle cyan dashed line `#00E5FF` at 40% opacity).
  - **Title-Safe Boundary**: 80% rectangle border from video frame edges (1px solid cyan line `#00E5FF` at 50% opacity).
  - **Center Crosshair**: Precision target '+' reticle at $(x_{\text{center}}, y_{\text{center}})$ with $16\text{px}$ arms.
  - **Aspect Ratio Guides**: Toggleable framing outlines for `16:9`, `2.39:1 CinemaScope`, `4:3`, and `1:1`.

#### 3.3.2 Telemetry Diagnostics OSD HUD (Toggleable via `OSD` or shortcut `O`)
Monospace technical HUD rendered in top-left corner:
- `FPS: <actual_fps> / <nominal_fps>`
- `DROPPED: <dropped_frames_count>`
- `V_CODEC: <video_codec_name> | BITRATE: <video_bitrate_kbps> kbps`
- `RES: <width>x<height>@<fps>Hz | COLOR: <colorspace> <bitdepth>-bit`
- `A_CODEC: <audio_codec_name> | SAMPLE_RATE: <audio_rate> Hz`
- `RENDER_TIME: <frame_render_ms> ms | A/V SKEW: <skew_ms> ms`

#### 3.3.3 Mechanical Timecode Tick Ruler Scrubber
- **Ruler Geometry**: Fixed height horizontal strip with background `#070709` and top/bottom borders `#1E1E24`.
- **Tick Marks**:
  - Major ticks every 60 seconds (or calibrated scale depending on duration): height 12px, color `#777788`.
  - Minor ticks every 10 seconds / 1 second: height 6px, color `#333344`.
- **Chapter Markers**: Solid vertical diamond notches `#00E5FF` at chapter start times; hover tooltip displays chapter title.
- **Scrubber Needle**: Vertical razor line in Safety Orange `#FF4400` with top indicator flag and real-time SMPTE tooltip.
- **Timecode Displays**:
  - Left: Elapsed SMPTE `HH:MM:SS:FF` in high-contrast white `#FFFFFF`.
  - Right: Remaining SMPTE `-HH:MM:SS:FF` in dim text `#777788`.

#### 3.3.4 Tactile Control Dock
Bottom panel fixed height with tactile grid-lined buttons:
- Step Backward: `[ |< 1F ]`
- Jump Backward: `[ -10s ]`
- Play / Pause Toggle: `[ > PLAY ]` / `[ || PAUSE ]` (active play indicator in Safety Orange `#FF4400`)
- Jump Forward: `[ +10s ]`
- Step Forward: `[ 1F >| ]`
- Stop: `[ [] STOP ]`
- Speed Selector: `[ RATE: 1.0x v ]` (menu options: 0.5x, 0.75x, 1.0x, 1.25x, 1.5x, 2.0x)
- Audio Stream Selector: `[ AUD: Track 1 (eng) v ]`
- Subtitle Stream Selector: `[ SUB: English v ]`
- Mode Toggles: `[ RETICLE ]`, `[ OSD ]`, `[ PiP ]`, `[ FULLSCREEN ]`, `[ SWITCH TO AUDIO DECK ]`.

---

### 3.4 Mode 2: Hi-Fi Audio Deck Mode

```
+-------------------------------------------------------------------------------+
| [MODE: AUDIO DECK]  TRACK 04 / 12                         [SWITCH TO VIDEO]   |
+-------------------------------------------------------------------------------+
|  TYPOGRAPHIC METADATA DISPLAY:                                                |
|  TITLE:  CYBERNETIC HORIZON                                                   |
|  ARTIST: KRAFTWERK                                                            |
|  ALBUM:  COMPUTER WORLD [1981]                                                |
|  FORMAT: [FLAC 24-bit / 96.0 kHz / 2450 kbps] [STEREO]                        |
+---------------------------------------+---------------------------------------+
|  STEREO PEAK VU METERS:               |  10-BAND GRAPHIC EQUALIZER RACK:      |
|  CH_L: [-60...-20...-6...0..+3] dB    |  [PRESET: Flat v]  [RESET FLAT]       |
|  [========================|  ] -3.2dB |  32  64 125 250 500  1k  2k  4k  8k 16k|
|  CH_R: [-60...-20...-6...0..+3] dB    |  ||  ||  ||  ||  ||  ||  ||  ||  ||  || |
|  [======================|    ] -5.8dB |  +0  +2  +4  +1  -1  -2  +0  +3  +4  +2 |
+---------------------------------------+---------------------------------------+
|  SYNCHRONIZED .LRC TELEPROMPTER:      |  PLAYLIST QUEUE MATRIX:               |
|    [01:12.40] Synthetic voices rising|  #  TITLE          ARTIST     DUR  BR |
|  > [01:16.85] TIME IS AN ILLUSION <   |  01 Neon Lights    Kraftwerk  4:20 FL |
|    [01:21.30] Data streams pulse...   | >02 Cyber Horizon  Kraftwerk  5:12 FL |
|    [01:25.80] In the digital matrix   |  03 Pocket Calc    Kraftwerk  3:45 FL |
+---------------------------------------+---------------------------------------+
| 00:01:16:25  |...|...|...|...|...|...[#]...|...|...|...|...|...| -00:03:55:05 |
+-------------------------------------------------------------------------------+
| [|< PREV] [-10s] [ > PLAY ] [+10s] [NEXT >|] [SHUFFLE: OFF] [REPEAT: ALL]    |
+-------------------------------------------------------------------------------+
```

#### 3.4.1 Typographic Metadata Masthead
- Displayed prominently in Swiss Sans Bold (Title) and JetBrains Mono (Technical badges).
- Extracted tags: Title, Artist, Album, Year, Genre, Track Number.
- High-resolution album artwork rendering in square frame with 1px border `#1E1E24`.
- Technical badge pill: `[FLAC 24-bit / 96kHz / 2,450 kbps]` or `[MP3 320kbps / 44.1kHz]`.

#### 3.4.2 Stereo Peak VU Meters (CH_L & CH_R)
- Dual logarithmic bargraph meters calibrated from $-60.0\text{ dB}$ to $+3.0\text{ dB}$.
- **Decibel Calculation**:
  Given audio buffer samples $x[n]$ for channel $c \in \{L, R\}$:
  $$\text{Peak Level } P_c = \max_{n} |x[n]|$$
  $$\text{dBFS} = 20 \log_{10}(P_c + 10^{-6})$$
- **Color Zones**:
  - Nominal Zone ($-60\text{ dB}$ to $-3.0\text{ dB}$): Signal Lime `#CCFF00`.
  - Headroom / Warning Zone ($-3.0\text{ dB}$ to $0.0\text{ dB}$): Safety Orange `#FF4400`.
  - Clip Zone ($> 0.0\text{ dB}$): High-contrast Red/Orange `#FF2200`.
- **Peak Hold Indicator**: Single 2px vertical bar at highest peak position with $500\text{ms}$ hold time before decaying at $-20\text{ dB/s}$.

#### 3.4.3 10-Band Graphic Equalizer Rack
- **Center Frequencies (ISO 1/1 Octave Bands)**:
  1. Band 1: `32 Hz` (Sub-bass)
  2. Band 2: `64 Hz` (Bass)
  3. Band 3: `125 Hz` (Upper Bass)
  4. Band 4: `250 Hz` (Low Midrange)
  5. Band 5: `500 Hz` (Midrange)
  6. Band 6: `1 kHz` (Upper Midrange)
  7. Band 7: `2 kHz` (Presence)
  8. Band 8: `4 kHz` (Brilliance)
  9. Band 9: `8 kHz` (High Treble)
  10. Band 10: `16 kHz` (Air)
- **Gain Range**: $-12.0\text{ dB}$ to $+12.0\text{ dB}$ per band with $0.1\text{ dB}$ adjustment resolution.
- **Filter Implementation**: 10 cascaded 2nd-order Biquad Peaking EQ IIR filters or dynamic FFmpeg equalizer filter string (`equalizer=f=32:t=q:w=1.414:g=G1,equalizer=f=64...`).
- **Presets Table**:
  - **Flat**: `[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]` dB
  - **Rock**: `[+4.5, +3.0, +1.5, 0.0, -1.0, -0.5, +1.5, +3.0, +4.0, +4.5]` dB
  - **Pop**: `[-1.0, +1.5, +3.0, +3.5, +2.0, -0.5, +1.5, +2.5, +3.0, +2.0]` dB
  - **Jazz**: `[+3.0, +2.0, +1.0, +1.5, -1.5, -1.5, 0.0, +1.5, +2.5, +3.0]` dB
  - **Classical**: `[+4.0, +3.0, +2.0, +1.5, -1.0, -1.0, 0.0, +2.0, +3.0, +3.5]` dB
  - **Bass Boost**: `[+7.0, +6.0, +4.5, +2.5, +1.0, 0.0, 0.0, 0.0, 0.0, 0.0]` dB
  - **Vocal Boost**: `[-2.0, -1.5, -1.0, +1.5, +3.5, +4.0, +3.0, +1.5, 0.0, -1.0]` dB
  - **Electronic**: `[+5.5, +4.5, +2.0, 0.0, -2.0, +2.0, +1.0, +2.5, +4.5, +5.0]` dB
  - **Acoustic**: `[+3.5, +2.5, +1.5, +0.5, +1.0, +1.5, +2.0, +2.5, +3.0, +2.0]` dB
- **Tactile Reset Button**: `[RESET FLAT]` instantly sets all 10 sliders to $0.0\text{ dB}$.

#### 3.4.4 Synchronized .lrc Teleprompter
- **Parser Specification**:
  - Supports standard LRC timestamps: `[mm:ss.xx]` or `[mm:ss.xxx]` (e.g. `[01:16.85] Line text`).
  - Supports multiple timestamps per line: `[00:12.00][00:24.00] Repeated chorus`.
  - Supports metadata ID tags: `[ti:Title]`, `[ar:Artist]`, `[al:Album]`, `[by:Creator]`, `[offset:+500]`.
  - Timestamp sort & binary search for real-time active lyric lookup.
- **Visual Display**:
  - Teleprompter view with active lyric centered and highlighted in bold high-contrast Signal Lime `#CCFF00` or White `#FFFFFF`.
  - Past lyrics dimmed to `#444455`; future lyrics rendered in `#777788`.
  - Smooth continuous vertical auto-scroll keeping the active line in viewport center.
  - **Click-to-Seek**: Clicking any lyric line immediately issues an exact seek to that lyric's timestamp.

#### 3.4.5 Playlist Queue Matrix
- Table matrix columns: `#` (Index), `TITLE`, `ARTIST`, `ALBUM`, `DURATION`, `BITRATE`, `FORMAT`.
- Interactive operations:
  - Drag-and-drop row reordering.
  - Double-click to play item immediately.
  - Delete key / right-click menu to remove tracks.
  - Shuffle toggle (Fisher-Yates random permutation).
  - Repeat mode toggle: `Off` -> `Repeat All` -> `Repeat One`.

---

## 4. Linux Desktop System Integration & MPRIS2 Specifications

### 4.1 MPRIS2 D-Bus Architecture (`org.mpris.MediaPlayer2.penguin`)

Penguin registers on the D-Bus Session Bus under the standard MPRIS2 specification:
- **Service Name**: `org.mpris.MediaPlayer2.penguin` (if primary instance) or `org.mpris.MediaPlayer2.penguin.instance<PID>` (if multiple instances).
- **Object Path**: `/org/mpris/MediaPlayer2`

```
                                      D-Bus Session Bus
                                              |
                     +------------------------+------------------------+
                     |                                                 |
       Interface: org.mpris.MediaPlayer2              Interface: org.mpris.MediaPlayer2.Player
       +-------------------------------+              +--------------------------------------+
       | Methods:                      |              | Methods:                             |
       |  - Raise()                    |              |  - Next()          - Previous()      |
       |  - Quit()                     |              |  - Pause()         - PlayPause()     |
       | Properties (read-only):       |              |  - Stop()          - Play()          |
       |  - CanQuit (b: true)          |              |  - Seek(x: offset_us)                |
       |  - CanRaise (b: true)         |              |  - SetPosition(o: id, x: pos_us)     |
       |  - HasTrackList (b: false)    |              |  - OpenUri(s: uri)                   |
       |  - Identity (s: "Penguin")    |              | Signals:                             |
       |  - DesktopEntry (s: "penguin")|              |  - Seeked(x: position_us)            |
       |  - SupportedUriSchemes (as)   |              | Properties:                          |
       |  - SupportedMimeTypes (as)    |              |  - PlaybackStatus (s)                |
       +-------------------------------+              |  - LoopStatus (s)   - Rate (d)       |
                                                      |  - Shuffle (b)      - Metadata (a{sv})|
                                                      |  - Volume (d)       - Position (x)   |
                                                      +--------------------------------------+
```

#### 4.1.1 `org.mpris.MediaPlayer2` (Root Interface) Specification
- **Methods**:
  - `Raise() -> ()`: Brings the Penguin application window to foreground and requests window focus.
  - `Quit() -> ()`: Cleanly terminates playback, flushes persistence state to disk, and closes the application.
- **Properties**:
  - `CanQuit` (`b`, read-only): `true`
  - `CanRaise` (`b`, read-only): `true`
  - `HasTrackList` (`b`, read-only): `false` (or `true` if TrackList interface implemented)
  - `Identity` (`s`, read-only): `"Penguin Media Player"`
  - `DesktopEntry` (`s`, read-only): `"penguin"`
  - `SupportedUriSchemes` (`as`, read-only): `["file", "http", "https", "ftp"]`
  - `SupportedMimeTypes` (`as`, read-only):
    `["audio/mpeg", "audio/flac", "audio/ogg", "audio/opus", "audio/wav", "audio/aac", "video/mp4", "video/x-matroska", "video/webm", "video/x-msvideo"]`

#### 4.1.2 `org.mpris.MediaPlayer2.Player` Interface Specification
- **Methods**:
  - `Next() -> ()`: Advances playback to the next track in queue.
  - `Previous() -> ()`: Returns to previous track or seeks to start of current track if position $> 3\text{s}$.
  - `Pause() -> ()`: Pauses playback.
  - `PlayPause() -> ()`: Toggles between playing and paused states.
  - `Stop() -> ()`: Stops playback and resets position to $0$.
  - `Play() -> ()`: Starts or resumes playback.
  - `Seek(x: offset_microseconds) -> ()`: Performs relative seek by offset microseconds.
  - `SetPosition(o: track_id, x: position_microseconds) -> ()`: Sets absolute playback position. If `track_id` matches current track, seeks to `position_microseconds`.
  - `OpenUri(s: uri) -> ()`: Appends or replaces queue with media at URI and begins playback.
- **Signals**:
  - `Seeked(x: position_microseconds)`: Emitted whenever playback position changes due to an explicit seek operation.
- **Properties**:
  - `PlaybackStatus` (`s`, read-only): `"Playing"`, `"Paused"`, or `"Stopped"`.
  - `LoopStatus` (`s`, read/write): `"None"`, `"Track"`, or `"Playlist"`.
  - `Rate` (`d`, read/write): Current playback rate ($0.5$ to $2.0$).
  - `Shuffle` (`b`, read/write): `true` if shuffle is enabled, `false` otherwise.
  - `Volume` (`d`, read/write): Audio volume level from $0.0$ ($0\%$) to $1.0$ ($100\%$).
  - `Position` (`x`, read-only): Current playback position in microseconds ($10^{-6}\text{ s}$).
  - `MinimumRate` (`d`, read-only): `0.5`
  - `MaximumRate` (`d`, read-only): `2.0`
  - `CanGoNext` (`b`, read-only): `true` if next track exists.
  - `CanGoPrevious` (`b`, read-only): `true` if previous track exists.
  - `CanPlay` (`b`, read-only): `true`
  - `CanPause` (`b`, read-only): `true`
  - `CanSeek` (`b`, read-only): `true`
  - `CanControl` (`b`, read-only): `true`
  - `Metadata` (`a{sv}`, read-only): Dictionary containing track metadata:
    - `mpris:trackid` (`o`): Object path (e.g. `/org/mpris/MediaPlayer2/Track/1` or `/org/mpris/MediaPlayer2/TrackList/NoTrack`)
    - `mpris:length` (`x`): Total duration in microseconds ($10^{-6}\text{ s}$)
    - `mpris:artUrl` (`s`): URI to album art image (`file:///...` or embedded temporary image URI)
    - `xesam:title` (`s`): Title of the media track
    - `xesam:artist` (`as`): Array of artist names
    - `xesam:album` (`s`): Album name
    - `xesam:albumArtist` (`as`): Array of album artist names
    - `xesam:genre` (`as`): Array of genres
    - `xesam:trackNumber` (`i`): Track sequence number
    - `xesam:url` (`s`): Media file URI (`file:///path/to/media.mp4`)

#### 4.1.3 D-Bus Property Change Broadcasting
When any property (`PlaybackStatus`, `Metadata`, `Volume`, `Rate`, `LoopStatus`, `Shuffle`) changes, Penguin emits the standard D-Bus signal:
`org.freedesktop.DBus.Properties.PropertiesChanged("org.mpris.MediaPlayer2.Player", changed_properties_dict, invalidated_properties_list)`.

---

### 4.2 Linux Desktop Packaging, CLI & Audio Server Routing

#### 4.2.1 Desktop Entry (`penguin.desktop`)
Location: `~/.local/share/applications/penguin.desktop` or `/usr/share/applications/penguin.desktop`
```ini
[Desktop Entry]
Version=1.5
Type=Application
Name=Penguin
GenericName=Media Player
Comment=Tactile Digital Brutalist Desktop Media Player
Exec=penguin %U
Icon=penguin
Terminal=false
StartupNotify=true
StartupWMClass=penguin
Categories=AudioVideo;Audio;Video;Player;Qt;
MimeType=audio/mpeg;audio/flac;audio/ogg;audio/opus;audio/wav;audio/aac;video/mp4;video/x-matroska;video/webm;video/x-msvideo;
Actions=PlayPause;Next;Previous;Stop;

[Desktop Action PlayPause]
Name=Play/Pause
Exec=penguin --toggle-pause

[Desktop Action Next]
Name=Next Track
Exec=penguin --next

[Desktop Action Previous]
Name=Previous Track
Exec=penguin --prev

[Desktop Action Stop]
Name=Stop Playback
Exec=penguin --stop
```

#### 4.2.2 Scalable Icon (`penguin.svg`)
Location: `/usr/share/icons/hicolor/scalable/apps/penguin.svg` or embedded Qt resource `:/icons/penguin.svg`.
Design: Razor-sharp brutalist penguin silhouette with industrial safety orange `#FF4400` eye/beak aperture and viewfinder crosshairs.

#### 4.2.3 Command-Line Interface (CLI) Specification
Command Synopsis:
```
penguin [OPTIONS] [FILE_OR_URL...]
```
Supported Arguments:

| Argument / Flag | Type | Description |
|---|---|---|
| `[FILE_OR_URL...]` | Positional | One or more paths or URLs to open and add to the playlist queue. Playback begins immediately with the first item. |
| `-a, --audio` | Flag | Force initial UI launch in Hi-Fi Audio Deck mode. |
| `-v, --video` | Flag | Force initial UI launch in Video Viewfinder mode. |
| `-f, --fullscreen` | Flag | Launch application in fullscreen mode. |
| `--volume <0-100>` | Integer | Set initial audio volume percentage (0 to 100). |
| `--speed <0.5-2.0>` | Float | Set initial playback speed rate. |
| `--sub <file>` | Path | Load explicit external subtitle file (.srt, .ass, .vtt) alongside first media item. |
| `--toggle-pause` | IPC Flag | Sends play/pause toggle command to running Penguin instance via D-Bus / SingleApplication. |
| `--next` | IPC Flag | Advances running Penguin instance to next track via D-Bus. |
| `--prev` | IPC Flag | Reverses running Penguin instance to previous track via D-Bus. |
| `--stop` | IPC Flag | Stops playback on running Penguin instance via D-Bus. |
| `--test` | Flag | Executes automated programmatic verification suite (headless offscreen mode). Exits with 0 on pass. |
| `-h, --help` | Flag | Prints comprehensive usage synopsis and exits 0. |
| `-V, --version` | Flag | Prints Penguin version and build information and exits 0. |

#### 4.2.4 Global & Window Keyboard Shortcuts Matrix

| Key / Combination | Action | Scope / Context |
|---|---|---|
| `Space` / `K` | Toggle Play / Pause | Global Window |
| `Left Arrow` / `J` | Seek Backward 10 Seconds (`-10s`) | Global Window |
| `Right Arrow` / `L` | Seek Forward 10 Seconds (`+10s`) | Global Window |
| `Shift + Left Arrow` | Seek Backward 30 Seconds (`-30s`) | Global Window |
| `Shift + Right Arrow`| Seek Forward 30 Seconds (`+30s`) | Global Window |
| `,` (Comma) | Step Backward 1 Frame (`< 1F`) | Video Mode (Auto-pauses) |
| `.` (Period) | Step Forward 1 Frame (`1F >`) | Video Mode (Auto-pauses) |
| `Up Arrow` | Volume Up (+5%) | Global Window |
| `Down Arrow` | Volume Down (-5%) | Global Window |
| `M` | Toggle Mute / Unmute | Global Window |
| `[` | Decrease Playback Speed (-0.1x) | Global Window |
| `]` | Increase Playback Speed (+0.1x) | Global Window |
| `Backspace` | Reset Playback Speed to 1.0x | Global Window |
| `F` / `F11` | Toggle Fullscreen | Global Window |
| `P` | Toggle Picture-in-Picture / Always-on-Top | Global Window |
| `O` | Toggle Telemetry OSD HUD | Video Mode |
| `R` | Toggle Safe-Area Reticles | Video Mode |
| `Tab` | Switch UI Mode (Video Viewfinder <-> Audio Deck) | Global Window |
| `Ctrl + O` | Open File Dialog | Global Window |
| `Ctrl + Shift + O` | Open Folder / Directory Dialog | Global Window |
| `Ctrl + Q` / `Alt + F4` | Quit Penguin | Global Window |

#### 4.2.5 Audio Routing: PipeWire & PulseAudio Integration
- The audio engine registers with PulseAudio / PipeWire audio server with:
  - `application.name = "Penguin Media Player"`
  - `application.icon_name = "penguin"`
  - `media.role = "music"` (in Audio mode) or `"video"` (in Video mode)
  - `stream.latency-min = "20ms"` for low-latency lip-sync audio/video rendering.
- Seamless stream handover across default output sink switches (e.g. plugging in USB DAC or Bluetooth headphones).

---

## 5. Media Library & State Persistence Specifications

### 5.1 File & Directory I/O and Drag-and-Drop Handling

1. **File Opening**:
   - Standard file open dialog filtering supported multimedia formats.
   - Immediate playback start for selected file; if multiple files selected, enqueued in selection order.
2. **Folder / Directory Opening**:
   - Recursively walks target directory for supported video and audio files.
   - Files sorted in natural alphanumeric order or by embedded track numbers.
   - Batch loaded into playlist queue model without freezing UI main thread (asynchronous directory scanner).
3. **Drag-and-Drop Handling**:
   - Accepts `text/uri-list` and `application/x-qt-windows-mime;value="FileName"` drops on main viewport.
   - Resolves dropped URIs to absolute filesystem paths.
   - If dropped item is a directory, executes recursive scanning.
   - If dropped item is a subtitle file (.srt/.ass/.vtt), attaches subtitle to currently playing video.
   - If dropped item is a media file/folder, appends or replaces queue based on user drop zone.

---

### 5.2 Persistence Schemas

#### 5.2.1 Configuration File Schema (`config.json`)
Location: `$XDG_CONFIG_HOME/penguin/config.json` (defaults to `~/.config/penguin/config.json`)

```json
{
  "version": 1,
  "volume": 0.85,
  "muted": false,
  "playback_rate": 1.0,
  "ui_mode": "video",
  "osd_enabled": true,
  "reticles_enabled": true,
  "reticle_mode": "action_title_safe",
  "equalizer": {
    "preset": "Rock",
    "gains": [4.5, 3.0, 1.5, 0.0, -1.0, -0.5, 1.5, 3.0, 4.0, 4.5]
  },
  "window": {
    "x": 100,
    "y": 100,
    "width": 1280,
    "height": 720,
    "is_maximized": false,
    "is_fullscreen": false
  },
  "subtitles": {
    "auto_load_sidecar": true,
    "font_size": 24,
    "color": "#FFFFFF",
    "encoding": "UTF-8"
  }
}
```

#### 5.2.2 SQLite State & History Database Schema (`history.db`)
Location: `$XDG_DATA_HOME/penguin/history.db` (defaults to `~/.local/share/penguin/history.db`)

```sql
-- Recent Playback History and Resume Positions
CREATE TABLE IF NOT EXISTS recent_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_path TEXT UNIQUE NOT NULL,
    title TEXT,
    artist TEXT,
    album TEXT,
    duration_ms INTEGER NOT NULL DEFAULT 0,
    last_position_ms INTEGER NOT NULL DEFAULT 0,
    last_played_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    play_count INTEGER DEFAULT 1
);

CREATE INDEX IF NOT EXISTS idx_history_last_played ON recent_history(last_played_at DESC);

-- Saved Playlists Master Table
CREATE TABLE IF NOT EXISTS saved_playlists (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Playlist Items Matrix
CREATE TABLE IF NOT EXISTS playlist_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    playlist_id INTEGER NOT NULL,
    position_index INTEGER NOT NULL,
    file_path TEXT NOT NULL,
    title TEXT,
    artist TEXT,
    duration_ms INTEGER DEFAULT 0,
    FOREIGN KEY(playlist_id) REFERENCES saved_playlists(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_playlist_order ON playlist_items(playlist_id, position_index);
```

---

## 6. Verification & Automated Test Suite Specifications

### 6.1 Test Suite Architecture & Offscreen Execution

The verification system provides **100% genuine automated validation** with zero mocks or stubbing in production code.

```
+-----------------------------------------------------------------------------------+
|                           AUTOMATED TEST ARCHITECTURE                             |
+-----------------------------------------------------------------------------------+
|                                                                                   |
|  +-------------------------------------+   +------------------------------------+ |
|  |     Headless Test Runner Script     |   |       CLI Self-Test Mode           | |
|  |      `tests/run_all_tests.sh`       |   |         `penguin --test`           | |
|  +------------------+------------------+   +------------------+-----------------+ |
|                     |                                         |                   |
|  +------------------v-----------------------------------------v-----------------+ |
|  |            Environment Harness: QT_QPA_PLATFORM=offscreen                    | |
|  |            Isolated D-Bus Bus:  dbus-run-session                             | |
|  +-------------------------------------+----------------------------------------+ |
|                                        |                                          |
|  +-------------------------------------v----------------------------------------+ |
|  |                        CORE TEST MODULE SUITES                               | |
|  |  1. MediaParserTest: Container/Codec tag extraction & metadata parsing       | |
|  |  2. SmpteTimecodeTest: Math calculations, frame rates, drop/non-drop frame   | |
|  |  3. LrcParserTest: Timestamp tags, sync matching, out-of-order cues          | |
|  |  4. EqualizerDspTest: 10-band gains, preset configurations, flat reset       | |
|  |  5. VuMeterDspTest: Peak & RMS decibel math (-60dB to +3dB, clipping)       | |
|  |  6. Mpris2DbusTest: D-Bus service, methods, properties, signal broadcasts   | |
|  |  7. PlaylistModelTest: Queue matrix, drag-and-drop moves, shuffle, repeat    | |
|  |  8. StatePersistenceTest: SQLite schema CRUD, config.json load/save         | |
|  |  9. PlaybackPipelineTest: Frame stepping, speed rate, seek accuracy          | |
|  +------------------------------------------------------------------------------+ |
+-----------------------------------------------------------------------------------+
```

### 6.2 Test Matrix & Synthetic Test Fixture Generation

Synthetic test fixtures can be generated deterministically via FFmpeg:
```bash
# Generate 5-second synthetic MP4 video (H.264 + AAC 48kHz, 60fps) with SMPTE timecode burn-in
ffmpeg -y -f lavfi -i testsrc=duration=5:size=1280x720:rate=60 \
  -f lavfi -i sine=frequency=1000:duration=5:sample_rate=48000 \
  -c:v libx264 -pix_fmt yuv420p -c:a aac /tmp/test_synthetic.mp4

# Generate synthetic FLAC 24-bit 96kHz audio with ID3 metadata
ffmpeg -y -f lavfi -i sine=frequency=440:duration=3:sample_rate=96000 \
  -c:a flac -sample_fmt s32 /tmp/test_synthetic.flac

# Generate synthetic WebVTT and SRT subtitle fixtures
cat << 'EOF' > /tmp/test_subtitle.srt
1
00:00:01,000 --> 00:00:03,000
Penguin Viewfinder Subtitle Test
EOF
```

---

## 7. Features Discovered & Technical Matrix

### 7.1 Features Discovered Table

| # | Category | Feature | Description | Inputs | Outputs | Error Behavior | Discovered Via |
|---|---|---|---|---|---|---|---|
| 1 | Playback Engine | Multi-format Container Demuxing | Plays MP4, MKV, WebM, AVI, MP3, FLAC, Opus, AAC, WAV | File path or media URI | Demuxed A/V elementary streams | Invalid file format error signal; graceful playback skip | R1 Spec, libmpv2 / FFmpeg API |
| 2 | Playback Engine | Millisecond Accurate Seeking | Exact seek to target timestamp by IDR decode | Position in ms ($t$) | Accurate decoded frame presentation | Clamps to $[0, \text{duration}]$ | R1 Spec, mpv exact-seek |
| 3 | Playback Engine | Single-Frame Stepping | Advances or reverses by exactly $1/\text{FPS}$ seconds | `< 1F` or `1F >` action | Pauses engine, renders exact neighbor frame | Clamps at file start/end | R1 Spec, SMPTE frame math |
| 4 | Playback Engine | Variable Speed Playback | Smooth speed adjustment with WSOLA pitch preservation | Rate $0.5\times$ to $2.0\times$ | Audio/Video playback at altered tempo with $1.0\times$ pitch | Clamps to $[0.5, 2.0]$ | R1 Spec, scaletempo DSP |
| 5 | Playback Engine | Audio Track Switching | Enumerates and dynamically switches audio stream | Audio Track ID | Seamless audio stream handover | Reverts to default track on failure | R1 Spec, FFmpeg stream map |
| 6 | Playback Engine | Subtitle Track Discovery & External Loading | Discovers embedded subtitles and loads sidecar `.srt`, `.ass`, `.vtt` | Subtitle ID or file path | Subtitle text overlay on video surface | Logs warning on malformed file, skips invalid cues | R1 Spec, Subtitle parser |
| 7 | Brutalist UI | Obsidian & Razor Grid Theme | Digital brutalist styling with `#070709`, `#0B0B0E`, `#1E1E24` | UI State / QSS / Theme | Consistent 0px radius, 1px razor borders | Fallback to dark theme | R2 Spec, Design System |
| 8 | Brutalist UI | Video Viewfinder Mode | Broadcast viewport with safe-area reticles (Action 90%, Title 80%) | Toggle action `R` | Overlay reticle crosshairs & boundaries | Reticle hidden if video absent | R2 Spec, Broadcast SMPTE |
| 9 | Brutalist UI | Telemetry Diagnostics OSD | HUD display of FPS, dropped frames, bitrate, resolution, render time | Toggle action `O` | Top-left HUD monospace telemetry text | Displays "N/A" if stream metrics unavailable | R2 Spec, Video HUD |
| 10 | Brutalist UI | SMPTE Tick Ruler Scrubber | Calibrated mechanical tick ruler with Safety Orange `#FF4400` needle | Playback position / User drag | Formatted `HH:MM:SS:FF` and remaining time | Clamps seek within bounds | R2 Spec, Timecode Scrubber |
| 11 | Brutalist UI | Hi-Fi Audio Deck Mode | Typographic masthead display for audio files | File metadata | Track, Artist, Album, Bitrate badges | Fallbacks to filename if tags missing | R2 Spec, Audio Deck |
| 12 | Brutalist UI | Stereo Peak VU Meters | Logarithmic stereo bargraphs ($-60\text{ dB}$ to $+3\text{ dB}$) with peak hold | Audio PCM sample buffers | Dual real-time animated meters (Lime/Orange) | Decays to $-60\text{ dB}$ on silence | R2 Spec, Audio DSP |
| 13 | Brutalist UI | 10-Band Graphic Equalizer | Tactile sliders for 32Hz–16kHz bands with presets & flat reset | Gain array ($-12\text{ dB}$ to $+12\text{ dB}$) | Real-time audio frequency filter response | Bounds gain to $[-12.0, +12.0]\text{ dB}$ | R2 Spec, Biquad Filter Rack |
| 14 | Brutalist UI | Synchronized LRC Teleprompter | Real-time active lyric tracking with auto-scroll and click-to-seek | `.lrc` timestamped file | Highlighted active lyric line in Signal Lime | Hidden if no `.lrc` file present | R2 Spec, LRC Parser |
| 15 | Brutalist UI | Playlist Queue Matrix | Tabular queue list with drag-and-drop reordering, shuffle, repeat | Track items / Drag events | Updated playback sequence | Retains current track on reorder | R2 Spec, Playlist Model |
| 16 | Desktop & MPRIS2 | D-Bus Service `org.mpris.MediaPlayer2.penguin` | Root and Player interfaces registered on D-Bus Session Bus | D-Bus method calls & property queries | Return values, PropertyChanged signals | Error return if method unsupported | R3 Spec, MPRIS2 Standard |
| 17 | Desktop & MPRIS2 | Desktop Entry & Scalable Icon | `.desktop` file and SVG icon for desktop environment launcher | Desktop environment | Application launcher, MIME associations | Validated against desktop-file-validate | R3 Spec, XDG Desktop Spec |
| 18 | Desktop & MPRIS2 | Command-Line Arguments | CLI flags (`-a`, `-v`, `-f`, `--volume`, `--test`, `--help`) | CLI argv strings | Configures runtime state or runs test suite | Exits 1 on invalid argument with usage | R3 Spec, POSIX CLI Spec |
| 19 | Desktop & MPRIS2 | PipeWire & PulseAudio Integration | Media stream role and application naming on audio server | Audio output stream | Sound output via system default sink | Graceful fallback to ALSA if daemon missing | R3 Spec, PulseAudio Client API |
| 20 | Library & State | File / Directory Loading & Drag-and-Drop | Asynchronous recursive directory scanning & MIME URI drops | File path, directory path, URI drop | Populated playlist queue | Skips non-media files silently | R4 Spec, XDG MIME Handling |
| 21 | Library & State | SQLite History & State Store | Persistent SQLite DB for playback history and saved playlists | Playback events, track positions | Saved rows in `history.db` | Auto-creates schema if DB file absent | R4 Spec, SQLite3 Storage |
| 22 | Library & State | JSON Configuration Persistence | Reads/writes volume, EQ gains, window geometry to `config.json` | App settings changes | Serialized JSON file in `$XDG_CONFIG_HOME` | Defaults to factory settings if JSON corrupt | R4 Spec, JSON Config Store |
| 23 | Verification | Headless Automated Test Suite | Offscreen unit and integration test runner (`penguin --test`) | `--test` flag or test binary | TAP / JSON test summary, exit code 0 or 1 | Fails on any assertion violation | R5 Spec, Qt Test / Offscreen QPA |

---

### 7.2 Edge Cases & Observed Behavior Matrix

| # | Feature | Input / Condition | Observed & Required Behavior |
|---|---|---|---|
| 1 | SMPTE Timecode Math | Timestamp at exactly $0\text{ ms}$ | Returns `00:00:00:00` without division by zero or underflow. |
| 2 | SMPTE Timecode Math | Non-integer frame rate (e.g. $23.976\text{ fps}$) | Frame index $N = \lfloor t \times 23.976 + 0.5 \rfloor$; frame number $FF$ wraps correctly at $24$ frames without drifting. |
| 3 | Frame Stepping | Backward step at position $t = 0.0\text{s}$ | Clamps at $0.0\text{s}$, maintains paused state, does not crash or seek to negative timestamps. |
| 4 | Frame Stepping | Forward step at last frame ($t \ge \text{duration} - 1/\text{FPS}$) | Clamps at EOF, presents final frame, does not wrap around unless repeat single is active. |
| 5 | Seeking | Seeking past end of file ($t > \text{duration}$) | Clamps to duration, triggers track finished event / advances to next playlist track. |
| 6 | Seeking | Rapid scrub bar dragging (high-frequency seek requests) | Drops intermediate seeks or uses fast keyframe seek during drag; executes final exact seek on mouse release. |
| 7 | Variable Speed | Speed transition while audio buffer is full | Flushes or cross-fades audio resampler buffer to avoid audible popping or click artifacts. |
| 8 | Subtitle Parsing | Malformed timestamp in `.srt` (e.g. `00:01:99,000` or missing arrow `-->`) | Skips malformed cue gracefully, logs parse warning, continues parsing subsequent cues. |
| 9 | Subtitle Parsing | Non-UTF-8 encoded `.srt` file (e.g. Windows-1252 / ISO-8859-1) | Auto-detects encoding via BOM / heuristic fallback, decodes text cleanly without replacement character corruption. |
| 10 | LRC Teleprompter | LRC file with out-of-order timestamps | Automatically sorts cue timestamps chronologically upon loading so binary search stays consistent. |
| 11 | LRC Teleprompter | Multiple timestamps on a single line (`[00:10.00][00:20.00] Chorus`) | Generates distinct cue entries for each timestamp pointing to the same text string. |
| 12 | VU Meter DSP | Complete digital silence (all samples $0.0$) | $P = 0 \implies \text{dBFS} = 20 \log_{10}(10^{-6}) = -120\text{ dB}$; clamped display to minimum meter limit $-60\text{ dB}$. |
| 13 | VU Meter DSP | Audio signal clipping ($|x[n]| > 1.0$) | Meter hits maximum $+3.0\text{ dB}$ headroom mark, lights up Safety Orange `#FF4400` clipping indicator. |
| 14 | Graphic Equalizer | Extreme gain boost ($+12\text{ dB}$ on all bands simultaneously) | Applies soft limiting or master gain compensation to prevent hard digital clipping distortion. |
| 15 | Graphic Equalizer | `Reset Flat` clicked while playback active | Smoothly interpolates filter coefficients back to unity gain ($0\text{ dB}$) without audio dropout. |
| 16 | MPRIS2 D-Bus | D-Bus session bus unavailable (e.g. headless CI container without daemon) | Detects connection failure gracefully, continues local GUI/CLI operation without crashing; headless test harnesses invoke `dbus-run-session`. |
| 17 | MPRIS2 D-Bus | Concurrent external D-Bus `Seek` and internal UI playback update | Serializes state transitions via thread-safe signals/slots; broadcasts `Seeked` signal with confirmed position. |
| 18 | Playlist Matrix | Dropping a mix of media files, directories, and non-media files (e.g. `.txt`, `.pdf`) | Ingests supported media files and directories; silently ignores unsupported non-media files. |
| 19 | State Persistence | Corrupt / truncated `config.json` on disk | Catches JSON parse exception, backs up corrupt file to `config.json.bak`, writes fresh default config. |
| 20 | State Persistence | Missing parent directories for `$XDG_CONFIG_HOME/penguin` | Recursively creates parent directories (`mkdir -p`) before writing configuration or SQLite database. |
| 21 | State Persistence | Simultaneous app instances writing to SQLite database | Opens SQLite with `WAL` (Write-Ahead Logging) mode and busy timeout ($5000\text{ms}$) to prevent `SQLITE_BUSY` lockups. |
| 22 | Headless Testing | Execution without X11 or Wayland display server (`$DISPLAY` unset) | Launches with `QT_QPA_PLATFORM=offscreen`, runs all test suites, and terminates cleanly with exit code 0. |

---

## 8. Acceptance Criteria Matrix & Verification Traceability

| Category | ID | Acceptance Criterion | Verification Method |
|---|---|---|---|
| **Playback & Engine** | `AC-ENG-01` | Successfully plays test audio files (MP3, FLAC, Opus, AAC, WAV) and video files (MP4, MKV, WebM, AVI) with duration and position tracking. | Headless & GUI playback tests with test fixtures. |
| **Playback & Engine** | `AC-ENG-02` | Seeking is millisecond-accurate; frame stepping (`< 1F`, `1F >`) and speed adjustment ($0.5\times–2.0\times$) operate without audio/video desync. | Automated timecode & frame verification test suite. |
| **Playback & Engine** | `AC-ENG-03` | Audio and subtitle track selectors discover all embedded tracks and dynamically switch between them. | Multi-track MKV/MP4 track switching test. |
| **Playback & Engine** | `AC-ENG-04` | External subtitle files (.srt, .ass, .vtt) load dynamically and render on the video surface. | Subtitle loader unit test & sidecar auto-discovery test. |
| **Brutalist UI** | `AC-UI-01` | Launches with full Brutalist dark theme styling matching exact color palette (`#070709`, `#0B0B0E`, `#1E1E24`, `#FF4400`, `#CCFF00`). | Visual inspection & QSS stylesheet property test. |
| **Brutalist UI** | `AC-UI-02` | Seamless dynamic switching between Video Viewfinder Mode and Audio Hi-Fi Deck Mode. | Mode switch controller test & layout geometry check. |
| **Brutalist UI** | `AC-UI-03` | Scrubber tick ruler updates in real time with formatted SMPTE timecode (`HH:MM:SS:FF`) and remaining time. | SMPTE formatter unit test & scrubber position test. |
| **Brutalist UI** | `AC-UI-04` | Video mode displays safe-area reticles and customizable OSD telemetry HUD (FPS, dropped frames, bitrate, resolution, render time). | Viewfinder reticle overlay & telemetry HUD test. |
| **Brutalist UI** | `AC-UI-05` | Audio mode displays animated stereo peak VU meters (CH_L & CH_R) and line-by-line synchronized `.lrc` teleprompter. | VU meter DSP math test & LRC teleprompter sync test. |
| **Brutalist UI** | `AC-UI-06` | 10-band graphic equalizer sliders adjust frequency gain with presets and tactile Flat reset. | Equalizer filter response test & preset validation. |
| **Desktop & MPRIS2** | `AC-DESK-01` | MPRIS2 D-Bus interface registers `org.mpris.MediaPlayer2.penguin` and responds to Play, Pause, Next, Previous, Seek, and updates metadata. | D-Bus introspection & `gdbus` / `pydbus` method call test. |
| **Desktop & MPRIS2** | `AC-DESK-02` | Launching from CLI with media path (e.g. `penguin /path/to/media.mp4`) opens and begins playback immediately. | CLI argument parsing test & process invocation test. |
| **Desktop & MPRIS2** | `AC-DESK-03` | Keyboard shortcuts (Space, Left/Right seek, F fullscreen, M mute, `<`/`>` frame step) operate reliably. | Key event simulation test suite. |
| **Desktop & MPRIS2** | `AC-DESK-04` | Desktop entry (`penguin.desktop`) passes validation and references scalable SVG icon. | `desktop-file-validate` on desktop file. |
| **Persistence** | `AC-PERS-01` | Supports file open, directory open, and drag-and-drop file ingestion. | File I/O & MIME drop event tests. |
| **Persistence** | `AC-PERS-02` | Persists volume, mute, playback speed, equalizer gains, recent history, and window geometry across application restarts. | Config JSON & SQLite database verification test. |
| **Test & Quality** | `AC-TEST-01` | Automated test suite passes with 0 errors in headless mode (`QT_QPA_PLATFORM=offscreen`). | `penguin --test` and `tests/run_all_tests.sh` execution. |
| **Test & Quality** | `AC-TEST-02` | Modular, documented C++ codebase with zero stubs, mocks, or placeholders in production paths. | Forensic codebase audit & static analysis. |

---

*End of Specification Report — Penguin Desktop Media Player*
