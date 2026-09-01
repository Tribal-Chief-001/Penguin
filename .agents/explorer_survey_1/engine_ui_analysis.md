# Playback Engine & UI Architecture Technical Analysis

**Project**: Penguin Media Player  
**Role**: Explorer 1 (Playback Engine & UI Architecture Specialist)  
**Date**: 2026-08-31  
**Status**: Complete  

---

## 1. System Environment & Library Ecosystem Feasibility

A comprehensive audit of the execution environment, compiler toolchains, multimedia frameworks, and Python libraries was conducted.

### 1.1 Environment Inventory & Capabilities Matrix

| Component / Library | Version / Path | Status | Capabilities & Role in Penguin |
|---|---|---|---|
| **Python Runtime** | 3.12.3 (`/usr/bin/python3`) | **Installed** | Modern Python 3.12 with asyncio, ctypes, sqlite3, typing |
| **Qt 6 C++ Framework** | 6.4.2 (`qmake6`, `g++ 13.3.0`) | **Installed** | `Qt6Core`, `Qt6Gui`, `Qt6Widgets`, `Qt6OpenGL`, `Qt6OpenGLWidgets`, `Qt6DBus` |
| **libmpv2** | 0.37.0 (`/lib/x86_64-linux-gnu/libmpv.so.2`) | **Installed** | Universal demuxer/decoder, OpenGL render context, frame stepping, exact seeking, lavfi filter graph, subtitle rendering |
| **GStreamer** | 1.24.2 (`gi.repository.Gst`) | **Installed** | `playbin3`, `equalizer-10bands`, `level` (60Hz VU meter), `pipewiresink`, `pulsesink` |
| **PyGObject (`gi`)** | 3.48.2 | **Installed** | Python GStreamer 1.0, GTK 3.0, GLib/GObject bindings |
| **FFmpeg / FFprobe** | 6.1.1 (`/usr/bin/ffmpeg`) | **Installed** | CLI format probing, lavfi filter verification, media test generation |
| **PyCairo (`cairo`)** | 1.18.0 | **Installed** | High-performance vector drawing engine |
| **D-Bus (`python3-dbus`)**| 1.3.2 (`dbus-daemon`, `dbus-run-session`) | **Installed** | MPRIS2 `org.mpris.MediaPlayer2` service registration & metadata broadcasting |
| **Audio Server** | PipeWire 1.0.5 / PulseAudio Client | **Installed** | Low-latency desktop audio routing via `libpulse` / `pipewiresink` / mpv `ao=pulse,pipewire` |
| **Metadata Tagging** | `mutagen` 1.47, `eyed3` 0.9.7 | **Installed** | ID3v2, Vorbis, FLAC, MP4 metadata and embedded cover art extraction |
| **Graphics Hardware** | Mesa Intel UHD Graphics (OpenGL 4.6) | **Available** | Hardware acceleration, EGL/GLX, offscreen QPA for headless CI |

### 1.2 Package & Deployment Assessment
- **C++ Qt6 Direct Build**: Qt6 C++ toolchain (`qmake6` / `g++`) is fully installed and compiled offscreen test binaries with zero external network dependencies.
- **Python Architecture**: Python 3.12 bindings connect seamlessly to `libmpv.so.2` via `ctypes` (or C-extension) and GStreamer 1.24 via `gi.repository.Gst`.
- **Display Compatibility**: Supports X11 (`xcb`), Wayland (`wayland`), and headless offscreen execution (`QT_QPA_PLATFORM=offscreen` / xvfb) for automated test suites.

---

## 2. Playback Backend Architecture Evaluation

### 2.1 Backend Architecture Comparison

```
+-------------------------------------------------------------------------------+
|                             Penguin Application Core                          |
+-------------------------------------------------------------------------------+
                                      |
                       +-----------------------------+
                       |   MediaPlayerBackend (IF)   |
                       +-----------------------------+
                               /              \
         +-----------------------+          +-----------------------+
         |      MpvBackend       |          |    GStreamerBackend   |
         |  (Primary Production) |          | (Alternative / Tests) |
         +-----------------------+          +-----------------------+
         | - libmpv C API        |          | - GstPlayBin3         |
         | - OpenGL FBO / wid    |          | - GstVideoOverlay     |
         | - Exact seek/step     |          | - equalizer-10bands   |
         | - libass Subtitles    |          | - level VU messager   |
         +-----------------------+          +-----------------------+
```

### 2.2 Primary Backend: `libmpv` Architecture Details

1. **Format & Container Support**:
   - Audio: MP3, FLAC, Opus, AAC, WAV, OGG, ALAC, AIFF, DSD.
   - Video: MP4, MKV, WebM, AVI, MOV, TS, FLV.
   - Hardware Decoding: `hwdec=auto-safe` (VA-API / NVDEC).

2. **Video Rendering Pipeline**:
   - **Method A (Zero-Overhead Window Embedding)**:
     - Widget assigns native X11/Wayland window ID via `wid` property (`mpv_set_option_string(handle, "wid", QString::number(widget->winId()))`).
     - Overlays (reticles, OSD HUD) render on a transparent overlay widget on top of the video container.
   - **Method B (QOpenGLWidget + mpv_render_context)**:
     - `mpv_render_context_create` initialized with `MPV_RENDER_PARAM_OPENGL_INIT_PARAMS`.
     - In `QOpenGLWidget::paintGL()`, `mpv_render_context_render()` renders directly into the Qt OpenGL FBO.
     - Allows direct Qt vector painting (`QPainter`) over the video frame in the same paint pass.

3. **Frame Stepping & Seeking Precision**:
   - **Frame Step Forward**: Command `frame-step`. Advances video by exactly $1/\text{FPS}$ seconds (e.g. 33.33ms @ 30fps).
   - **Frame Step Backward**: Command `frame-back-step`. Accurately steps to previous keyframe/delta-frame.
   - **Exact Millisecond Seeking**: Command `seek <seconds> absolute+exact`. Bypasses keyframe snapping to decode exact target timestamp.
   - **Relative Jumps**: Command `seek +10.0 relative+exact` or `seek -10.0 relative+exact`.

4. **SMPTE Timecode Engine**:
   - Formulas:
     $$\text{Total Frames } N = \text{round}(\text{time\_pos} \times \text{FPS})$$
     $$\text{Hours} = \lfloor N / (3600 \times \text{FPS}) \rfloor, \quad \text{Mins} = \lfloor (N \pmod{3600 \times \text{FPS}}) / (60 \times \text{FPS}) \rfloor$$
     $$\text{Secs} = \lfloor (N \pmod{60 \times \text{FPS}}) / \text{FPS} \rfloor, \quad \text{Frames} = N \pmod{\text{FPS}}$$
   - Formatting: `HH:MM:SS:FF` (standard 24, 25, 30, 60 fps) and drop-frame `HH:MM:SS;FF` (29.97, 59.94 fps).

5. **Track & Subtitle Switching**:
   - Querying: JSON property `track-list` provides full enumeration of all streams (audio, video, subtitles) with codec, language, title, and channel count.
   - Switching: Set property `aid` (audio track ID), `vid` (video track ID), `sid` (subtitle track ID), `secondary-sid` (dual subtitle track ID).
   - External Subtitles: `sub-add <path>` loads external `.srt`, `.ass`, `.vtt` dynamically with `libass` styling, font selection, and synchronization offset (`sub-delay`).

6. **Speed & Pitch Control**:
   - Property `speed` adjustable continuously from `0.5` to `2.0`.
   - Default filter `scaletempo2` preserves audio pitch during playback rate modification.

---

## 3. Audio Pipeline & DSP Architecture

### 3.1 10-Band Graphic Equalizer

```
+----------------------------------------------------------------------------------------------------+
|                                    10-Band Equalizer Frequency Rack                                |
|  [31.25Hz] [62.5Hz] [125Hz] [250Hz] [500Hz] [1.0kHz] [2.0kHz] [4.0kHz] [8.0kHz] [16.0kHz]          |
|    -12dB        -12dB      -12dB     -12dB     -12dB      -12dB      -12dB     -12dB     -12dB    -12dB  |
|      to           to         to        to        to         to         to        to        to       to   |
|    +12dB        +12dB      +12dB     +12dB     +12dB      +12dB      +12dB     +12dB     +12dB    +12dB  |
+----------------------------------------------------------------------------------------------------+
```

#### Equalizer Implementation Options:
1. **mpv `lavfi` Filter Graph**:
   - Filter parameter string:
     `lavfi=[equalizer=f=31.25:width_type=o:w=1:g=G0,equalizer=f=62.5:width_type=o:w=1:g=G1,equalizer=f=125:width_type=o:w=1:g=G2,equalizer=f=250:width_type=o:w=1:g=G3,equalizer=f=500:width_type=o:w=1:g=G4,equalizer=f=1000:width_type=o:w=1:g=G5,equalizer=f=2000:width_type=o:w=1:g=G6,equalizer=f=4000:width_type=o:w=1:g=G7,equalizer=f=8000:width_type=o:w=1:g=G8,equalizer=f=16000:width_type=o:w=1:g=G9]`
   - Dynamically applied via `mpv_set_property_string(handle, "af", filter_string)` without audio dropouts.

2. **GStreamer `equalizer-10bands`**:
   - Element `equalizer-10bands` provides direct properties `band0` through `band9` with gain in dB (float).

3. **Software Biquad IIR DSP (RBJ Peaking EQ Filter)**:
   - For standalone unit testing and signal simulation:
     $$A = 10^{\text{gain\_db} / 40}, \quad \omega_0 = 2\pi f_0 / f_s, \quad \alpha = \frac{\sin(\omega_0)}{2Q}$$
     $$b_0 = 1 + \alpha A, \quad b_1 = -2\cos(\omega_0), \quad b_2 = 1 - \alpha A$$
     $$a_0 = 1 + \alpha / A, \quad a_1 = -2\cos(\omega_0), \quad a_2 = 1 - \alpha / A$$
     $$y[n] = \frac{b_0}{a_0}x[n] + \frac{b_1}{a_0}x[n-1] + \frac{b_2}{a_0}x[n-2] - \frac{a_1}{a_0}y[n-1] - \frac{a_2}{a_0}y[n-2]$$

#### Factory Presets Definition:
- **Flat**: `[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]`
- **Rock**: `[+4.5, +3.5, +2.0, 0.0, -1.5, -1.0, +1.0, +2.5, +4.0, +4.5]`
- **Pop**: `[-1.5, +1.0, +3.0, +4.0, +3.0, 0.0, -1.0, -1.5, +1.0, +2.0]`
- **Jazz**: `[+3.0, +2.0, +1.0, +1.5, -1.5, -1.5, 0.0, +1.5, +2.5, +3.5]`
- **Classical**: `[+4.0, +3.0, +2.0, +1.5, -1.0, -1.0, 0.0, +2.0, +3.0, +3.5]`
- **Bass Boost**: `[+6.0, +5.0, +4.0, +2.5, +1.0, 0.0, 0.0, 0.0, 0.0, 0.0]`
- **Vocal**: `[+1.0, -1.0, -2.0, +2.0, +4.5, +4.5, +3.0, +1.0, 0.0, -1.0]`
- **Electronic**: `[+4.0, +3.5, +1.0, 0.0, -1.5, +2.0, +1.0, +2.5, +4.0, +4.0]`

---

### 3.2 Stereo Peak VU Meter Engine

1. **Signal Metric Extraction**:
   - **Peak Level**: $\text{Peak} = \max_{i \in [0, N-1]} |x[i]|$
   - **RMS Level**: $\text{RMS} = \sqrt{\frac{1}{N}\sum_{i=0}^{N-1} x[i]^2}$
   - **Decibel Conversion (dBFS)**: $\text{dB} = 20 \log_{10}(\text{Peak})$, clamped to range $[-60.0\text{ dBFS}, 0.0\text{ dBFS}]$.
   - **Normalized Metric**: $\text{norm} = \text{clamp}\left(\frac{\text{dB} - (-60.0)}{60.0}, 0.0, 1.0\right)$.

2. **Ballistics & Dynamics**:
   - **Attack Phase**: Instantaneous response ($\approx 0\text{ ms}$).
   - **Decay Phase**: Linear/exponential fall-off at $20\text{ dB/sec}$ ($\Delta V = 20 \times \Delta t$).
   - **Peak-Hold Marker**: Holds max peak for $1.2\text{ s}$ before decaying at $30\text{ dB/sec}$.
   - **Segmented LED Color Ramp**:
     - $-60\text{ dB}$ to $-18\text{ dB}$: Obsidian / Dark Lime (`#2A4A1A` / `#4E7A27`)
     - $-18\text{ dB}$ to $-3\text{ dB}$: Signal Lime (`#CCFF00`)
     - $-3\text{ dB}$ to $0\text{ dB}$ (and overload $+3\text{ dB}$): Safety Orange (`#FF4400`) & Clip Red (`#FF1133`)

---

### 3.3 Synchronized `.lrc` Teleprompter Engine

1. **Parser Specification**:
   - Supports timestamp formats `[mm:ss.xx]` (centiseconds) and `[mm:ss.xxx]` (milliseconds).
   - Extracts metadata tags: `[ti:Title]`, `[ar:Artist]`, `[al:Album]`, `[by:Author]`, `[offset:+/-ms]`.
   - Resolves multi-timestamp lines (e.g. `[00:12.00][00:24.00]Chorus line`).
   - Produces a chronologically sorted sequence of `(time_sec, text)` tuples.

2. **Real-Time Active Line Tracking**:
   - Binary search (`bisect_right`) on sorted timestamp array to find index $k$ such that $t_k \le \text{time\_pos} < t_{k+1}$ in $O(\log M)$ time.
   - Smooth viewport centering with easing animation so active line remains in the vertical sweet spot.
   - Interactive seek: Clicking any lyric row dispatches `seek(t_k)` to the playback backend.

---

## 4. UI Architecture: Tactile Digital Brutalism

### 4.1 Design System Tokens & Hierarchy

```
+---------------------------------------------------------------------------------+
|                                 DESIGN SYSTEM TOKENS                            |
+-------------------+-----------------------------+-------------------------------+
| Token Class       | Hex Value                   | Semantic Usage                |
+-------------------+-----------------------------+-------------------------------+
| Surface Darkest   | #070709                     | Root window background canvas |
| Surface Panel     | #0B0B0E                     | Control docks, card containers|
| Surface Raised    | #141418                     | Sliders, buttons, badges      |
| Grid Line Sharp   | #1E1E24 (1px solid)         | Structural separation borders |
| Grid Line Active  | #333342                     | Focused element borders       |
| Primary Text      | #FFFFFF                     | Track title, active timecode  |
| Secondary Text    | #8A8A9A                     | Labels, artist, passive units |
| Muted Text        | #4A4A5A                     | Inactive lyrics, ticks        |
| Signal Lime       | #CCFF00                     | Active lyric, VU high level   |
| Safety Orange     | #FF4400                     | Playhead, active playback indicator |
| Cyan Accent       | #00F0FF                     | Audio deck accent, telemetry  |
+-------------------+-----------------------------+-------------------------------+
```

### 4.2 Mode 1: Video Viewfinder Mode Layout

```
+---------------------------------------------------------------------------------+
| [PENGUIN // VIEWFINDER MODE]                                  [⛶ FULL] [⚙ MODE] |
+---------------------------------------------------------------------------------+
| + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - + |
| | [OSD TELEMETRY]                                                             | |
| | FPS: 30.00 | DROP: 0 | H.264 1080p | 1,411 kbps | RENDER: 1.8ms | SMPTE: .. | |
| |                                                                             | |
| |                +-----------------------------------------+                  | |
| |                |  [ ACTION SAFE RETICLE (90%) ]          |                  | |
| |                |      +---------------------------+      |                  | |
| |                |      | [ TITLE SAFE (80%) ]      |      |                  | |
| |                |      |            +              |      |                  | |
| |                |      |      (CENTER CROSS)       |      |                  | |
| |                |      +---------------------------+      |                  | |
| |                +-----------------------------------------+                  | |
| + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - + |
+---------------------------------------------------------------------------------+
| 00:00:00:00  ||||||||||||||||||▼|||||||||||||||||||||||||||||||||  -00:02:15:12 |
|               00:00:45:00 (SMPTE TICK RULER PROGRESS SCRUBBER)                  |
+---------------------------------------------------------------------------------+
| [ < 1F ] [ 1F > ] | [ -10S ] [ ▶ PLAY ] [ +10S ] | SPEED: [ 1.0x ▼ ] | [ ♫ ] [ 🗩 ] |
+---------------------------------------------------------------------------------+
```

#### Core Components in Viewfinder Mode:
1. **`ViewfinderVideoWidget`**: Hosts video canvas via OpenGL texture rendering or native window embedding.
2. **`ReticleOverlayWidget`**: Transparent paint overlay with 1px dashed Action/Title safe brackets, center crosshair, and aspect ratio crop lines.
3. **`OSDTelemetryHUD`**: Monospace technical diagnostics overlay displaying real-time FPS, dropped frames, bitrate, resolution, color matrix, audio bitrate, and SMPTE clock.
4. **`MechanicalTickRulerScrubber`**:
   - Custom QPainter widget drawing 1px graduation ticks (major, minor, sub-frame).
   - Industrial safety orange playhead needle with real-time SMPTE tooltip bubble.
   - Chapter marker diamond pins on the timeline.
   - Dual time displays (Elapsed SMPTE on left, Remaining SMPTE on right).
5. **`TactileControlDock`**: Sharp 1px bordered brutalist push-buttons with tactile hover and active orange indicators.

---

### 4.3 Mode 2: Hi-Fi Audio Deck Mode Layout

```
+---------------------------------------------------------------------------------+
| [PENGUIN // HI-FI AUDIO DECK]                                 [QUEUE] [⚙ MODE]  |
+----------------------------------------------------+----------------------------+
| [NOW PLAYING DISPLAY]                              | [STEREO PEAK VU METER]     |
| TITLE:  STUDIO PRECISION 01                        | CH_L [|||||||||||||||░░]   |
| ARTIST: BRUTALIST SOUND SYSTEM                     | CH_R [||||||||||||||||░]   |
| ALBUM:  PENGUIN MASTER ARCHIVE (2026)              | -60 -40 -20 -12 -6 -3 0 dB |
| FORMAT: FLAC // 24-BIT // 96.0 kHz // 2,840 kbps   +----------------------------+
+----------------------------------------------------+ [10-BAND GRAPHIC EQ RACK] |
| [SYNCHRONIZED .LRC TELEPROMPTER]                   |  +12 |   |   |   |   |   | |
|   00:01.60  Initial system startup...              |    0 | | | | | | | | | | | |
| ► 00:04.30  Observing raster frame buffer          |  -12 |   |   |   |   |   | |
|   00:08.60  Tactile feedback engaged               |      32 64 125 500 1k 4k 16k|
|   00:14.10  Telemetry readout nominal              | [PRESETS: ROCK POP FLAT]   |
+----------------------------------------------------+----------------------------+
| [PLAYLIST MATRIX / QUEUE]                                                       |
| 01 | Studio Precision 01     | Brutalist Sound System | 03:45 | FLAC 24/96      |
| 02 | Obsidian Grid Monolith  | Tactile Signal Lab     | 04:12 | FLAC 24/48      |
+---------------------------------------------------------------------------------+
| [ ❚❚ PAUSE ] [⏮ PREV] [⏭ NEXT] [ 🔁 LOOP ] [ 🔀 SHUFFLE ] | VOL: [||||||||░░] 80% |
+---------------------------------------------------------------------------------+
```

#### Core Components in Audio Deck Mode:
1. **`TypographicTrackCard`**: Monolithic high-contrast track metadata block with format tags.
2. **`StereoVUMeterWidget`**: Real-time dual LED ladder meters with logarithmic dB scaling, peak-hold decay, and signal lime / safety orange color gradients.
3. **`EqualizerRackWidget`**: 10 vertical precision faders with center detents, numeric gain readouts, and quick-preset switcher.
4. **`LyricsTeleprompterWidget`**: Smooth kinetic auto-scrolling lyrics reader with active row spotlight in signal lime (`#CCFF00`) and click-to-seek support.
5. **`PlaylistMatrixWidget`**: Brutalist monospace data table supporting drag-and-drop file import, track reordering, and search filtering.

---

## 5. Linux Desktop System Integration Architecture

### 5.1 MPRIS2 D-Bus Architecture (`org.mpris.MediaPlayer2.penguin`)

1. **Interfaces Implemented**:
   - `org.mpris.MediaPlayer2`: Root properties (`Identity`, `CanQuit`, `CanRaise`, `SupportedMimeTypes`).
   - `org.mpris.MediaPlayer2.Player`: Playback controls (`Play`, `Pause`, `PlayPause`, `Stop`, `Next`, `Previous`, `Seek`, `SetPosition`, `OpenUri`).
   - Properties: `PlaybackStatus` (`Playing`, `Paused`, `Stopped`), `Metadata` (`mpris:trackid`, `mpris:length`, `xesam:title`, `xesam:artist`, `xesam:album`, `xesam:artUrl`), `Position`, `Volume`, `Rate`, `CanSeek`, `CanPlay`, `CanPause`, `CanControl`.

2. **Desktop File & CLI Invocation**:
   - Command-Line Arguments:
     `penguin [file_or_url] [--audio] [--video] [--fullscreen] [--volume=<0-100>] [--help]`
   - XDG Desktop Entry: `penguin.desktop` registering MIME types for all common media formats.

---

## 6. Architecture Synthesis & Recommendations

### Summary Recommendation:
1. **Core Backend**: `libmpv.so.2` provides industry-leading codec compatibility, frame-accurate stepping, millisecond seeking, and audio filter support.
2. **Alternative Backend**: GStreamer 1.24 fallback pipeline with `equalizer-10bands` and `level` plugin.
3. **GUI Framework**:
   - C++ Qt6 (`QtWidgets` + custom `QPainter` widgets + `QOpenGLWidget`) provides native performance, 60fps animations, and zero styling overhead.
   - Python GTK3/Cairo or Qt6 bindings provide matching capabilities.
4. **Testing Architecture**:
   - Headless verification using `QT_QPA_PLATFORM=offscreen` and programmatic backend simulation scripts ensuring 100% automated test coverage without requiring a physical monitor.

