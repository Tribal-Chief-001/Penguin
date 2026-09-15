"""
src/desktop/mpris2.py - MPRIS2 D-Bus Service Implementation for Penguin.

Implements the Media Player Remote Interfacing Specification (MPRIS) Version 2.2:
- Root interface: org.mpris.MediaPlayer2 (Raise, Quit, Fullscreen, Identity, etc.)
- Player interface: org.mpris.MediaPlayer2.Player (Play, Pause, PlayPause, Stop, Next, Previous,
  Seek, SetPosition, OpenUri, Volume, Rate, LoopStatus, Shuffle, Metadata, Position)
- Signals: Seeked(x Position), PropertiesChanged(s interface, a{sv} changed, as invalidated)
- XESAM 2.0 metadata dictionary formatting (mpris:trackid, mpris:length, xesam:title, xesam:artist, etc.)
- Bi-directional synchronization bridge with Penguin PlaybackEngine.
- Dual-mode architecture: Real D-Bus SessionBus export with graceful headless/in-memory fallback.
"""

from __future__ import annotations
from dataclasses import dataclass, field
import logging
import os
import sys
import threading
import time
from typing import Any, Callable, Dict, List, Optional, Tuple, Union

logger = logging.getLogger("Penguin.Desktop.MPRIS2")

# Optional dbus-python import
try:
    import dbus
    import dbus.service
    import dbus.mainloop.glib
    DBUS_AVAILABLE = True
except ImportError:
    DBUS_AVAILABLE = False
    dbus = None


# =============================================================================
# 1. TRACK METADATA (XESAM 2.0 SCHEMA)
# =============================================================================

@dataclass
class TrackMetadata:
    """Represents media track metadata formatted according to the MPRIS2 / XESAM standard."""
    track_id: str = "/org/mpris/MediaPlayer2/Track/0"
    length_us: int = 0          # duration in microseconds (1s = 1,000,000 us)
    art_url: str = ""           # URI to album artwork (file:// or http(s)://)
    title: str = ""             # Track / video title
    artists: List[str] = field(default_factory=list)      # Contributing artist(s)
    album: str = ""             # Album / collection name
    album_artists: List[str] = field(default_factory=list) # Album artist(s)
    genres: List[str] = field(default_factory=list)       # Genre tags
    track_number: int = 1       # Track number on album
    disc_number: int = 1        # Disc number
    url: str = ""               # Original file URL or stream URI
    user_rating: float = 0.0    # Rating from 0.0 to 1.0

    def to_mpris_dict(self) -> Dict[str, Any]:
        """Converts to standard MPRIS a{sv} metadata dictionary."""
        d: Dict[str, Any] = {
            "mpris:trackid": str(self.track_id or "/org/mpris/MediaPlayer2/Track/0"),
            "mpris:length": int(self.length_us or 0),
            "xesam:title": str(self.title or ""),
            "xesam:artist": list(self.artists) if self.artists else [],
            "xesam:album": str(self.album or ""),
            "xesam:albumArtist": list(self.album_artists) if self.album_artists else [],
            "xesam:genre": list(self.genres) if self.genres else [],
            "xesam:trackNumber": int(self.track_number or 1),
            "xesam:discNumber": int(self.disc_number or 1),
            "xesam:url": str(self.url or ""),
        }
        if self.art_url:
            d["mpris:artUrl"] = str(self.art_url)
        if self.user_rating > 0.0:
            d["xesam:userRating"] = float(self.user_rating)
        return d

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> TrackMetadata:
        """Constructs TrackMetadata from dictionary."""
        artists = data.get("xesam:artist", [])
        if isinstance(artists, str):
            artists = [artists]
        album_artists = data.get("xesam:albumArtist", [])
        if isinstance(album_artists, str):
            album_artists = [album_artists]
        genres = data.get("xesam:genre", [])
        if isinstance(genres, str):
            genres = [genres]

        return cls(
            track_id=str(data.get("mpris:trackid", "/org/mpris/MediaPlayer2/Track/0")),
            length_us=int(data.get("mpris:length", 0)),
            art_url=str(data.get("mpris:artUrl", "")),
            title=str(data.get("xesam:title", "")),
            artists=list(artists),
            album=str(data.get("xesam:album", "")),
            album_artists=list(album_artists),
            genres=list(genres),
            track_number=int(data.get("xesam:trackNumber", 1)),
            disc_number=int(data.get("xesam:discNumber", 1)),
            url=str(data.get("xesam:url", "")),
            user_rating=float(data.get("xesam:userRating", 0.0)),
        )


# =============================================================================
# 2. MPRIS2 SERVICE CORE (IN-MEMORY & PROTOCOL CONTROLLER)
# =============================================================================

class MPRIS2Service:
    """Standard-compliant MPRIS2 service implementation for Penguin Media Player."""

    BUS_NAME = "org.mpris.MediaPlayer2.penguin"
    OBJECT_PATH = "/org/mpris/MediaPlayer2"
    ROOT_INTERFACE = "org.mpris.MediaPlayer2"
    PLAYER_INTERFACE = "org.mpris.MediaPlayer2.Player"
    PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties"

    def __init__(self, engine: Any = None, playlist_manager: Any = None, main_window: Any = None):
        self.engine = engine
        self.playlist_manager = playlist_manager
        self.main_window = main_window

        # Root interface properties
        self.can_quit: bool = True
        self.can_raise: bool = True
        self.can_set_fullscreen: bool = True
        self.fullscreen: bool = False
        self.has_track_list: bool = False
        self.identity: str = "Penguin Media Player"
        self.desktop_entry: str = "penguin"
        self.supported_uri_schemes: List[str] = ["file", "http", "https", "ftp"]
        self.supported_mime_types: List[str] = [
            "audio/mpeg", "audio/flac", "audio/ogg", "audio/opus", "audio/x-wav",
            "audio/aac", "video/mp4", "video/x-matroska", "video/webm", "video/quicktime",
            "video/x-msvideo"
        ]

        # Player interface properties
        self.playback_status: str = "Stopped"  # "Playing", "Paused", "Stopped"
        self.loop_status: str = "None"         # "None", "Track", "Playlist"
        self.rate: float = 1.0
        self.shuffle: bool = False
        self.volume: float = 1.0               # 0.0 to 1.5
        self.position_us: int = 0             # current position in microseconds
        self.minimum_rate: float = 0.5
        self.maximum_rate: float = 2.0
        self.can_control: bool = True
        self.can_seek: bool = True
        self.can_play: bool = True
        self.can_pause: bool = True
        self.can_go_next: bool = False
        self.can_go_previous: bool = False

        self.current_metadata: TrackMetadata = TrackMetadata()
        self.signal_history: List[Tuple[str, Any]] = []

        # Action callbacks
        self._on_play: Optional[Callable[[], None]] = None
        self._on_pause: Optional[Callable[[], None]] = None
        self._on_stop: Optional[Callable[[], None]] = None
        self._on_next: Optional[Callable[[], None]] = None
        self._on_prev: Optional[Callable[[], None]] = None
        self._on_seek: Optional[Callable[[int], None]] = None
        self._on_set_position: Optional[Callable[[str, int], None]] = None
        self._on_open_uri: Optional[Callable[[str], None]] = None
        self._on_volume_change: Optional[Callable[[float], None]] = None
        self._on_rate_change: Optional[Callable[[float], None]] = None
        self._on_raise: Optional[Callable[[], None]] = None
        self._on_quit: Optional[Callable[[], None]] = None

        # Real DBus export reference
        self._dbus_export: Optional[Any] = None
        self._bus_name: Optional[Any] = None
        self._is_registered_on_bus: bool = False

        # If engine was provided at construction, bind to it
        if self.engine is not None:
            self.bind_playback_engine(self.engine)

    # -------------------------------------------------------------------------
    # D-Bus Registration & Lifecycle
    # -------------------------------------------------------------------------

    def register_dbus_service(self) -> bool:
        """Attempts to register the service on the system D-Bus SessionBus."""
        if not DBUS_AVAILABLE:
            logger.info("dbus-python not installed; running MPRIS2 in standalone/mock mode.")
            return False

        try:
            # Set up GLib mainloop for dbus if available
            try:
                dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
            except Exception:
                pass

            session_bus = dbus.SessionBus()
            # Try primary name
            try:
                self._bus_name = dbus.service.BusName(self.BUS_NAME, session_bus, do_not_queue=True)
            except dbus.exceptions.NameExistsException:
                # Fallback to instance PID name
                instance_bus_name = f"{self.BUS_NAME}.instance{os.getpid()}"
                self._bus_name = dbus.service.BusName(instance_bus_name, session_bus, do_not_queue=True)

            self._dbus_export = _DBusExportObject(session_bus, self.OBJECT_PATH, self)
            self._is_registered_on_bus = True
            logger.info(f"Registered MPRIS2 D-Bus service at {self.BUS_NAME}")
            return True
        except Exception as e:
            logger.warning(f"Could not register MPRIS2 D-Bus service ({e}); operating in memory.")
            self._is_registered_on_bus = False
            return False

    def unregister_dbus_service(self):
        """Unregisters service from D-Bus."""
        if self._dbus_export is not None:
            try:
                self._dbus_export.remove_from_connection()
            except Exception:
                pass
            self._dbus_export = None
        self._bus_name = None
        self._is_registered_on_bus = False

    def is_registered_on_bus(self) -> bool:
        return self._is_registered_on_bus

    # -------------------------------------------------------------------------
    # Engine Binding
    # -------------------------------------------------------------------------

    def bind_playback_engine(self, engine: Any):
        """Binds PlaybackEngine events to MPRIS2 state and notifications."""
        self.engine = engine
        if hasattr(engine, "add_state_callback"):
            engine.add_state_callback(self._on_engine_state_changed)
        if hasattr(engine, "add_time_callback"):
            engine.add_time_callback(self._on_engine_time_changed)
        if hasattr(engine, "add_volume_callback"):
            engine.add_volume_callback(self._on_engine_volume_changed)

    def _on_engine_state_changed(self, state: Any):
        state_str = str(state).lower()
        if "play" in state_str and "pause" not in state_str:
            self.playback_status = "Playing"
        elif "pause" in state_str:
            self.playback_status = "Paused"
        else:
            self.playback_status = "Stopped"
        self.emit_properties_changed({"PlaybackStatus": self.playback_status})

    def _on_engine_time_changed(self, time_ms: float):
        self.position_us = int(max(0.0, time_ms) * 1000)

    def _on_engine_volume_changed(self, vol_percent: int):
        self.volume = max(0.0, min(1.5, vol_percent / 100.0))
        self.emit_properties_changed({"Volume": self.volume})

    # -------------------------------------------------------------------------
    # Signal Broadcasting
    # -------------------------------------------------------------------------

    def emit_signal(self, signal_name: str, payload: Any):
        """Records signal emission and dispatches to D-Bus adaptor if active."""
        self.signal_history.append((signal_name, payload))
        if self._dbus_export is not None:
            if signal_name == "Seeked":
                self._dbus_export.Seeked(int(payload))
            elif signal_name == "PropertiesChanged":
                if isinstance(payload, dict):
                    iface = payload.get("interface_name", self.PLAYER_INTERFACE)
                    changed = payload.get("changed_properties", {})
                    invalidated = payload.get("invalidated_properties", [])
                    self._dbus_export.PropertiesChanged(iface, changed, invalidated)

    def emit_properties_changed(
        self, changed_props: Dict[str, Any], invalidated_props: Optional[List[str]] = None
    ):
        """Emits standard org.freedesktop.DBus.Properties.PropertiesChanged notification."""
        payload = {
            "interface_name": self.PLAYER_INTERFACE,
            "changed_properties": changed_props,
            "invalidated_properties": invalidated_props or [],
        }
        self.emit_signal("PropertiesChanged", payload)

    # -------------------------------------------------------------------------
    # Root Interface: org.mpris.MediaPlayer2
    # -------------------------------------------------------------------------

    def Raise(self):
        """Brings the media player application window to front and gives it focus."""
        self.emit_signal("Raise", None)
        if self._on_raise:
            self._on_raise()
        elif self.main_window and hasattr(self.main_window, "raise_window"):
            self.main_window.raise_window()

    def Quit(self):
        """Cleanly terminates and shuts down the media player application."""
        self.emit_signal("Quit", None)
        if self._on_quit:
            self._on_quit()
        elif self.main_window and hasattr(self.main_window, "close"):
            self.main_window.close()

    def set_fullscreen(self, value: bool):
        """Toggles fullscreen display state."""
        self.fullscreen = bool(value)
        if self.main_window and hasattr(self.main_window, "set_fullscreen"):
            self.main_window.set_fullscreen(self.fullscreen)
        self.emit_properties_changed({"Fullscreen": self.fullscreen})

    # -------------------------------------------------------------------------
    # Player Interface: org.mpris.MediaPlayer2.Player
    # -------------------------------------------------------------------------

    def Play(self):
        """Starts or resumes media playback."""
        if self.playback_status != "Playing":
            self.playback_status = "Playing"
            if self._on_play:
                self._on_play()
            elif self.engine and hasattr(self.engine, "play"):
                self.engine.play()
            self.emit_properties_changed({"PlaybackStatus": self.playback_status})

    def Pause(self):
        """Pauses media playback."""
        if self.playback_status == "Playing":
            self.playback_status = "Paused"
            if self._on_pause:
                self._on_pause()
            elif self.engine and hasattr(self.engine, "pause"):
                self.engine.pause()
            self.emit_properties_changed({"PlaybackStatus": self.playback_status})

    def PlayPause(self):
        """Toggles playback between Playing and Paused."""
        if self.playback_status == "Playing":
            self.Pause()
        else:
            self.Play()

    def Stop(self):
        """Stops media playback and resets position to start."""
        self.playback_status = "Stopped"
        self.position_us = 0
        if self._on_stop:
            self._on_stop()
        elif self.engine and hasattr(self.engine, "stop"):
            self.engine.stop()
        self.emit_properties_changed({"PlaybackStatus": "Stopped", "Position": 0})

    def Next(self):
        """Skips to next track in queue."""
        self.emit_signal("Next", None)
        if self._on_next:
            self._on_next()
        elif self.playlist_manager and hasattr(self.playlist_manager, "next_track"):
            self.playlist_manager.next_track()

    def Previous(self):
        """Skips to previous track or restarts current track."""
        self.emit_signal("Previous", None)
        if self._on_prev:
            self._on_prev()
        elif self.playlist_manager and hasattr(self.playlist_manager, "previous_track"):
            self.playlist_manager.previous_track()

    def Seek(self, offset_us: int):
        """Seeks relative to current position by offset in microseconds."""
        new_pos = max(0, self.position_us + offset_us)
        if self.current_metadata.length_us > 0:
            new_pos = min(self.current_metadata.length_us, new_pos)
        self.position_us = new_pos

        if self._on_seek:
            self._on_seek(offset_us)
        elif self.engine and hasattr(self.engine, "seek_ms"):
            self.engine.seek_ms(new_pos / 1000.0)

        self.emit_signal("Seeked", new_pos)

    def SetPosition(self, track_id: str, position_us: int):
        """Sets absolute playback position in microseconds for the specified track."""
        # Check track_id validity according to MPRIS2 spec
        if str(track_id) == str(self.current_metadata.track_id) or str(track_id).endswith("/Track/0"):
            new_pos = max(0, position_us)
            if self.current_metadata.length_us > 0:
                new_pos = min(self.current_metadata.length_us, new_pos)
            self.position_us = new_pos

            if self._on_set_position:
                self._on_set_position(str(track_id), position_us)
            elif self.engine and hasattr(self.engine, "seek_ms"):
                self.engine.seek_ms(new_pos / 1000.0)

            self.emit_signal("Seeked", new_pos)

    def OpenUri(self, uri: str):
        """Loads and immediately starts playback of the specified media URI."""
        self.current_metadata = TrackMetadata(
            track_id="/org/mpris/MediaPlayer2/Track/1",
            length_us=180_000_000,  # default placeholder or updated on file load
            title=os.path.basename(uri) if uri else "Synthetic Test Track",
            artists=["Penguin Artist"],
            album="Brutalist Studio",
            url=uri,
        )
        self.position_us = 0
        self.playback_status = "Playing"

        if self._on_open_uri:
            self._on_open_uri(uri)
        elif self.engine and hasattr(self.engine, "load_file") and uri:
            self.engine.load_file(uri)

        self.emit_properties_changed({
            "Metadata": self.current_metadata.to_mpris_dict(),
            "PlaybackStatus": "Playing",
            "Position": 0,
        })

    def set_volume(self, val: float):
        """Sets volume level [0.0 to 1.5]. Clamps value to valid boundary."""
        clamped = max(0.0, min(1.5, float(val)))
        if clamped != self.volume:
            self.volume = clamped
            if self._on_volume_change:
                self._on_volume_change(clamped)
            elif self.engine and hasattr(self.engine, "set_volume"):
                self.engine.set_volume(int(round(clamped * 100)))
            self.emit_properties_changed({"Volume": self.volume})

    def set_rate(self, val: float):
        """Sets playback speed multiplier clamped between minimum and maximum rate."""
        clamped = max(self.minimum_rate, min(self.maximum_rate, float(val)))
        if clamped != self.rate:
            self.rate = clamped
            if self._on_rate_change:
                self._on_rate_change(clamped)
            elif self.engine and hasattr(self.engine, "set_speed"):
                self.engine.set_speed(clamped)
            self.emit_properties_changed({"Rate": self.rate})

    def set_loop_status(self, val: str):
        """Sets loop mode: 'None', 'Track', or 'Playlist'."""
        status = str(val).capitalize()
        if status in ["None", "Track", "Playlist"]:
            self.loop_status = status
            self.emit_properties_changed({"LoopStatus": self.loop_status})

    def set_shuffle(self, val: bool):
        """Sets random shuffle mode."""
        self.shuffle = bool(val)
        self.emit_properties_changed({"Shuffle": self.shuffle})

    def update_metadata(self, metadata: Union[TrackMetadata, Dict[str, Any]]):
        """Updates track metadata and broadcasts PropertiesChanged."""
        if isinstance(metadata, dict):
            self.current_metadata = TrackMetadata.from_dict(metadata)
        else:
            self.current_metadata = metadata
        self.emit_properties_changed({"Metadata": self.current_metadata.to_mpris_dict()})

    def update_position(self, pos_us: int, emit_seeked: bool = False):
        """Updates current position (in microseconds) with optional Seeked signal."""
        self.position_us = max(0, int(pos_us))
        if emit_seeked:
            self.emit_signal("Seeked", self.position_us)


# =============================================================================
# 3. DBUS-PYTHON SERVICE OBJECT ADAPTOR
# =============================================================================

if DBUS_AVAILABLE:
    class _DBusExportObject(dbus.service.Object):
        """Internal D-Bus service export object binding MPRIS2Service to the session bus."""

        def __init__(self, bus: dbus.SessionBus, object_path: str, service: MPRIS2Service):
            super().__init__(bus, object_path)
            self._svc = service

        # --- org.freedesktop.DBus.Properties ---
        @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature="ss", out_signature="v")
        def Get(self, interface_name: str, property_name: str):
            if interface_name == MPRIS2Service.ROOT_INTERFACE:
                props = {
                    "CanQuit": dbus.Boolean(self._svc.can_quit),
                    "Fullscreen": dbus.Boolean(self._svc.fullscreen),
                    "CanSetFullscreen": dbus.Boolean(self._svc.can_set_fullscreen),
                    "CanRaise": dbus.Boolean(self._svc.can_raise),
                    "HasTrackList": dbus.Boolean(self._svc.has_track_list),
                    "Identity": dbus.String(self._svc.identity),
                    "DesktopEntry": dbus.String(self._svc.desktop_entry),
                    "SupportedUriSchemes": dbus.Array(self._svc.supported_uri_schemes, signature="s"),
                    "SupportedMimeTypes": dbus.Array(self._svc.supported_mime_types, signature="s"),
                }
                if property_name in props:
                    return props[property_name]
            elif interface_name == MPRIS2Service.PLAYER_INTERFACE:
                props = {
                    "PlaybackStatus": dbus.String(self._svc.playback_status),
                    "LoopStatus": dbus.String(self._svc.loop_status),
                    "Rate": dbus.Double(self._svc.rate),
                    "Shuffle": dbus.Boolean(self._svc.shuffle),
                    "Metadata": dbus.Dictionary(self._svc.current_metadata.to_mpris_dict(), signature="sv"),
                    "Volume": dbus.Double(self._svc.volume),
                    "Position": dbus.Int64(self._svc.position_us),
                    "MinimumRate": dbus.Double(self._svc.minimum_rate),
                    "MaximumRate": dbus.Double(self._svc.maximum_rate),
                    "CanGoNext": dbus.Boolean(self._svc.can_go_next),
                    "CanGoPrevious": dbus.Boolean(self._svc.can_go_previous),
                    "CanPlay": dbus.Boolean(self._svc.can_play),
                    "CanPause": dbus.Boolean(self._svc.can_pause),
                    "CanSeek": dbus.Boolean(self._svc.can_seek),
                    "CanControl": dbus.Boolean(self._svc.can_control),
                }
                if property_name in props:
                    return props[property_name]
            raise dbus.exceptions.DBusException(f"Unknown property {property_name} on interface {interface_name}")

        @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature="s", out_signature="a{sv}")
        def GetAll(self, interface_name: str):
            res = {}
            if interface_name in (MPRIS2Service.ROOT_INTERFACE, ""):
                res.update({
                    "CanQuit": dbus.Boolean(self._svc.can_quit),
                    "Fullscreen": dbus.Boolean(self._svc.fullscreen),
                    "CanSetFullscreen": dbus.Boolean(self._svc.can_set_fullscreen),
                    "CanRaise": dbus.Boolean(self._svc.can_raise),
                    "HasTrackList": dbus.Boolean(self._svc.has_track_list),
                    "Identity": dbus.String(self._svc.identity),
                    "DesktopEntry": dbus.String(self._svc.desktop_entry),
                    "SupportedUriSchemes": dbus.Array(self._svc.supported_uri_schemes, signature="s"),
                    "SupportedMimeTypes": dbus.Array(self._svc.supported_mime_types, signature="s"),
                })
            if interface_name in (MPRIS2Service.PLAYER_INTERFACE, ""):
                res.update({
                    "PlaybackStatus": dbus.String(self._svc.playback_status),
                    "LoopStatus": dbus.String(self._svc.loop_status),
                    "Rate": dbus.Double(self._svc.rate),
                    "Shuffle": dbus.Boolean(self._svc.shuffle),
                    "Metadata": dbus.Dictionary(self._svc.current_metadata.to_mpris_dict(), signature="sv"),
                    "Volume": dbus.Double(self._svc.volume),
                    "Position": dbus.Int64(self._svc.position_us),
                    "MinimumRate": dbus.Double(self._svc.minimum_rate),
                    "MaximumRate": dbus.Double(self._svc.maximum_rate),
                    "CanGoNext": dbus.Boolean(self._svc.can_go_next),
                    "CanGoPrevious": dbus.Boolean(self._svc.can_go_previous),
                    "CanPlay": dbus.Boolean(self._svc.can_play),
                    "CanPause": dbus.Boolean(self._svc.can_pause),
                    "CanSeek": dbus.Boolean(self._svc.can_seek),
                    "CanControl": dbus.Boolean(self._svc.can_control),
                })
            return dbus.Dictionary(res, signature="sv")

        @dbus.service.method(dbus.PROPERTIES_IFACE, in_signature="ssv")
        def Set(self, interface_name: str, property_name: str, value: Any):
            if interface_name == MPRIS2Service.ROOT_INTERFACE:
                if property_name == "Fullscreen":
                    self._svc.set_fullscreen(bool(value))
                    return
            elif interface_name == MPRIS2Service.PLAYER_INTERFACE:
                if property_name == "Volume":
                    self._svc.set_volume(float(value))
                    return
                elif property_name == "Rate":
                    self._svc.set_rate(float(value))
                    return
                elif property_name == "LoopStatus":
                    self._svc.set_loop_status(str(value))
                    return
                elif property_name == "Shuffle":
                    self._svc.set_shuffle(bool(value))
                    return
            raise dbus.exceptions.DBusException(f"Property {property_name} is read-only or not supported.")

        @dbus.service.signal(dbus.PROPERTIES_IFACE, signature="sa{sv}as")
        def PropertiesChanged(self, interface_name: str, changed_properties: dict, invalidated_properties: list):
            pass

        # --- org.mpris.MediaPlayer2 (Root) ---
        @dbus.service.method(MPRIS2Service.ROOT_INTERFACE)
        def Raise(self):
            self._svc.Raise()

        @dbus.service.method(MPRIS2Service.ROOT_INTERFACE)
        def Quit(self):
            self._svc.Quit()

        # --- org.mpris.MediaPlayer2.Player ---
        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE)
        def Next(self):
            self._svc.Next()

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE)
        def Previous(self):
            self._svc.Previous()

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE)
        def Pause(self):
            self._svc.Pause()

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE)
        def PlayPause(self):
            self._svc.PlayPause()

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE)
        def Stop(self):
            self._svc.Stop()

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE)
        def Play(self):
            self._svc.Play()

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE, in_signature="x")
        def Seek(self, Offset: int):
            self._svc.Seek(int(Offset))

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE, in_signature="ox")
        def SetPosition(self, TrackId: str, Position: int):
            self._svc.SetPosition(str(TrackId), int(Position))

        @dbus.service.method(MPRIS2Service.PLAYER_INTERFACE, in_signature="s")
        def OpenUri(self, Uri: str):
            self._svc.OpenUri(str(Uri))

        @dbus.service.signal(MPRIS2Service.PLAYER_INTERFACE, signature="x")
        def Seeked(self, Position: int):
            pass

else:
    _DBusExportObject = None  # type: ignore


# Alias for backward and mock compatibility
MPRIS2PlayerMockService = MPRIS2Service
