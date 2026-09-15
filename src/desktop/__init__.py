"""
Penguin Media Player — Linux Desktop Integration Layer.

Modules:
- mpris2: MPRIS2 D-Bus service (org.mpris.MediaPlayer2 and org.mpris.MediaPlayer2.Player).
- cli: FreeDesktop command-line argument parser, options dataclass, and single-instance IPC forwarding.
- audio_routing: PipeWire & PulseAudio low-latency audio routing, sample rate negotiation, and sink control.
- packaging: FreeDesktop .desktop generation, SVG icon packaging, and spec validator.
"""

from src.desktop.mpris2 import (
    MPRIS2Service,
    MPRIS2PlayerMockService,
    TrackMetadata,
)
from src.desktop.cli import (
    PenguinCLIParser,
    PenguinCLIOptions,
    parse_args,
    forward_ipc_command,
    is_mpris2_instance_running,
)
from src.desktop.audio_routing import (
    AudioRouter,
    AudioBackend,
    AudioSinkInfo,
    AudioRoutingConfig,
    AudioChannelLayout,
)
from src.desktop.packaging import (
    DesktopPackager,
)

__all__ = [
    # MPRIS2 D-Bus
    "MPRIS2Service",
    "MPRIS2PlayerMockService",
    "TrackMetadata",
    # CLI & Single-Instance IPC
    "PenguinCLIParser",
    "PenguinCLIOptions",
    "parse_args",
    "forward_ipc_command",
    "is_mpris2_instance_running",
    # Audio Routing
    "AudioRouter",
    "AudioBackend",
    "AudioSinkInfo",
    "AudioRoutingConfig",
    "AudioChannelLayout",
    # Desktop Packaging
    "DesktopPackager",
]
