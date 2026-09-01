# Penguin System Integration, Persistence & Automated Test Architecture Analysis

**Author:** Explorer 2 (System Integration & Test Automation Specialist)  
**Date:** 2026-08-31  
**Project:** Penguin Media Player  
**Target Platform:** Linux Desktop (GNOME, KDE Plasma, XFCE, Sway, Hyprland, Wayland/X11, PipeWire/PulseAudio)

---

## 1. Executive Summary & Problem Space

Penguin is an advanced, dual-mode media player (Viewfinder Video & Hi-Fi Audio Deck) designed for the modern Linux desktop with a Tactile Digital Brutalist aesthetic. To achieve first-class desktop citizenship and carrier-grade stability, the system integration and verification layer must solve four fundamental challenges:

1. **Seamless MPRIS2 D-Bus Compliance:** Full compatibility with desktop media controllers (GNOME Shell, KDE Media Player applet, playerctl, waybar, mpris-tail, hardware media keys, Bluetooth headset controls) without thread lockups or mainloop contention.
2. **Standard Desktop Packaging & Robust CLI:** XDG compliance (.desktop entries, MIME associations, icon theming) and a flexible CLI interface supporting single-instance IPC, file queuing, and playback parameter overrides.
3. **Robust State Persistence:** Atomic, high-performance local database storage (SQLite WAL + JSON) tracking window geometry, volume, playback positions, queue state, and equalizer presets across restarts.
4. **Comprehensive Headless Test Automation:** 100% automated verification in display-less CI/CD environments and developer sandboxes using offscreen Qt rendering, ephemeral private D-Bus sessions, and zero-network synthetic media generation (MP4, MKV multi-stream, MP3, FLAC, WAV, .lrc, .srt).

---

## 2. MPRIS2 D-Bus Integration Architecture

### 2.1 Specification Breakdown

The Media Player Remote Interfacing Specification (MPRIS) v2.2 standardizes D-Bus remote control for media players. Penguin must register on the **Session Bus** under:

- **Bus Name:** `org.mpris.MediaPlayer2.penguin` (or `org.mpris.MediaPlayer2.penguin.instance_PID` when supporting parallel instances)
- **Object Path:** `/org/mpris/MediaPlayer2`

Penguin must implement three standardized interfaces on `/org/mpris/MediaPlayer2`:

```
/org/mpris/MediaPlayer2
├── org.mpris.MediaPlayer2           (Root interface: application lifecycle & capabilities)
├── org.mpris.MediaPlayer2.Player    (Player interface: transport, metadata, volume, position)
└── org.freedesktop.DBus.Properties  (Standard property access & change notification signals)
```

#### Detailed Interface Contracts

##### 1. `org.mpris.MediaPlayer2` (Root)
| Member | Type | Signature | Description |
|---|---|---|---|
| `Raise()` | Method | `() -> ()` | Brings the Penguin UI window to the foreground |
| `Quit()` | Method | `() -> ()` | Gracefully terminates the application |
| `CanQuit` | Property (ro) | `b` | Always `true` |
| `Fullscreen` | Property (rw) | `b` | `true` if window is in fullscreen mode |
| `CanSetFullscreen` | Property (ro) | `b` | `true` |
| `CanRaise` | Property (ro) | `b` | `true` |
| `HasTrackList` | Property (ro) | `b` | `false` (basic player) or `true` if TrackList interface implemented |
| `Identity` | Property (ro) | `s` | `"Penguin"` |
| `DesktopEntry` | Property (ro) | `s` | `"penguin"` (corresponds to `penguin.desktop`) |
| `SupportedUriSchemes`| Property (ro) | `as` | `["file", "http", "https"]` |
| `SupportedMimeTypes` | Property (ro) | `as` | `["audio/mpeg", "audio/flac", "audio/x-wav", "video/mp4", "video/x-matroska", "video/webm"]` |

##### 2. `org.mpris.MediaPlayer2.Player` (Transport & Playback)
| Member | Type | Signature | Description |
|---|---|---|---|
| `Next()` | Method | `() -> ()` | Skips to the next track in the playlist |
| `Previous()` | Method | `() -> ()` | Skips to previous track or restarts current track |
| `Pause()` | Method | `() -> ()` | Pauses playback |
| `PlayPause()` | Method | `() -> ()` | Toggles play/pause state |
| `Stop()` | Method | `() -> ()` | Stops playback and resets position |
| `Play()` | Method | `() -> ()` | Starts or resumes playback |
| `Seek(x)` | Method | `(x) -> ()` | Relative seek by offset in microseconds (`x: int64`) |
| `SetPosition(o, x)`| Method | `(o, x) -> ()` | Absolute seek to position `x` (microseconds) for track `o` (track_id) |
| `OpenUri(s)` | Method | `(s) -> ()` | Opens URI (file path or HTTP stream) and begins playback |
| `Seeked(x)` | Signal | `(x)` | Emitted when playback position changes abruptly (value in microseconds) |
| `PlaybackStatus` | Property (ro) | `s` | `"Playing"`, `"Paused"`, or `"Stopped"` |
| `LoopStatus` | Property (rw) | `s` | `"None"`, `"Track"`, or `"Playlist"` |
| `Rate` | Property (rw) | `d` | Playback speed multiplier (e.g. `1.0`, `1.5`, `0.5`) |
| `Shuffle` | Property (rw) | `b` | Shuffle playlist state (`true` / `false`) |
| `Metadata` | Property (ro) | `a{sv}` | Track metadata dictionary (see specification table below) |
| `Volume` | Property (rw) | `d` | Audio volume: `0.0` (0%) to `1.0` (100%) or `1.5` (150% boost) |
| `Position` | Property (ro) | `x` | Current playback position in microseconds (`int64`) |
| `MinimumRate` | Property (ro) | `d` | `0.5` |
| `MaximumRate` | Property (ro) | `d` | `2.0` |
| `CanGoNext` | Property (ro) | `b` | `true` if next track exists |
| `CanGoPrevious`| Property (ro) | `b` | `true` if previous track exists |
| `CanPlay` | Property (ro) | `b` | `true` when playable track loaded |
| `CanPause` | Property (ro) | `b` | `true` when track playing |
| `CanSeek` | Property (ro) | `b` | `true` for seekable media |
| `CanControl` | Property (ro) | `b` | `true` |

##### Metadata Dictionary Keys (`Metadata: a{sv}`)
| Key | Type | Example |
|---|---|---|
| `mpris:trackid` | `o` (DBus Object Path) | `/org/mpris/MediaPlayer2/Track/0` or `/org/penguin/track/12345` |
| `mpris:length` | `x` (int64 microseconds) | `240000000` (4 minutes) |
| `mpris:artUrl` | `s` (URI) | `file:///tmp/penguin_art/cover.jpg` or `http://...` |
| `xesam:title` | `s` (string) | `"Cybernetic Horizon"` |
| `xesam:artist` | `as` (array of strings) | `["Apex Synth", "Studio Duo"]` |
| `xesam:album` | `s` (string) | `"Brutalist Echoes"` |
| `xesam:albumArtist` | `as` (array of strings) | `["Apex Synth"]` |
| `xesam:genre` | `as` (array of strings) | `["Electronic", "Synthwave"]` |
| `xesam:url` | `s` (string URI) | `file:///home/user/Music/track.flac` |
| `xesam:userRating` | `d` (double 0.0-1.0) | `1.0` |

##### 3. `org.freedesktop.DBus.Properties`
Whenever `PlaybackStatus`, `Metadata`, `Volume`, `Rate`, or `LoopStatus` change, Penguin must emit the signal:
`PropertiesChanged(interface_name: "org.mpris.MediaPlayer2.Player", changed_properties: {property_name: value}, invalidated_properties: [])`

---

### 2.2 Evaluation of Python D-Bus Implementations

| Implementation | Event Loop Integration | Pros | Cons | Recommendation |
|---|---|---|---|---|
| **`QtDBus` (`PyQt6.QtDBus` / `PySide6.QtDBus`)** | Native Qt Event Loop | Zero external thread bridges; signals emit synchronously via Qt signals; native QDBusAbstractAdaptor | Complex Variant packaging in pure Python bindings | **Primary for Qt GUI Mode** |
| **`dbus-python` (`python3-dbus`)** | GLib / Qt via `dbus.mainloop` | Ubiquitous across Linux distros; battle-tested C-bindings; strict type safety (`dbus.Int64`, `dbus.Dictionary`) | Requires main loop integration; legacy C codebase | **Top-tier headless/server fallback** |
| **`jeepney`** | Pure Python / Asyncio / Trio | Pure Python (no C library dependencies); easy unit testing over mock sockets | No built-in high-level service object adaptor; boilerplate for introspection XML | **Great for lightweight client testing** |
| **`dasbus`** | GLib / PyGObject | Modern pythonic decorator syntax (`@dbus_interface`); clean variant serialization | Requires PyGObject (`gi.repository.GLib`) main loop | **Viable alternative** |
| **`dbus-next`** | Pure Python / Asyncio | Excellent asynchronous service publishing; typed signatures | Third-party dependency | **Alternative** |

### 2.3 Proposed MPRIS Architecture: Decoupled Adapter Pattern

To ensure Penguin runs reliably both on full desktop sessions with D-Bus and in headless / container / CI environments where D-Bus may be unavailable or unconfigured, we design an **Abstract MPRIS Adapter**:

```
                  ┌─────────────────────────────────────┐
                  │        Penguin Playback Engine      │
                  │  (Signals: position, state, meta)   │
                  └──────────────────┬──────────────────┘
                                     │
                                     ▼
                  ┌─────────────────────────────────────┐
                  │         MPRISService Bridge         │
                  │     (Normalizes values & types)     │
                  └──────┬───────────────────────┬──────┘
                         │                       │
           [If D-Bus Bus Available]    [If Headless / Mock / No Bus]
                         │                       │
                         ▼                       ▼
            ┌────────────────────────┐  ┌─────────────────────────┐
            │   DBusMPRISAdapter     │  │     NullMPRISAdapter    │
            │ (QtDBus / dbus-python) │  │  (In-Memory Mock State  │
            │ Registers service on   │  │   for 100% CI Testing)  │
            │  Session D-Bus Bus     │  └─────────────────────────┘
            └────────────────────────┘
```

#### Key Implementation Requirements:
1. **Graceful Fallback:** If `QDBusConnection.sessionBus().isConnected()` is `false` or `dbus.SessionBus()` raises `DBusException`, Penguin automatically selects `NullMPRISAdapter`, logs an informational diagnostic message, and continues normal GUI/CLI playback without crashing.
2. **Safe Microsecond Conversion:** Python/Qt timestamps (milliseconds or seconds float) must be strictly converted to `int64` microseconds ($ms \times 1000$) to conform with the MPRIS2 specification.
3. **Strict Volume Normalization:** MPRIS `Volume` must be clamped between `0.0` and `1.0` (or `1.5` max), mapping directly to the internal audio engine scale.

---

## 3. Desktop Packaging & CLI Architecture

### 3.1 FreeDesktop `.desktop` File Specification

File path: `penguin.desktop`  
System target: `/usr/share/applications/penguin.desktop`  
User target: `~/.local/share/applications/penguin.desktop`

```ini
[Desktop Entry]
Version=1.5
Type=Application
Name=Penguin
GenericName=Media Player
Comment=Tactile Digital Brutalist Audio & Video Player
Exec=penguin %U
Icon=penguin
Terminal=false
StartupNotify=true
StartupWMClass=penguin
Categories=AudioVideo;Audio;Video;Player;Recorder;Qt;
MimeType=audio/mpeg;audio/x-wav;audio/flac;audio/ogg;audio/aac;audio/x-matroska;video/mp4;video/x-matroska;video/webm;video/quicktime;video/x-msvideo;application/ogg;application/x-ogg;x-scheme-handler/mpris;
Keywords=audio;video;player;mpris;media;flac;mp3;mp4;mkv;equalizer;lyrics;hifi;brutalist;
Actions=PlayPause;Next;Previous;Stop;

[Desktop Action PlayPause]
Name=Play/Pause
Exec=penguin --action play-pause
Icon=media-playback-start

[Desktop Action Next]
Name=Next Track
Exec=penguin --action next
Icon=media-skip-forward

[Desktop Action Previous]
Name=Previous Track
Exec=penguin --action previous
Icon=media-skip-backward

[Desktop Action Stop]
Name=Stop Playback
Exec=penguin --action stop
Icon=media-playback-stop
```

### 3.2 FreeDesktop Icon Hierarchy

Icons follow the FreeDesktop Icon Theme specification:

```
~/.local/share/icons/hicolor/
├── scalable/apps/penguin.svg        # Infinite precision master vector icon (Brutalist Penguin glyph)
├── symbolic/apps/penguin-symbolic.svg# Monochrome 16px silhouette for GNOME top bar & system tray
├── 512x512/apps/penguin.png
├── 256x256/apps/penguin.png
├── 128x128/apps/penguin.png
├── 64x64/apps/penguin.png
├── 48x48/apps/penguin.png
├── 32x32/apps/penguin.png
└── 16x16/apps/penguin.png
```

### 3.3 Command-Line Interface (CLI) Specification

Penguin provides a fast, rich CLI powered by Python's standard library `argparse`:

```
usage: penguin [-h] [-v] [--audio | --video] [-f] [--volume VOLUME]
               [--speed SPEED] [--loop {none,track,playlist}] [--shuffle]
               [--eq PRESET] [--headless-test] [--dev-telemetry]
               [--action {play-pause,next,previous,stop,raise}]
               [files/urls ...]

Tactile Digital Brutalist Audio & Video Player for Linux.

positional arguments:
  files/urls             Media file paths, directories, or stream URLs to open and play.

options:
  -h, --help             Show this help message and exit.
  -v, --version          Show application version and build metadata.
  --audio                Force Hi-Fi Audio Deck UI Mode.
  --video                Force Viewfinder Video UI Mode.
  -f, --fullscreen       Launch immediately in fullscreen mode.
  --volume VOLUME        Set initial audio volume (0-100).
  --speed SPEED          Set playback speed multiplier (0.5 to 2.0).
  --loop {none,track,playlist}
                         Set playlist loop mode.
  --shuffle              Enable random shuffle mode.
  --eq PRESET            Apply equalizer preset by name (e.g. 'Flat', 'Bass Boost', 'Studio Mastering').
  --headless-test        Run automated self-diagnostics and verification suite without GUI.
  --dev-telemetry        Enable OSD diagnostic HUD overlay (FPS, codec, dropped frames, render latency).
  --action ACTION        Send IPC control action to already-running Penguin instance.
```

### 3.4 Single-Instance IPC Strategy

When a user runs `penguin song2.mp3` or clicks a file in their file manager while Penguin is already running:

1. **Primary Protocol (MPRIS D-Bus):**
   - Probe D-Bus Session Bus for `org.mpris.MediaPlayer2.penguin`.
   - If owned: Call `OpenUri("file:///path/to/song2.mp3")` and `Raise()`.
   - The CLI process prints `"Enqueued into running Penguin instance."` and exits with code `0`.
2. **Secondary Protocol (`QLocalServer` / Unix Socket):**
   - If D-Bus is unavailable, connect to Unix domain socket `/tmp/penguin-$UID/ipc.sock` (or `$XDG_RUNTIME_DIR/penguin.sock`).
   - Send JSON payload: `{"command": "open", "uris": ["..."], "raise": true}`.
   - If no server is listening, acquire socket, bind, and launch GUI.

---

## 4. State Persistence Architecture (SQLite + JSON)

### 4.1 Linux XDG Storage Paths

Following the **XDG Base Directory Specification**:

| Storage Type | Environment Variable | Default Path | Purpose |
|---|---|---|---|
| **Database** | `$XDG_DATA_HOME` | `~/.local/share/penguin/penguin.db` | Playback history, media metadata cache, playlists, equalizer presets |
| **Settings** | `$XDG_CONFIG_HOME` | `~/.config/penguin/config.json` | High-level user preferences, UI theme flags, key bindings |
| **Thumbnails**| `$XDG_CACHE_HOME` | `~/.cache/penguin/art/` | Extracted embedded cover art images and waveform cache |
| **Runtime IPC**| `$XDG_RUNTIME_DIR` | `/run/user/$UID/penguin.sock` | Ephemeral IPC socket and PID lock |

### 4.2 SQLite Database Schema (Production DDL)

Engine configuration:
```sql
PRAGMA journal_mode = WAL;
PRAGMA synchronous = NORMAL;
PRAGMA foreign_keys = ON;
PRAGMA busy_timeout = 5000;
```

#### Complete DDL:

```sql
-- Schema version tracking for seamless future migrations
CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER PRIMARY KEY,
    applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- General Key-Value App Settings (Window geometry, volume, modes)
CREATE TABLE IF NOT EXISTS app_settings (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Media Playback History (Tracks resume positions, play counts, metadata)
CREATE TABLE IF NOT EXISTS media_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    uri TEXT UNIQUE NOT NULL,
    title TEXT,
    artist TEXT,
    album TEXT,
    duration_ms INTEGER DEFAULT 0,
    last_position_ms INTEGER DEFAULT 0,
    play_count INTEGER DEFAULT 1,
    last_played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    media_type TEXT CHECK(media_type IN ('audio', 'video', 'stream')) NOT NULL,
    file_size INTEGER DEFAULT 0,
    cover_art_path TEXT
);

-- Playlists & Current Queue Matrix
CREATE TABLE IF NOT EXISTS playlist_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    playlist_name TEXT NOT NULL DEFAULT 'default',
    position_order INTEGER NOT NULL,
    uri TEXT NOT NULL,
    title TEXT,
    artist TEXT,
    duration_ms INTEGER DEFAULT 0,
    added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(playlist_name, position_order)
);

-- 10-Band Graphic Equalizer Presets
CREATE TABLE IF NOT EXISTS equalizer_presets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,
    band_32hz REAL DEFAULT 0.0,
    band_64hz REAL DEFAULT 0.0,
    band_125hz REAL DEFAULT 0.0,
    band_250hz REAL DEFAULT 0.0,
    band_500hz REAL DEFAULT 0.0,
    band_1khz REAL DEFAULT 0.0,
    band_2khz REAL DEFAULT 0.0,
    band_4khz REAL DEFAULT 0.0,
    band_8khz REAL DEFAULT 0.0,
    band_16khz REAL DEFAULT 0.0,
    preamp REAL DEFAULT 0.0,
    is_builtin BOOLEAN DEFAULT 0
);

-- Create fast indexes
CREATE INDEX IF NOT EXISTS idx_history_last_played ON media_history(last_played_at DESC);
CREATE INDEX IF NOT EXISTS idx_playlist_order ON playlist_items(playlist_name, position_order ASC);
```

### 4.3 Default Equalizer Presets

Penguin initializes the database with 8 studio-tuned factory presets:

| Preset Name | 32Hz | 64Hz | 125Hz | 250Hz | 500Hz | 1kHz | 2kHz | 4kHz | 8kHz | 16kHz | Preamp |
|---|---|---|---|---|---|---|---|---|---|---|---|
| **Flat** | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| **Bass Boost** | +6.0 | +5.0 | +3.5 | +1.5 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | -2.0 |
| **Studio Mastering** | +1.0 | +1.5 | 0.0 | -0.5 | +0.5 | +1.0 | +1.5 | +2.0 | +2.5 | +2.0 | -1.0 |
| **Vocal Clarity** | -2.0 | -1.0 | 0.0 | +1.5 | +3.0 | +3.5 | +3.0 | +1.5 | 0.0 | -1.0 | 0.0 |
| **Electronic / Synth** | +5.5 | +4.5 | +2.0 | 0.0 | -1.0 | +1.5 | +2.5 | +3.5 | +4.5 | +4.0 | -2.0 |
| **Acoustic / Live** | +2.0 | +1.5 | +1.0 | 0.0 | +1.0 | +2.0 | +2.5 | +3.0 | +3.5 | +3.0 | -1.0 |
| **Rock / Metal** | +4.5 | +3.5 | +1.0 | -0.5 | -1.5 | +1.0 | +2.0 | +3.5 | +4.0 | +4.5 | -1.5 |
| **Night Mode (Low Dynamic)** | -4.0 | -3.0 | -1.5 | 0.0 | +1.0 | +2.0 | +2.0 | +1.0 | -1.0 | -3.0 | +1.0 |

### 4.4 App State Lifecycle

- **On Shutdown:**
  - Persist window geometry (`x, y, width, height, is_maximized, is_fullscreen`).
  - Persist current playback position if playing (for resume prompt).
  - Persist active volume and mute state.
  - Persist current playlist queue and active track index.
- **On Startup:**
  - Restore window geometry with multi-monitor sanity clamp (ensuring the window coordinates are on a visible screen).
  - Load active EQ preset and audio volume.
  - Populate playlist queue model.

---

## 5. Comprehensive Automated Test Strategy

### 5.1 Test Architecture & Pyramid

```
                ┌──────────────────────────────────────────────┐
                │          Level 3: Headless CLI Test          │
                │        (penguin --headless-test)             │
                │   End-to-end self-verification runner        │
                └──────────────────────┬───────────────────────┘
                                       │
                ┌──────────────────────────────────────────────┐
                │       Level 2: Headless Integration          │
                │ (QT_QPA_PLATFORM=offscreen, dbus-run-session)│
                │ Synthetic Media Playback, MPRIS2, SQLite WAL │
                └──────────────────────┬───────────────────────┘
                                       │
                ┌──────────────────────────────────────────────┐
                │          Level 1: Core Unit Tests            │
                │ (Pure Python stdlib, zero I/O, math & DSP)   │
                │ LRC Parser, Biquad EQ Math, SMPTE Timecode   │
                └──────────────────────────────────────────────┘
```

---

### 5.2 Zero-Dependency Synthetic Test Media Generation

To guarantee tests never fail due to network hiccups or missing proprietary test files, Penguin includes a programmatic **Synthetic Test Media Generator** using `ffmpeg` lavfi filters and a pure Python `wave` fallback:

#### Synthetic Media Generator Matrix

```python
# test_fixtures/media_factory.py
import subprocess
import wave
import struct
import math
from pathlib import Path

class SyntheticMediaFactory:
    """Generates synthetic media test assets programmatically in < 1 second."""

    @staticmethod
    def create_video_mp4(dest_path: Path, duration_sec: int = 3, fps: int = 24):
        """Generates valid H.264 / AAC MP4 video with SMPTE test pattern."""
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"testsrc=duration={duration_sec}:size=640x360:rate={fps}",
            "-f", "lavfi", "-i", f"sine=frequency=440:duration={duration_sec}",
            "-c:v", "libx264", "-pix_fmt", "yuv420p",
            "-c:a", "aac",
            str(dest_path)
        ]
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    @staticmethod
    def create_multitrack_mkv(dest_path: Path, srt_path: Path, duration_sec: int = 2):
        """Generates MKV with 1 video, 2 audio streams (eng, jpn), and 1 subtitle track."""
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"testsrc=duration={duration_sec}:size=320x240:rate=24",
            "-f", "lavfi", "-i", f"sine=frequency=440:duration={duration_sec}",
            "-f", "lavfi", "-i", f"sine=frequency=880:duration={duration_sec}",
            "-i", str(srt_path),
            "-map", "0:v", "-map", "1:a", "-map", "2:a", "-map", "3:s",
            "-metadata:s:a:0", "title=English Stereo", "-metadata:s:a:0", "language=eng",
            "-metadata:s:a:1", "title=Commentary Track", "-metadata:s:a:1", "language=jpn",
            "-metadata:s:s:0", "title=English CC", "-metadata:s:s:0", "language=eng",
            "-c:v", "libx264", "-c:a", "aac", "-c:s", "srt",
            str(dest_path)
        ]
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    @staticmethod
    def create_flac_audio(dest_path: Path, freq_hz: int = 1000, duration_sec: int = 2):
        """Generates lossless FLAC test tone."""
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"sine=frequency={freq_hz}:duration={duration_sec}",
            "-c:a", "flac",
            str(dest_path)
        ]
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    @staticmethod
    def create_wav_pure_python(dest_path: Path, freq_hz: float = 440.0, duration_sec: float = 1.0, sample_rate: int = 44100):
        """Generates PCM WAV without ffmpeg dependency using Python stdlib wave module."""
        n_samples = int(duration_sec * sample_rate)
        with wave.open(str(dest_path), 'wb') as wav:
            wav.setnchannels(1)
            wav.setsampwidth(2)
            wav.setframerate(sample_rate)
            frames = bytearray()
            for i in range(n_samples):
                sample_val = int(32767.0 * 0.5 * math.sin(2.0 * math.pi * freq_hz * i / sample_rate))
                frames.extend(struct.pack('<h', sample_val))
            wav.writeframes(frames)

    @staticmethod
    def create_lrc_file(dest_path: Path):
        """Generates synchronized LRC lyrics with millisecond timestamps."""
        content = (
            "[ti:Brutalist Echoes]\n"
            "[ar:Apex Synth]\n"
            "[al:Penguin Studio]\n"
            "[00:00.10]TACTILE DIGITAL BRUTALISM\n"
            "[00:00.60]OBSIDIAN CHASSIS ACTIVE\n"
            "[00:01.20]STUDIO PRECISION MONITORING\n"
            "[00:01.80]SIGNAL LIME GAIN PEAK 0.0dB\n"
        )
        dest_path.write_text(content, encoding='utf-8')
```

---

### 5.3 Equalizer Filter Response Mathematical Verification

The 10-band graphic equalizer operates on second-order IIR Biquad peaking filters (Robert Bristow-Johnson Audio EQ Cookbook).

#### Mathematical Equations:
Given:
- $F_s$: Sampling rate (e.g., $44100\text{ Hz}$)
- $f_0$: Center band frequency (e.g., $1000\text{ Hz}$)
- $dB\text{gain}$: Target band boost/cut (e.g., $+6.0\text{ dB}$)
- $Q$: Quality factor ($Q = 1.4142$ for 1-octave bandwidth)

$$A = 10^{\frac{dB\text{gain}}{40}}$$
$$\omega_0 = \frac{2 \pi f_0}{F_s}, \quad \alpha = \frac{\sin(\omega_0)}{2 Q}, \quad c = \cos(\omega_0)$$

$$\begin{aligned}
b_0 &= 1 + \alpha A, & b_1 &= -2 c, & b_2 &= 1 - \alpha A \\
a_0 &= 1 + \frac{\alpha}{A}, & a_1 &= -2 c, & a_2 &= 1 - \frac{\alpha}{A}
\end{aligned}$$

Normalized coefficients:
$$B_0 = \frac{b_0}{a_0}, \quad B_1 = \frac{b_1}{a_0}, \quad B_2 = \frac{b_2}{a_0}, \quad A_1 = \frac{a_1}{a_0}, \quad A_2 = \frac{a_2}{a_0}$$

Transfer function evaluated at frequency $f$ ($z = e^{j 2 \pi f / F_s}$):
$$H(z) = \frac{B_0 + B_1 z^{-1} + B_2 z^{-2}}{1 + A_1 z^{-1} + A_2 z^{-2}}$$
$$|H(z)|_{dB} = 20 \log_{10} |H(z)|$$

#### Automated Verification Assertion:
For any center frequency $f_0 \in \{32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000\}$, the evaluated response $|H(e^{j 2\pi f_0 / F_s})|_{dB}$ must match $dB\text{gain}$ within a numerical tolerance of $\pm 0.001\text{ dB}$.

---

### 5.4 Headless Execution Environment & D-Bus Harness

#### Running Qt Tests Headlessly:
No X11 display server is required. Tests configure the offscreen QPA platform:
```bash
export QT_QPA_PLATFORM=offscreen
export XDG_RUNTIME_DIR=/tmp/penguin_runtime_test
mkdir -p $XDG_RUNTIME_DIR
```

#### Isolated Ephemeral D-Bus Session:
To test the real D-Bus bus registration without contaminating the host user session:
```bash
dbus-run-session python3 -m unittest discover -s tests -p "test_*.py"
```
Or programmatically in Python fixtures via `subprocess.Popen(["dbus-daemon", "--session", "--print-address"])`.

---

### 5.5 Headless CLI Self-Test Runner (`penguin --headless-test`)

When launched with `--headless-test`, Penguin executes an integrated suite of diagnostics and outputs a high-contrast Brutalist ASCII summary table, exiting with code `0` on success or `1` on failure:

```
================================================================================
                    PENGUIN HEADLESS SELF-VERIFICATION REPORT                   
================================================================================
 [1/6] Database Subsystem:
   - SQLite WAL initialization .......................................... [PASS]
   - Schema migration & table integrity ................................. [PASS]
   - Equalizer presets load (8 presets verified) ........................ [PASS]
 [2/6] DSP & Equalizer Filter Engine:
   - 10-Band Biquad coefficient computation ............................. [PASS]
   - Center frequency response tolerance (+-0.001dB) .................... [PASS]
 [3/6] Metadata & Lyric Parsers:
   - Synchronized LRC parser timestamp accuracy ......................... [PASS]
   - SMPTE Timecode formatter (HH:MM:SS:FF @ 24/30fps) .................. [PASS]
 [4/6] Media Demuxer & Probe Engine:
   - MP4 / H.264 video probe ............................................ [PASS]
   - Multitrack MKV stream discovery (2 audio, 1 sub) ................... [PASS]
   - FLAC / MP3 / WAV audio probe ....................................... [PASS]
 [5/6] MPRIS2 D-Bus Interface Contract:
   - Identity & DesktopEntry property match ............................. [PASS]
   - PlaybackStatus & LoopStatus state machine .......................... [PASS]
   - Volume clamping & conversion ....................................... [PASS]
   - PropertiesChanged signal payload structure ......................... [PASS]
 [6/6] CLI Argument Parser:
   - Flag validation (--audio, --video, --fullscreen, --eq) ............. [PASS]
================================================================================
 ALL 6 SUBSYSTEM VERIFICATIONS PASSED (0 ERRORS, 0 WARNINGS) - EXIT 0
================================================================================
```

---

## 6. Implementation Action Plan for Downstream Agents

1. **Persistence Module:** Create `penguin/persistence/database.py` implementing the SQLite WAL schema, settings store, history upserts, and preset seeding.
2. **MPRIS Module:** Create `penguin/integration/mpris.py` with `BaseMPRISAdapter`, `DBusMPRISAdapter`, and `NullMPRISAdapter`.
3. **CLI Module:** Create `penguin/cli/args.py` with `argparse` configuration and `--headless-test` dispatch.
4. **Packaging Assets:** Provide `penguin.desktop` and scalable vector icon `penguin.svg`.
5. **Automated Test Suite:** Provide `tests/test_database.py`, `tests/test_dsp_equalizer.py`, `tests/test_lrc_parser.py`, `tests/test_mpris.py`, `tests/test_media_probe.py`, and `tests/fixtures/media_factory.py`.
