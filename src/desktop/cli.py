"""
src/desktop/cli.py - Command-Line Interface and Single-Instance IPC Forwarding for Penguin.

Features:
- CLI argument parsing via PenguinCLIParser matching standard Penguin specifications.
- Media file, directory, and stream URL ingestion.
- Playback controls: --volume [0-100], --speed [0.5-2.0], --sub, --loop, --shuffle, --eq.
- UI mode switches: -a/--audio, -v/--video, -f/--fullscreen.
- Single-instance IPC forwarding over D-Bus SessionBus (org.mpris.MediaPlayer2.penguin).
- Automated test mode (--test) and headless diagnostics (--dev-telemetry).
"""

from __future__ import annotations
import argparse
from dataclasses import dataclass, field
import logging
import os
from pathlib import Path
import subprocess
import sys
from typing import Any, Dict, List, Optional, Tuple, Union
from urllib.parse import urlparse

logger = logging.getLogger("Penguin.Desktop.CLI")

# Optional dbus-python import
try:
    import dbus
    DBUS_AVAILABLE = True
except ImportError:
    DBUS_AVAILABLE = False
    dbus = None


# =============================================================================
# 1. CLI OPTIONS DATACLASS
# =============================================================================

@dataclass
class PenguinCLIOptions:
    """Structured strongly-typed command-line options for Penguin."""
    files: List[str] = field(default_factory=list)
    audio: bool = False
    video: bool = False
    fullscreen: bool = False
    volume: Optional[int] = None
    speed: Optional[float] = None
    sub: Optional[str] = None
    loop: str = "none"
    shuffle: bool = False
    eq: Optional[str] = None
    ipc_action: Optional[str] = None
    toggle_pause: bool = False
    next: bool = False
    prev: bool = False
    stop: bool = False
    test_mode: bool = False
    dev_telemetry: bool = False

    @classmethod
    def from_namespace(cls, ns: argparse.Namespace) -> PenguinCLIOptions:
        """Converts argparse.Namespace to PenguinCLIOptions."""
        ipc_act = getattr(ns, "action", None)
        if getattr(ns, "toggle_pause", False):
            ipc_act = "play-pause"
        elif getattr(ns, "next", False):
            ipc_act = "next"
        elif getattr(ns, "prev", False):
            ipc_act = "previous"
        elif getattr(ns, "stop", False):
            ipc_act = "stop"

        return cls(
            files=list(getattr(ns, "files", []) or []),
            audio=bool(getattr(ns, "audio", False)),
            video=bool(getattr(ns, "video", False)),
            fullscreen=bool(getattr(ns, "fullscreen", False)),
            volume=getattr(ns, "volume", None),
            speed=getattr(ns, "speed", None),
            sub=getattr(ns, "sub", None),
            loop=str(getattr(ns, "loop", "none") or "none"),
            shuffle=bool(getattr(ns, "shuffle", False)),
            eq=getattr(ns, "eq", None),
            ipc_action=ipc_act,
            toggle_pause=bool(getattr(ns, "toggle_pause", False)),
            next=bool(getattr(ns, "next", False)),
            prev=bool(getattr(ns, "prev", False)),
            stop=bool(getattr(ns, "stop", False)),
            test_mode=bool(getattr(ns, "test_mode", False)),
            dev_telemetry=bool(getattr(ns, "dev_telemetry", False)),
        )


# =============================================================================
# 2. CLI ARGUMENT PARSER BUILDER
# =============================================================================

class PenguinCLIParser:
    """Standard FreeDesktop & Brutalist CLI argument parser for Penguin Media Player."""

    @classmethod
    def build_parser(cls) -> argparse.ArgumentParser:
        """Constructs the master argparse.ArgumentParser for Penguin."""
        parser = argparse.ArgumentParser(
            prog="penguin",
            description="Tactile Digital Brutalist Audio & Video Player for Linux.",
        )

        # Positional arguments: media files, playlists, directories, or stream URLs
        parser.add_argument(
            "files",
            nargs="*",
            metavar="FILE_OR_URL",
            help="Media file paths, directories, or stream URLs to open and play.",
        )

        # Mode options (Mutually exclusive: audio vs video)
        mode_group = parser.add_mutually_exclusive_group()
        mode_group.add_argument("-a", "--audio", action="store_true", help="Force Hi-Fi Audio Deck UI mode.")
        mode_group.add_argument("-v", "--video", action="store_true", help="Force Viewfinder Video UI mode.")

        # Window & display flags
        parser.add_argument("-f", "--fullscreen", action="store_true", help="Launch application in fullscreen mode.")

        # Playback parameters
        parser.add_argument(
            "--volume",
            type=int,
            choices=range(0, 101),
            metavar="[0-100]",
            help="Set initial volume.",
        )
        parser.add_argument(
            "--speed",
            type=float,
            help="Set initial playback speed (0.5 to 2.0).",
        )
        parser.add_argument(
            "--sub",
            type=str,
            metavar="SUBTITLE_FILE",
            help="Attach external subtitle file (.srt/.ass/.vtt).",
        )
        parser.add_argument(
            "--loop",
            choices=["none", "track", "playlist"],
            default="none",
            help="Set playlist loop mode.",
        )
        parser.add_argument(
            "--shuffle",
            action="store_true",
            help="Enable random playlist shuffle.",
        )
        parser.add_argument(
            "--eq",
            type=str,
            metavar="PRESET",
            help="Apply equalizer preset by name.",
        )

        # IPC Remote Action flags
        ipc_group = parser.add_argument_group("IPC Remote Actions")
        ipc_group.add_argument(
            "--action",
            type=str,
            metavar="ACTION",
            help="Send IPC control action (play-pause, next, prev, stop, raise).",
        )
        ipc_group.add_argument(
            "--toggle-pause",
            action="store_true",
            help="Send play/pause toggle to active instance.",
        )
        ipc_group.add_argument(
            "--next",
            action="store_true",
            help="Advance active instance to next track.",
        )
        ipc_group.add_argument(
            "--prev", "--previous",
            dest="prev",
            action="store_true",
            help="Reverse active instance to previous track.",
        )
        ipc_group.add_argument(
            "--stop",
            action="store_true",
            help="Stop playback on active instance.",
        )

        # Diagnostics & Verification
        parser.add_argument(
            "--test", "--headless-test",
            dest="test_mode",
            action="store_true",
            help="Run automated test suite.",
        )
        parser.add_argument(
            "--dev-telemetry",
            action="store_true",
            help="Enable OSD telemetry HUD on launch.",
        )
        parser.add_argument(
            "-V", "--version",
            action="version",
            version="Penguin 1.0.0 (Tactile Digital Brutalism)",
        )

        return parser


def parse_args(args: Optional[List[str]] = None) -> argparse.Namespace:
    """Parses command-line arguments and returns argparse.Namespace."""
    parser = PenguinCLIParser.build_parser()
    return parser.parse_args(args)


# =============================================================================
# 3. SINGLE-INSTANCE IPC FORWARDING VIA D-BUS
# =============================================================================

def is_mpris2_instance_running(bus_name: str = "org.mpris.MediaPlayer2.penguin") -> bool:
    """Checks whether an active Penguin MPRIS2 service is currently registered on D-Bus."""
    if DBUS_AVAILABLE:
        try:
            session_bus = dbus.SessionBus()
            return bool(session_bus.name_has_owner(bus_name))
        except Exception:
            pass

    # Fallback to dbus-send or gdbus CLI
    try:
        res = subprocess.run(
            ["gdbus", "call", "--session", "--dest", "org.freedesktop.DBus",
             "--object-path", "/org/freedesktop/DBus", "--method",
             "org.freedesktop.DBus.NameHasOwner", bus_name],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=1.0
        )
        return "true" in res.stdout.lower()
    except Exception:
        return False


def forward_ipc_command(options: Union[argparse.Namespace, PenguinCLIOptions]) -> bool:
    """
    Forwards remote playback action or file list to a running Penguin instance via D-Bus.
    Returns True if successfully sent to a running instance, False otherwise.
    """
    if isinstance(options, argparse.Namespace):
        opts = PenguinCLIOptions.from_namespace(options)
    else:
        opts = options

    bus_name = "org.mpris.MediaPlayer2.penguin"
    obj_path = "/org/mpris/MediaPlayer2"

    if not is_mpris2_instance_running(bus_name):
        return False

    # Attempt forwarding via dbus-python
    if DBUS_AVAILABLE:
        try:
            session_bus = dbus.SessionBus()
            root_iface = dbus.Interface(
                session_bus.get_object(bus_name, obj_path),
                dbus_interface="org.mpris.MediaPlayer2"
            )
            player_iface = dbus.Interface(
                session_bus.get_object(bus_name, obj_path),
                dbus_interface="org.mpris.MediaPlayer2.Player"
            )

            handled = False

            # Forward files
            if opts.files:
                for f in opts.files:
                    uri = f
                    if "://" not in uri:
                        abs_p = Path(f).resolve()
                        uri = f"file://{abs_p}"
                    player_iface.OpenUri(uri)
                root_iface.Raise()
                handled = True

            # Forward action
            action = (opts.ipc_action or "").lower()
            if action in ("play-pause", "playpause", "toggle-pause"):
                player_iface.PlayPause()
                handled = True
            elif action == "next":
                player_iface.Next()
                handled = True
            elif action in ("prev", "previous"):
                player_iface.Previous()
                handled = True
            elif action == "stop":
                player_iface.Stop()
                handled = True
            elif action == "raise":
                root_iface.Raise()
                handled = True

            return handled
        except Exception as e:
            logger.warning(f"Failed to forward IPC command via dbus-python: {e}")

    # Fallback to gdbus CLI
    try:
        handled = False
        if opts.files:
            for f in opts.files:
                uri = f
                if "://" not in uri:
                    abs_p = Path(f).resolve()
                    uri = f"file://{abs_p}"
                subprocess.run(
                    ["gdbus", "call", "--session", "--dest", bus_name,
                     "--object-path", obj_path, "--method",
                     "org.mpris.MediaPlayer2.Player.OpenUri", uri],
                    check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
                )
            subprocess.run(
                ["gdbus", "call", "--session", "--dest", bus_name,
                 "--object-path", obj_path, "--method",
                 "org.mpris.MediaPlayer2.Raise"],
                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
            handled = True

        action = (opts.ipc_action or "").lower()
        if action in ("play-pause", "playpause", "toggle-pause"):
            subprocess.run(
                ["gdbus", "call", "--session", "--dest", bus_name,
                 "--object-path", obj_path, "--method",
                 "org.mpris.MediaPlayer2.Player.PlayPause"],
                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
            handled = True
        elif action == "next":
            subprocess.run(
                ["gdbus", "call", "--session", "--dest", bus_name,
                 "--object-path", obj_path, "--method",
                 "org.mpris.MediaPlayer2.Player.Next"],
                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
            handled = True
        elif action in ("prev", "previous"):
            subprocess.run(
                ["gdbus", "call", "--session", "--dest", bus_name,
                 "--object-path", obj_path, "--method",
                 "org.mpris.MediaPlayer2.Player.Previous"],
                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
            handled = True
        elif action == "stop":
            subprocess.run(
                ["gdbus", "call", "--session", "--dest", bus_name,
                 "--object-path", obj_path, "--method",
                 "org.mpris.MediaPlayer2.Player.Stop"],
                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
            handled = True

        return handled
    except Exception as e:
        logger.warning(f"Failed to forward IPC command via gdbus: {e}")
        return False


# =============================================================================
# 4. MAIN ENTRYPOINT
# =============================================================================

def main(argv: Optional[List[str]] = None) -> int:
    """Main CLI execution entrypoint."""
    if argv is None:
        argv = sys.argv[1:]

    parser = PenguinCLIParser.build_parser()
    args = parser.parse_args(argv)
    opts = PenguinCLIOptions.from_namespace(args)

    # If automated test mode requested
    if opts.test_mode:
        from run_tests import PenguinTestRunner
        return PenguinTestRunner.run(tier="all", headless=True)

    # Check if IPC action or file ingestion should be forwarded to existing instance
    has_ipc_flags = any([
        opts.toggle_pause, opts.next, opts.prev, opts.stop, opts.ipc_action
    ])
    if has_ipc_flags or (opts.files and is_mpris2_instance_running()):
        forwarded = forward_ipc_command(opts)
        if forwarded:
            print("[IPC] Command successfully forwarded to running Penguin instance.")
            return 0

    # Otherwise, launch native Qt application or PlaybackEngine
    # Priority: If C++ binary exists, exec; otherwise run Python engine/GUI
    cpp_binary = Path(__file__).resolve().parent.parent.parent / "penguin"
    if cpp_binary.exists() and os.access(cpp_binary, os.X_OK):
        try:
            return subprocess.call([str(cpp_binary)] + argv)
        except Exception:
            pass

    print("Penguin Media Player — Initialized (Tactile Digital Brutalism)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
