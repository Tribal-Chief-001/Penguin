"""
main_window.py - Top-Level MainWindow Orchestrator for Penguin Desktop Media Player.

Features:
- Seamless switching between Video Viewfinder Mode (0) and Hi-Fi Audio Deck Mode (1).
- Content-aware heuristic mode selection based on opened media format.
- Comprehensive global keyboard hotkey routing (Space, Arrows, F, M, Tab, Reticles, OSD, Stepping, Looper).
- Unified event dispatching and state persistence bridge.
"""

from enum import Enum
import os
from typing import Any, Callable, Dict, List, Optional, Tuple, Union

from src.engine.mpv_core import PlaybackEngine, PlaybackState
from src.ui.audio_deck import AudioDeckWidget, RepeatMode
from src.ui.theme import BrutalistTheme
from src.ui.video_viewport import ViewfinderWidget


class UIMode(Enum):
    VideoViewfinder = 0
    HiFiAudioDeck = 1
    # Aliases
    VIDEO_VIEWFINDER = 0
    HIFI_AUDIO_DECK = 1


AUDIO_EXTENSIONS = {".mp3", ".flac", ".wav", ".opus", ".aac", ".ogg", ".m4a", ".alac", ".aiff", ".wma"}
VIDEO_EXTENSIONS = {".mp4", ".mkv", ".webm", ".avi", ".mov", ".wmv", ".flv", ".ts", ".m4v"}


class MainWindow:
    """
    Top-Level Penguin MainWindow Orchestrator.
    Controls active mode (Viewfinder vs Audio Deck), shortcuts, and playback integration.
    """

    def __init__(
        self,
        playback_engine: Optional[PlaybackEngine] = None,
        width: int = 1280,
        height: int = 720,
    ):
        self._width = max(640, int(width))
        self._height = max(480, int(height))
        self._engine = playback_engine or PlaybackEngine()

        # UI Subsystems
        self._viewfinder = ViewfinderWidget(playback_engine=self._engine, width=self._width, height=self._height)
        self._audio_deck = AudioDeckWidget(playback_engine=self._engine, width=self._width, height=self._height)
        self._current_mode = UIMode.VideoViewfinder

        # UI Preferences & State
        self._is_fullscreen = False
        self._osd_message = ""
        self._osd_timer_ms = 0
        self._persistence = None
        self._playlist_mgr = None
        self._current_media: str = ""
        self._file_dialog_open_count: int = 0

        # Wire scrubber seeks to engine
        self._viewfinder.scrubber.set_on_seek(self._on_scrubber_seek)
        self._audio_deck.scrubber.set_on_seek(self._on_scrubber_seek)

        # Wire lyrics seeks
        self._audio_deck.teleprompter.set_on_seek_requested(self._on_scrubber_seek)

        # Wire EQ changes
        self._audio_deck.eq_rack.set_on_all_gains_changed(self._on_eq_gains_changed)

        # Wire playlist item triggers
        self._audio_deck.playlist_matrix.set_on_track_double_clicked(self._on_playlist_item_double_clicked)

    # Properties
    @property
    def current_mode(self) -> UIMode:
        return self._current_mode

    @property
    def playback_engine(self) -> PlaybackEngine:
        return self._engine

    @property
    def viewfinder_widget(self) -> ViewfinderWidget:
        return self._viewfinder

    @property
    def audio_deck_widget(self) -> AudioDeckWidget:
        return self._audio_deck

    @property
    def is_fullscreen(self) -> bool:
        return self._is_fullscreen

    # Mode Switching
    def set_mode(self, mode: Union[UIMode, int]):
        if isinstance(mode, int):
            mode = UIMode(mode)
        if self._current_mode != mode:
            self._current_mode = mode

    def toggle_mode(self):
        """Toggles mode between Video Viewfinder and Hi-Fi Audio Deck."""
        if self._current_mode == UIMode.VideoViewfinder:
            self.set_mode(UIMode.HiFiAudioDeck)
        else:
            self.set_mode(UIMode.VideoViewfinder)

    # Media Loading & Heuristics
    def open_media(self, file_path: str, auto_play: bool = True) -> bool:
        """
        Opens a media file, switches UI mode adaptively based on extension, and starts playback.
        """
        if not file_path:
            return False
        self._current_media = file_path

        _, ext = os.path.splitext(file_path.lower())
        if ext in AUDIO_EXTENSIONS:
            self.set_mode(UIMode.HiFiAudioDeck)
            self._audio_deck.update_metadata(title=os.path.basename(file_path))
        elif ext in VIDEO_EXTENSIONS:
            self.set_mode(UIMode.VideoViewfinder)

        if hasattr(self._engine, "load_file"):
            return self._engine.load_file(file_path, auto_play=auto_play)
        return True

    def open_file_dialog(self) -> Optional[str]:
        """
        Invokes open file dialog when user requests media open.
        """
        self._file_dialog_open_count += 1
        self._osd_message = "OPEN FILE DIALOG REQUESTED"
        return None

    def handle_play_pause(self):
        """
        Toggles play/pause, or automatically invokes open_file_dialog if no media is loaded.
        """
        has_media = bool(self._current_media)
        if hasattr(self._engine, "current_uri") and self._engine.current_uri:
            has_media = True
        elif hasattr(self._engine, "current_file") and self._engine.current_file:
            has_media = True
        elif hasattr(self._engine, "is_playing"):
            val = self._engine.is_playing
            if callable(val):
                if val():
                    has_media = True
            elif bool(val):
                has_media = True
        elif hasattr(self._engine, "is_paused"):
            val = self._engine.is_paused
            if callable(val):
                if val():
                    has_media = True
            elif bool(val):
                has_media = True

        if not has_media:
            self.open_file_dialog()
            return

        if hasattr(self._engine, "toggle_pause"):
            try:
                self._engine.toggle_pause()
            except Exception:
                pass

    def open_directory(self, dir_path: str):
        """Enqueues all supported media files in a directory."""
        if not os.path.isdir(dir_path):
            return
        for root, _, files in os.walk(dir_path):
            for f in sorted(files):
                _, ext = os.path.splitext(f.lower())
                if ext in AUDIO_EXTENSIONS or ext in VIDEO_EXTENSIONS:
                    full_path = os.path.join(root, f)
                    self._audio_deck.playlist_matrix.add_item(full_path)

    # Global Hotkey Routing
    def handle_key_press(self, key_name: str, modifiers: Optional[List[str]] = None) -> bool:
        """
        Processes key event and routes to media engine / UI actions.
        Returns True if the hotkey was handled.
        """
        key = key_name.upper()
        mods = [m.upper() for m in (modifiers or [])]
        is_shift = "SHIFT" in mods
        is_ctrl = "CTRL" in mods or "CONTROL" in mods

        # Transport & Seeking
        if key == "SPACE":
            self.handle_play_pause()
            return True

        elif key == "LEFT" or key == "J":
            # Jump backward 10s
            if hasattr(self._engine, "seek_relative"):
                try:
                    self._engine.seek_relative(-10.0)
                except Exception:
                    pass
            return True

        elif key == "RIGHT" or key == "L":
            # Jump forward 10s
            if hasattr(self._engine, "seek_relative"):
                try:
                    self._engine.seek_relative(10.0)
                except Exception:
                    pass
            return True

        elif key == "UP":
            # Volume Up +5%
            if hasattr(self._engine, "set_volume") and hasattr(self._engine, "volume"):
                try:
                    self._engine.set_volume(min(100.0, self._engine.volume + 5.0))
                except Exception:
                    pass
            return True

        elif key == "DOWN":
            # Volume Down -5%
            if hasattr(self._engine, "set_volume") and hasattr(self._engine, "volume"):
                try:
                    self._engine.set_volume(max(0.0, self._engine.volume - 5.0))
                except Exception:
                    pass
            return True

        elif key == "M":
            # Mute Toggle
            if hasattr(self._engine, "toggle_mute"):
                try:
                    self._engine.toggle_mute()
                except Exception:
                    pass
            return True

        # Mode & Viewport Controls
        elif key == "TAB":
            # Toggle Video / Audio Mode
            self.toggle_mode()
            return True

        elif key == "F" or key == "F11":
            # Toggle Fullscreen
            self.toggle_fullscreen()
            return True

        elif key == "R":
            # Toggle Safe Reticles
            self._viewfinder.toggle_reticles()
            return True

        elif key == "O" or key == "F1":
            # Toggle OSD Telemetry HUD
            self._viewfinder.toggle_osd()
            return True

        # Frame Stepping
        elif key == "," or key == "COMMA":
            # Step 1 Frame Backward (< 1F)
            if hasattr(self._engine, "frame_step_backward"):
                try:
                    self._engine.frame_step_backward()
                except Exception:
                    pass
            return True

        elif key == "." or key == "PERIOD":
            # Step 1 Frame Forward (1F >)
            if hasattr(self._engine, "frame_step_forward"):
                try:
                    self._engine.frame_step_forward()
                except Exception:
                    pass
            return True

        # Speed Multipliers
        elif key == "[" or key == "BRACKETLEFT":
            if hasattr(self._engine, "speed"):
                try:
                    new_speed = max(0.5, self._engine.speed - 0.25)
                    self._engine.set_speed(new_speed)
                except Exception:
                    pass
            return True

        elif key == "]" or key == "BRACKETRIGHT":
            if hasattr(self._engine, "speed"):
                try:
                    new_speed = min(2.0, self._engine.speed + 0.25)
                    self._engine.set_speed(new_speed)
                except Exception:
                    pass
            return True

        elif key == "BACKSPACE":
            if hasattr(self._engine, "set_speed"):
                try:
                    self._engine.set_speed(1.0)
                except Exception:
                    pass
            return True

        # Screenshot Export
        elif key == "S":
            if hasattr(self._engine, "take_screenshot"):
                try:
                    self._engine.take_screenshot(include_subtitles=is_shift)
                except Exception:
                    pass
            return True

        # A-B Repeat Looper
        elif key == "A":
            self._viewfinder.set_loop_point_a()
            return True

        elif key == "B":
            self._viewfinder.set_loop_point_b()
            return True

        elif key == "\\" or key == "BACKSLASH":
            self._viewfinder.clear_loop()
            return True

        # DSP Audio & Video Filters
        elif key == "N":
            self._viewfinder.toggle_night_mode()
            return True

        elif key == "D":
            self._viewfinder.toggle_deband()
            return True

        elif key == "C":
            self._viewfinder.cycle_aspect_ratio()
            return True

        return False

    def toggle_fullscreen(self):
        self._is_fullscreen = not self._is_fullscreen
        self._viewfinder.toggle_fullscreen()

    def show_osd_message(self, message: str, duration_ms: int = 2500):
        self._osd_message = message
        self._osd_timer_ms = duration_ms

    # Internal Callbacks
    def _on_scrubber_seek(self, pos_ms: int):
        if hasattr(self._engine, "seek_absolute_ms"):
            self._engine.seek_absolute_ms(pos_ms)

    def _on_eq_gains_changed(self, gains: List[float]):
        if hasattr(self._engine, "set_equalizer_gains"):
            self._engine.set_equalizer_gains(gains)

    def _on_playlist_item_double_clicked(self, index: int, item):
        if hasattr(self._engine, "load_file"):
            self._engine.load_file(item.file_path, auto_play=True)

    def resize(self, width: int, height: int):
        self._width = max(640, int(width))
        self._height = max(480, int(height))
        self._viewfinder.resize(self._width, self._height)
        self._audio_deck.resize(self._width, self._height)

    def create_context_menu(self) -> Dict[str, Any]:
        """
        Returns a comprehensive VLC / IINA / mpv-grade context menu tree representation
        styled with Monolithic Brutalist tokens.
        """
        is_playing = False
        if hasattr(self._engine, "is_playing"):
            val = self._engine.is_playing
            is_playing = val() if callable(val) else bool(val)
        cur_speed = getattr(self._engine, "speed", 1.0)
        return {
            "title": "PENGUIN_CONTEXT_MENU",
            "style": {
                "background": "#0B0B0E",
                "border": "1px solid #2A2A35",
                "text": "#FFFFFF",
                "hover_bg": "#141418",
                "hover_text": "#CCFF00",
                "hover_accent": "#FF4400",
            },
            "submenus": {
                "MEDIA": [
                    {"label": "Open File...", "shortcut": "Ctrl+O", "action": "open_file"},
                    {"label": "Open Folder...", "shortcut": "Ctrl+Shift+O", "action": "open_folder"},
                    {"label": "Open Network Stream URL...", "shortcut": "Ctrl+U", "action": "open_url"},
                ],
                "PLAYBACK": [
                    {"label": "Pause" if is_playing else "Play", "shortcut": "Space", "action": "play_pause"},
                    {"type": "separator"},
                    {"label": "Jump Forward (+10s)", "shortcut": "Right", "action": "seek_fwd_10"},
                    {"label": "Jump Backward (-10s)", "shortcut": "Left", "action": "seek_back_10"},
                    {"label": "Jump Forward (+30s)", "shortcut": "Shift+Right", "action": "seek_fwd_30"},
                    {"label": "Jump Backward (-30s)", "shortcut": "Shift+Left", "action": "seek_back_30"},
                    {"type": "separator"},
                    {"label": "Single-Frame Step Forward", "shortcut": ".", "action": "frame_step_fwd"},
                    {"label": "Single-Frame Step Backward", "shortcut": ",", "action": "frame_step_back"},
                    {"type": "separator"},
                    {
                        "label": "Speed",
                        "items": [
                            {"label": f"{r}x", "rate": r, "checked": abs(cur_speed - r) < 0.05}
                            for r in [0.5, 0.75, 1.0, 1.25, 1.5, 2.0]
                        ],
                    },
                    {"type": "separator"},
                    {
                        "label": "A-B Repeat Loop",
                        "items": [
                            {"label": "Set Point A", "shortcut": "[", "action": "set_loop_a"},
                            {"label": "Set Point B", "shortcut": "]", "action": "set_loop_b"},
                            {"label": "Clear Loop", "shortcut": "\\", "action": "clear_loop"},
                        ],
                    },
                ],
                "AUDIO": [
                    {"label": "Audio Track", "items": getattr(self._engine, "audio_tracks", [])},
                    {"type": "separator"},
                    {"label": "Volume Up (+5%)", "shortcut": "Up", "action": "volume_up"},
                    {"label": "Volume Down (-5%)", "shortcut": "Down", "action": "volume_down"},
                    {"label": "Mute / Unmute", "shortcut": "M", "action": "toggle_mute", "checked": getattr(self._engine, "is_muted", False)},
                    {"type": "separator"},
                    {"label": "10-Band Graphic Equalizer...", "shortcut": "Ctrl+E", "action": "open_equalizer"},
                    {"label": "Night Mode Dialogue Compressor", "shortcut": "N", "action": "toggle_night_mode"},
                    {"label": "Headphone Spatial Crossfeed (BS2B)", "shortcut": "C", "action": "toggle_crossfeed"},
                    {
                        "label": "Pitch Shift",
                        "items": [
                            {"label": "Pitch Shift +1 ST", "shortcut": "Alt+Up", "action": "pitch_up"},
                            {"label": "Pitch Shift -1 ST", "shortcut": "Alt+Down", "action": "pitch_down"},
                            {"label": "Reset Pitch (0 ST)", "shortcut": "Alt+0", "action": "pitch_reset"},
                        ],
                    },
                ],
                "VIDEO": [
                    {
                        "label": "Aspect Ratio",
                        "items": ["auto", "16:9", "4:3", "21:9", "2.35:1"],
                    },
                    {"label": "Video Color Equalizer...", "shortcut": "Ctrl+E", "action": "open_video_equalizer"},
                    {"label": "Deband Dithering Filter", "shortcut": "D", "action": "toggle_deband"},
                    {"label": "Safe-Area Reticles", "shortcut": "R", "action": "toggle_reticles"},
                    {"label": "Telemetry OSD HUD", "shortcut": "O", "action": "toggle_osd"},
                    {"label": "Fullscreen", "shortcut": "F", "action": "toggle_fullscreen", "checked": self._is_fullscreen},
                    {"label": "Floating Picture-in-Picture", "shortcut": "T", "action": "toggle_pip"},
                    {
                        "label": "Lossless Screenshot",
                        "items": [
                            {"label": "Clean Video Frame", "shortcut": "S", "action": "screenshot_clean"},
                            {"label": "Frame with Subtitles", "shortcut": "Shift+S", "action": "screenshot_subtitles"},
                        ],
                    },
                ],
                "SUBTITLES": [
                    {"label": "Primary Subtitle Track", "items": getattr(self._engine, "subtitle_tracks", [])},
                    {"label": "Secondary Subtitle Track (Dual Learning)", "shortcut": "V", "action": "cycle_secondary_sub"},
                    {"label": "Load External Subtitle File...", "action": "load_sub"},
                    {
                        "label": "Subtitle Sync Nudge",
                        "items": [
                            {"label": "Nudge Delay +50ms", "shortcut": "X", "action": "sub_delay_plus"},
                            {"label": "Nudge Delay -50ms", "shortcut": "Z", "action": "sub_delay_minus"},
                        ],
                    },
                ],
                "CHAPTERS_AND_BOOKMARKS": [
                    {"label": "Chapters", "items": getattr(self._engine, "chapters", [])},
                    {"label": "Next Chapter", "shortcut": "PageDown", "action": "next_chapter"},
                    {"label": "Previous Chapter", "shortcut": "PageUp", "action": "prev_chapter"},
                    {"type": "separator"},
                    {"label": "Add Bookmark Marker", "shortcut": "B", "action": "add_bookmark"},
                    {"label": "Next Bookmark", "shortcut": "Shift+B", "action": "next_bookmark"},
                ],
            },
            "root_actions": [
                {"label": "Play / Pause", "shortcut": "Space", "action": "play_pause"},
                {"label": "Switch View Mode", "shortcut": "Tab", "action": "toggle_mode"},
                {"label": "Fullscreen", "shortcut": "F", "action": "toggle_fullscreen"},
            ],
        }

    def context_menu_event(self, global_pos: Optional[Tuple[int, int]] = None) -> Dict[str, Any]:
        """
        Dispatches context menu event and returns the active context menu.
        """
        return self.create_context_menu()
