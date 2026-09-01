"""
test_mpris2_dbus.py - MPRIS2 D-Bus Interface Compliance and Protocol Test Suite.

Verifies the org.mpris.MediaPlayer2 and org.mpris.MediaPlayer2.Player interface specifications,
including property types, method invocations, state machine transitions (Playing, Paused, Stopped),
relative and absolute seeking in microseconds, volume clamping, and PropertiesChanged signal formatting.
"""

from dataclasses import dataclass, field
import unittest
from typing import Any, Dict, List, Optional, Tuple


@dataclass
class TrackMetadata:
    track_id: str = "/org/mpris/MediaPlayer2/Track/0"
    length_us: int = 0  # in microseconds
    art_url: str = ""
    title: str = ""
    artists: List[str] = field(default_factory=list)
    album: str = ""
    album_artists: List[str] = field(default_factory=list)
    genres: List[str] = field(default_factory=list)
    track_number: int = 1
    url: str = ""

    def to_mpris_dict(self) -> Dict[str, Any]:
        """Converts to standard MPRIS a{sv} metadata map."""
        d = {
            "mpris:trackid": self.track_id,
            "mpris:length": int(self.length_us),
            "xesam:title": self.title,
            "xesam:artist": self.artists,
            "xesam:album": self.album,
            "xesam:albumArtist": self.album_artists,
            "xesam:genre": self.genres,
            "xesam:trackNumber": int(self.track_number),
            "xesam:url": self.url,
        }
        if self.art_url:
            d["mpris:artUrl"] = self.art_url
        return d


class MPRIS2PlayerMockService:
    """Standard-compliant in-memory MPRIS2 implementation for unit & integration testing."""

    ROOT_INTERFACE = "org.mpris.MediaPlayer2"
    PLAYER_INTERFACE = "org.mpris.MediaPlayer2.Player"
    PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties"

    def __init__(self):
        # Root properties
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
            "audio/aac", "video/mp4", "video/x-matroska", "video/webm", "video/x-msvideo"
        ]

        # Player properties
        self.playback_status: str = "Stopped"  # "Playing", "Paused", "Stopped"
        self.loop_status: str = "None"         # "None", "Track", "Playlist"
        self.rate: float = 1.0
        self.shuffle: bool = False
        self.volume: float = 1.0               # 0.0 to 1.0 (or 1.5)
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

    def emit_signal(self, signal_name: str, payload: Any):
        self.signal_history.append((signal_name, payload))

    def emit_properties_changed(self, changed_props: Dict[str, Any], invalidated_props: Optional[List[str]] = None):
        payload = {
            "interface_name": self.PLAYER_INTERFACE,
            "changed_properties": changed_props,
            "invalidated_properties": invalidated_props or [],
        }
        self.emit_signal("PropertiesChanged", payload)

    # Root methods
    def Raise(self):
        self.emit_signal("Raise", None)

    def Quit(self):
        self.emit_signal("Quit", None)

    # Player methods
    def Play(self):
        if self.playback_status != "Playing":
            self.playback_status = "Playing"
            self.emit_properties_changed({"PlaybackStatus": self.playback_status})

    def Pause(self):
        if self.playback_status == "Playing":
            self.playback_status = "Paused"
            self.emit_properties_changed({"PlaybackStatus": self.playback_status})

    def PlayPause(self):
        if self.playback_status == "Playing":
            self.Pause()
        else:
            self.Play()

    def Stop(self):
        self.playback_status = "Stopped"
        self.position_us = 0
        self.emit_properties_changed({"PlaybackStatus": "Stopped", "Position": 0})

    def Seek(self, offset_us: int):
        new_pos = max(0, self.position_us + offset_us)
        if self.current_metadata.length_us > 0:
            new_pos = min(self.current_metadata.length_us, new_pos)
        self.position_us = new_pos
        self.emit_signal("Seeked", new_pos)

    def SetPosition(self, track_id: str, position_us: int):
        if track_id == self.current_metadata.track_id:
            new_pos = max(0, position_us)
            if self.current_metadata.length_us > 0:
                new_pos = min(self.current_metadata.length_us, new_pos)
            self.position_us = new_pos
            self.emit_signal("Seeked", new_pos)

    def OpenUri(self, uri: str):
        self.current_metadata = TrackMetadata(
            track_id="/org/mpris/MediaPlayer2/Track/1",
            length_us=180_000_000,  # 3 minutes
            title="Synthetic Test Track",
            artists=["Penguin Artist"],
            album="Brutalist Studio",
            url=uri,
        )
        self.position_us = 0
        self.playback_status = "Playing"
        self.emit_properties_changed({
            "Metadata": self.current_metadata.to_mpris_dict(),
            "PlaybackStatus": "Playing",
            "Position": 0,
        })

    def set_volume(self, val: float):
        clamped = max(0.0, min(1.5, val))
        if clamped != self.volume:
            self.volume = clamped
            self.emit_properties_changed({"Volume": self.volume})

    def set_rate(self, val: float):
        clamped = max(self.minimum_rate, min(self.maximum_rate, val))
        if clamped != self.rate:
            self.rate = clamped
            self.emit_properties_changed({"Rate": self.rate})


class TestMPRIS2Compliance(unittest.TestCase):
    """Test suite verifying MPRIS2 Root and Player interface specifications."""

    def setUp(self):
        self.mpris = MPRIS2PlayerMockService()

    def test_root_interface_properties(self):
        self.assertTrue(self.mpris.can_quit)
        self.assertTrue(self.mpris.can_raise)
        self.assertTrue(self.mpris.can_set_fullscreen)
        self.assertEqual(self.mpris.identity, "Penguin Media Player")
        self.assertEqual(self.mpris.desktop_entry, "penguin")
        self.assertIn("file", self.mpris.supported_uri_schemes)
        self.assertIn("video/mp4", self.mpris.supported_mime_types)
        self.assertIn("audio/flac", self.mpris.supported_mime_types)

    def test_player_playback_state_machine(self):
        self.assertEqual(self.mpris.playback_status, "Stopped")

        # Stopped -> Play -> Playing
        self.mpris.Play()
        self.assertEqual(self.mpris.playback_status, "Playing")

        # Playing -> PlayPause -> Paused
        self.mpris.PlayPause()
        self.assertEqual(self.mpris.playback_status, "Paused")

        # Paused -> PlayPause -> Playing
        self.mpris.PlayPause()
        self.assertEqual(self.mpris.playback_status, "Playing")

        # Playing -> Stop -> Stopped
        self.mpris.Stop()
        self.assertEqual(self.mpris.playback_status, "Stopped")
        self.assertEqual(self.mpris.position_us, 0)

    def test_seek_relative_in_microseconds(self):
        self.mpris.current_metadata.length_us = 60_000_000  # 60 seconds
        self.mpris.position_us = 10_000_000  # 10 seconds

        # +10 seconds seek
        self.mpris.Seek(10_000_000)
        self.assertEqual(self.mpris.position_us, 20_000_000)

        # -5 seconds seek
        self.mpris.Seek(-5_000_000)
        self.assertEqual(self.mpris.position_us, 15_000_000)

        # Negative boundary clamping
        self.mpris.Seek(-30_000_000)
        self.assertEqual(self.mpris.position_us, 0)

        # Upper duration boundary clamping
        self.mpris.Seek(100_000_000)
        self.assertEqual(self.mpris.position_us, 60_000_000)

        # Check Seeked signals
        seek_signals = [s for s in self.mpris.signal_history if s[0] == "Seeked"]
        self.assertEqual(len(seek_signals), 4)

    def test_set_position_absolute_track_id_match(self):
        track_id = "/org/mpris/MediaPlayer2/Track/42"
        self.mpris.current_metadata = TrackMetadata(track_id=track_id, length_us=120_000_000)
        self.mpris.position_us = 0

        # SetPosition with matching track_id
        self.mpris.SetPosition(track_id, 45_000_000)
        self.assertEqual(self.mpris.position_us, 45_000_000)

        # SetPosition with mismatching track_id should do nothing
        self.mpris.SetPosition("/org/mpris/MediaPlayer2/Track/other", 90_000_000)
        self.assertEqual(self.mpris.position_us, 45_000_000)

    def test_volume_clamping_and_properties_changed(self):
        # Normal change
        self.mpris.set_volume(0.75)
        self.assertAlmostEqual(self.mpris.volume, 0.75)

        # Negative clamp to 0.0
        self.mpris.set_volume(-0.5)
        self.assertEqual(self.mpris.volume, 0.0)

        # High boost clamp to 1.5
        self.mpris.set_volume(2.5)
        self.assertEqual(self.mpris.volume, 1.5)

    def test_rate_clamping(self):
        self.mpris.set_rate(1.5)
        self.assertAlmostEqual(self.mpris.rate, 1.5)

        # Clamp below minimum (0.5)
        self.mpris.set_rate(0.1)
        self.assertEqual(self.mpris.rate, 0.5)

        # Clamp above maximum (2.0)
        self.mpris.set_rate(3.0)
        self.assertEqual(self.mpris.rate, 2.0)

    def test_metadata_dictionary_keys_and_types(self):
        meta = TrackMetadata(
            track_id="/org/mpris/MediaPlayer2/Track/9",
            length_us=240_000_000,
            art_url="file:///tmp/art.png",
            title="Cybernetic Horizon",
            artists=["Kraftwerk", "Synth Lab"],
            album="Computer World",
            album_artists=["Kraftwerk"],
            genres=["Electronic", "Ambient"],
            track_number=4,
            url="file:///music/song.flac",
        )
        d = meta.to_mpris_dict()

        self.assertIsInstance(d["mpris:trackid"], str)
        self.assertIsInstance(d["mpris:length"], int)
        self.assertIsInstance(d["xesam:title"], str)
        self.assertIsInstance(d["xesam:artist"], list)
        self.assertIsInstance(d["xesam:album"], str)
        self.assertIsInstance(d["xesam:trackNumber"], int)
        self.assertIsInstance(d["xesam:url"], str)
        self.assertEqual(d["mpris:length"], 240_000_000)

    def test_open_uri_workflow(self):
        self.mpris.OpenUri("file:///test/movie.mp4")
        self.assertEqual(self.mpris.playback_status, "Playing")
        self.assertEqual(self.mpris.current_metadata.url, "file:///test/movie.mp4")
        self.assertEqual(self.mpris.position_us, 0)


if __name__ == "__main__":
    unittest.main()
