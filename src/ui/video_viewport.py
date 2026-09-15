"""
video_viewport.py - Viewfinder Video Viewport, Safe-Area Reticles, OSD Telemetry HUD, and Tactile Dock.

Features:
- Borderless native video viewport embedding model.
- Technical safe-area reticles: Action-Safe 90%, Title-Safe 80%, Center crosshairs (+), and technical tags.
- Floating Diagnostics OSD HUD: Real-time FPS, dropped frames, bitrate, resolution, render time, and A/V skew.
- Mechanical SMPTE tick scrubber integration.
- Tactile bottom video control dock: Frame stepping, speed scaling, stream selector combos, screenshot exporter, A-B looping, night compressor.
"""

from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional, Tuple

from src.engine.smpte import SMPTETimecode
from src.ui.scrubber import MechanicalTickScrubber
from src.ui.theme import (
    BG_DEEP_OBSIDIAN,
    SURFACE_PANEL_BASE,
    SURFACE_RAISED,
    GRID_STRUCTURAL_BORDER,
    GRID_LINE_ACTIVE,
    TEXT_HIGH_CONTRAST,
    TEXT_SECONDARY_DIM,
    TEXT_MUTED,
    ACCENT_SAFETY_ORANGE,
    ACCENT_SIGNAL_LIME,
    ACCENT_TELEMETRY_CYAN,
    ACCENT_CLIP_RED,
)


@dataclass
class DiagnosticsData:
    """Real-time hardware & stream telemetry diagnostics metrics."""
    fps: float = 0.0
    nominal_fps: float = 0.0
    dropped_frames: int = 0
    video_codec: str = "none"
    audio_codec: str = "none"
    video_bitrate: int = 0
    audio_bitrate: int = 0
    video_width: int = 0
    video_height: int = 0
    color_primaries: str = "BT.709"
    color_depth: str = "8-bit"
    render_time_ms: float = 0.0
    av_skew_ms: float = 0.0


class DiagnosticsHUDWidget:
    """
    Floating OSD Hardware & Stream Diagnostics HUD Overlay.
    Renders 4-row high-density telemetry readouts over viewport.
    """

    def __init__(self, width: int = 380, height: int = 110):
        self._width = width
        self._height = height
        self._visible = True
        self._diag = DiagnosticsData()

    @property
    def is_visible(self) -> bool:
        return self._visible

    @property
    def diagnostics(self) -> DiagnosticsData:
        return self._diag

    def set_visible(self, visible: bool):
        self._visible = bool(visible)

    def toggle_visible(self):
        self._visible = not self._visible

    def update_diagnostics(self, diag: DiagnosticsData):
        self._diag = diag

    def format_rows(self) -> List[str]:
        """Formats 4 diagnostic telemetry rows."""
        d = self._diag
        fps_str = f"FPS: {d.fps:.2f} / {d.nominal_fps:.2f}" if d.nominal_fps > 0 else f"FPS: {d.fps:.2f}"
        drop_str = f"DROP: {d.dropped_frames}"
        v_bitrate_kbps = d.video_bitrate // 1000 if d.video_bitrate > 0 else 0
        v_codec_str = f"V_CODEC: {d.video_codec.upper()} {v_bitrate_kbps} kbps" if v_bitrate_kbps else f"V_CODEC: {d.video_codec.upper()}"
        res_str = f"RES: {d.video_width}x{d.video_height}" if d.video_width > 0 else "RES: --"
        a_codec_str = f"A_CODEC: {d.audio_codec.upper()}"
        color_str = f"COLOR: {d.color_primaries} {d.color_depth}"
        render_str = f"RENDER: {d.render_time_ms:.2f}ms"
        skew_str = f"A/V SKEW: {d.av_skew_ms:+.3f}ms"

        return [
            "TELEMETRY OSD // HARDWARE DIAGNOSTICS",
            f"{fps_str.ljust(22)} {drop_str}",
            f"{v_codec_str.ljust(24)} {res_str}",
            f"{a_codec_str.ljust(24)} {color_str}",
            f"{render_str.ljust(22)} {skew_str}",
        ]


class SafeAreaReticles:
    """
    Technical Safe-Area Broadcast Reticles Overlay Controller.
    Computes coordinates for 90% action-safe, 80% title-safe, and center crosshairs (+).
    """

    def __init__(self, action_safe_ratio: float = 0.90, title_safe_ratio: float = 0.80):
        self._action_safe_ratio = action_safe_ratio
        self._title_safe_ratio = title_safe_ratio
        self._crosshair_len_px = 16
        self._crosshair_ring_radius_px = 4
        self._corner_bracket_len_px = 12
        self._visible = True

    @property
    def is_visible(self) -> bool:
        return self._visible

    def set_visible(self, visible: bool):
        self._visible = bool(visible)

    def toggle_visible(self):
        self._visible = not self._visible

    def calculate_bounds(self, width: int, height: int) -> Dict[str, Any]:
        """Calculates exact pixel rects and center crosshair for viewport."""
        w, h = float(width), float(height)
        action_w = w * self._action_safe_ratio
        action_h = h * self._action_safe_ratio
        action_x = (w - action_w) / 2.0
        action_y = (h - action_h) / 2.0

        title_w = w * self._title_safe_ratio
        title_h = h * self._title_safe_ratio
        title_x = (w - title_w) / 2.0
        title_y = (h - title_h) / 2.0

        center_x = w / 2.0
        center_y = h / 2.0

        return {
            "action_safe": {
                "x": action_x,
                "y": action_y,
                "width": action_w,
                "height": action_h,
                "ratio": self._action_safe_ratio,
                "label": "ACTION SAFE 90%",
                "bracket_len": self._corner_bracket_len_px,
            },
            "title_safe": {
                "x": title_x,
                "y": title_y,
                "width": title_w,
                "height": title_h,
                "ratio": self._title_safe_ratio,
                "label": "TITLE SAFE 80%",
            },
            "center_crosshairs": {
                "cx": center_x,
                "cy": center_y,
                "arm_length": self._crosshair_len_px,
                "ring_radius": self._crosshair_ring_radius_px,
            },
        }


class ViewfinderWidget:
    """
    Video Viewfinder Mode Container.
    Assembles video surface, safe reticles, telemetry HUD, tick scrubber, and control dock.
    """

    def __init__(self, playback_engine=None, width: int = 800, height: int = 600):
        self._width = max(400, int(width))
        self._height = max(300, int(height))
        self._engine = playback_engine

        # Subcomponents
        self._reticles = SafeAreaReticles()
        self._hud = DiagnosticsHUDWidget()
        self._scrubber = MechanicalTickScrubber(width=self._width, height=38)

        # State
        self._is_fullscreen = False
        self._night_mode = False
        self._deband = False
        self._aspect_ratios = ["auto", "16:9", "2.39:1", "4:3", "1:1", "21:9"]
        self._aspect_ratio_index = 0
        self._loop_a_ms: Optional[int] = None
        self._loop_b_ms: Optional[int] = None

    @property
    def scrubber(self) -> MechanicalTickScrubber:
        return self._scrubber

    @property
    def hud(self) -> DiagnosticsHUDWidget:
        return self._hud

    @property
    def reticles(self) -> SafeAreaReticles:
        return self._reticles

    def is_osd_visible(self) -> bool:
        return self._hud.is_visible

    def is_reticles_visible(self) -> bool:
        return self._reticles.is_visible

    def is_fullscreen(self) -> bool:
        return self._is_fullscreen

    def toggle_osd(self):
        self._hud.toggle_visible()

    def toggle_reticles(self):
        self._reticles.toggle_visible()

    def toggle_fullscreen(self):
        self._is_fullscreen = not self._is_fullscreen

    def toggle_night_mode(self) -> bool:
        self._night_mode = not self._night_mode
        if self._engine and hasattr(self._engine, "toggle_night_mode"):
            self._engine.toggle_night_mode()
        return self._night_mode

    def toggle_deband(self) -> bool:
        self._deband = not self._deband
        if self._engine and hasattr(self._engine, "toggle_deband"):
            self._engine.toggle_deband()
        return self._deband

    def cycle_aspect_ratio(self) -> str:
        self._aspect_ratio_index = (self._aspect_ratio_index + 1) % len(self._aspect_ratios)
        ratio = self._aspect_ratios[self._aspect_ratio_index]
        if self._engine and hasattr(self._engine, "set_aspect_ratio"):
            self._engine.set_aspect_ratio(ratio)
        return ratio

    def set_loop_point_a(self, pos_ms: Optional[int] = None):
        if pos_ms is None and self._engine and hasattr(self._engine, "position_ms"):
            pos_ms = self._engine.position_ms
        self._loop_a_ms = pos_ms or 0
        if self._engine and hasattr(self._engine, "set_loop_point_a"):
            self._engine.set_loop_point_a(self._loop_a_ms)

    def set_loop_point_b(self, pos_ms: Optional[int] = None):
        if pos_ms is None and self._engine and hasattr(self._engine, "position_ms"):
            pos_ms = self._engine.position_ms
        self._loop_b_ms = pos_ms or 0
        if self._engine and hasattr(self._engine, "set_loop_point_b"):
            self._engine.set_loop_point_b(self._loop_b_ms)

    def clear_loop(self):
        self._loop_a_ms = None
        self._loop_b_ms = None
        if self._engine and hasattr(self._engine, "clear_loop"):
            self._engine.clear_loop()

    def resize(self, width: int, height: int):
        self._width = max(400, int(width))
        self._height = max(300, int(height))
        self._scrubber.resize(self._width, 38)
