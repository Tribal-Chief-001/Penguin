"""
theme.py - Tactile Digital Brutalism Design System & Styling Engine for Penguin.

Defines:
- Exact color palette tokens (Obsidian, Grid, Accents, Telemetry, Clip)
- Monospace & Swiss typography configuration
- Global QSS stylesheet generators with razor-sharp 0px border-radius and 1px grids
- Custom UI component styling definitions
"""

from typing import Dict, Any, Optional, Tuple


# =============================================================================
# Color Palette Tokens
# =============================================================================

BG_DEEP_OBSIDIAN = "#070709"        # Root window background, canvas, table viewport
SURFACE_PANEL_BASE = "#0B0B0E"      # Docks, header bars, card backgrounds, dropdowns
SURFACE_RAISED = "#141418"          # Hover state, slider grooves, inactive LED segments
GRID_STRUCTURAL_BORDER = "#1E1E24"  # 1px architectural grid lines, container borders
GRID_LINE_ACTIVE = "#333342"        # Active borders, hover outlines, separators
TEXT_HIGH_CONTRAST = "#FFFFFF"      # Primary labels, active timecodes, titles
TEXT_SECONDARY_DIM = "#777788"      # Secondary labels, artists, remaining timecodes
TEXT_MUTED = "#444455"              # Past lyric lines, disabled buttons, dB ticks
ACCENT_SAFETY_ORANGE = "#FF4400"    # Playhead needle, video mode, warning, loop point A
ACCENT_SIGNAL_LIME = "#CCFF00"      # Audio mode, active VU LEDs, active lyrics, EQ boost
ACCENT_TELEMETRY_CYAN = "#00E5FF"   # Safe reticles, chapter diamonds, OSD HUD, audio badge
ACCENT_CLIP_RED = "#FF2200"         # VU meter clipping (>0 dB), CLIP alert badge

# Monolithic Kinetic Precision & Smoked Glass Tokens
GLASS_SMOKED_OBSIDIAN = "rgba(7, 7, 9, 0.85)"
GLASS_PANEL_TRANSLUCENT = "rgba(14, 14, 20, 0.88)"
GLASS_BORDER_SUBTLE = "rgba(255, 255, 255, 0.08)"
GLASS_BORDER_GLOW = "rgba(0, 229, 255, 0.30)"
GLOW_LASER_ORANGE = "rgba(255, 68, 0, 0.30)"
GLOW_LASER_LIME = "rgba(204, 255, 0, 0.30)"
GLOW_LASER_CYAN = "rgba(0, 229, 255, 0.35)"

# Typography Font Stacks
MONOSPACE_FONT_FAMILY = "JetBrains Mono, DejaVu Sans Mono, Liberation Mono, Monospace, Courier New"
SANS_FONT_FAMILY = "Inter, Liberation Sans, DejaVu Sans, Helvetica Neue, Segoe UI, sans-serif"


class BrutalistTheme:
    """Centralized Tactile Digital Brutalism theme definition and styling."""

    BG_DEEP_OBSIDIAN = BG_DEEP_OBSIDIAN
    SURFACE_PANEL_BASE = SURFACE_PANEL_BASE
    SURFACE_RAISED = SURFACE_RAISED
    GRID_STRUCTURAL_BORDER = GRID_STRUCTURAL_BORDER
    GRID_LINE_ACTIVE = GRID_LINE_ACTIVE
    TEXT_HIGH_CONTRAST = TEXT_HIGH_CONTRAST
    TEXT_SECONDARY_DIM = TEXT_SECONDARY_DIM
    TEXT_MUTED = TEXT_MUTED
    ACCENT_SAFETY_ORANGE = ACCENT_SAFETY_ORANGE
    ACCENT_SIGNAL_LIME = ACCENT_SIGNAL_LIME
    ACCENT_TELEMETRY_CYAN = ACCENT_TELEMETRY_CYAN
    ACCENT_CLIP_RED = ACCENT_CLIP_RED

    GLASS_SMOKED_OBSIDIAN = GLASS_SMOKED_OBSIDIAN
    GLASS_PANEL_TRANSLUCENT = GLASS_PANEL_TRANSLUCENT
    GLASS_BORDER_SUBTLE = GLASS_BORDER_SUBTLE
    GLASS_BORDER_GLOW = GLASS_BORDER_GLOW
    GLOW_LASER_ORANGE = GLOW_LASER_ORANGE
    GLOW_LASER_LIME = GLOW_LASER_LIME
    GLOW_LASER_CYAN = GLOW_LASER_CYAN

    MONOSPACE_FONTS = ["JetBrains Mono", "DejaVu Sans Mono", "Liberation Mono", "Monospace", "Courier New"]
    SANS_FONTS = ["Inter", "Liberation Sans", "DejaVu Sans", "Helvetica Neue", "Segoe UI", "sans-serif"]

    @classmethod
    def get_color_tokens(cls) -> Dict[str, str]:
        """Returns dictionary of all color tokens."""
        return {
            "BG_DEEP_OBSIDIAN": cls.BG_DEEP_OBSIDIAN,
            "SURFACE_PANEL_BASE": cls.SURFACE_PANEL_BASE,
            "SURFACE_RAISED": cls.SURFACE_RAISED,
            "GRID_STRUCTURAL_BORDER": cls.GRID_STRUCTURAL_BORDER,
            "GRID_LINE_ACTIVE": cls.GRID_LINE_ACTIVE,
            "TEXT_HIGH_CONTRAST": cls.TEXT_HIGH_CONTRAST,
            "TEXT_SECONDARY_DIM": cls.TEXT_SECONDARY_DIM,
            "TEXT_MUTED": cls.TEXT_MUTED,
            "ACCENT_SAFETY_ORANGE": cls.ACCENT_SAFETY_ORANGE,
            "ACCENT_SIGNAL_LIME": cls.ACCENT_SIGNAL_LIME,
            "ACCENT_TELEMETRY_CYAN": cls.ACCENT_TELEMETRY_CYAN,
            "ACCENT_CLIP_RED": cls.ACCENT_CLIP_RED,
        }

    @classmethod
    def global_stylesheet(cls) -> str:
        """Returns the complete global QSS stylesheet enforcing Brutalist styling."""
        return f"""
        /* Penguin Tactile Digital Brutalism Global Stylesheet */
        QMainWindow, QWidget {{
            background-color: {cls.BG_DEEP_OBSIDIAN};
            color: {cls.TEXT_HIGH_CONTRAST};
            font-family: {MONOSPACE_FONT_FAMILY};
            border-radius: 0px;
            selection-background-color: {cls.ACCENT_SAFETY_ORANGE};
            selection-color: {cls.TEXT_HIGH_CONTRAST};
        }}

        /* Buttons */
        QPushButton {{
            background-color: {cls.SURFACE_PANEL_BASE};
            color: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            padding: 4px 10px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-weight: bold;
            font-size: 11px;
            min-height: 24px;
        }}
        QPushButton:hover {{
            background-color: {cls.SURFACE_RAISED};
            border: 1px solid {cls.GRID_LINE_ACTIVE};
        }}
        QPushButton:pressed {{
            background-color: {cls.GRID_LINE_ACTIVE};
            color: {cls.ACCENT_SAFETY_ORANGE};
        }}
        QPushButton:checked {{
            background-color: {cls.SURFACE_RAISED};
            border: 1px solid {cls.ACCENT_SAFETY_ORANGE};
            color: {cls.ACCENT_SAFETY_ORANGE};
        }}
        QPushButton:disabled {{
            background-color: {cls.BG_DEEP_OBSIDIAN};
            color: {cls.TEXT_MUTED};
            border: 1px solid {cls.SURFACE_RAISED};
        }}

        /* Text Input & Search Fields */
        QLineEdit {{
            background-color: {cls.SURFACE_PANEL_BASE};
            color: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            padding: 4px 8px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-size: 11px;
        }}
        QLineEdit:focus {{
            border: 1px solid {cls.ACCENT_TELEMETRY_CYAN};
        }}

        /* Sliders */
        QSlider::groove:horizontal {{
            height: 4px;
            background: {cls.SURFACE_RAISED};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
        }}
        QSlider::sub-page:horizontal {{
            background: {cls.ACCENT_SAFETY_ORANGE};
        }}
        QSlider::handle:horizontal {{
            background: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.TEXT_HIGH_CONTRAST};
            width: 10px;
            margin: -4px 0;
            border-radius: 0px;
        }}
        QSlider::handle:horizontal:hover {{
            background: {cls.ACCENT_SAFETY_ORANGE};
            border: 1px solid {cls.ACCENT_SAFETY_ORANGE};
        }}

        QSlider::groove:vertical {{
            width: 4px;
            background: {cls.SURFACE_RAISED};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
        }}
        QSlider::handle:vertical {{
            background: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.TEXT_HIGH_CONTRAST};
            height: 10px;
            margin: 0 -4px;
            border-radius: 0px;
        }}
        QSlider::handle:vertical:hover {{
            background: {cls.ACCENT_SIGNAL_LIME};
            border: 1px solid {cls.ACCENT_SIGNAL_LIME};
        }}

        /* Combo Boxes */
        QComboBox {{
            background-color: {cls.SURFACE_PANEL_BASE};
            color: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            padding: 3px 8px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-size: 11px;
            min-height: 24px;
        }}
        QComboBox:hover {{
            border: 1px solid {cls.GRID_LINE_ACTIVE};
        }}
        QComboBox::drop-down {{
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 18px;
            border-left: 1px solid {cls.GRID_STRUCTURAL_BORDER};
        }}
        QComboBox QAbstractItemView {{
            background-color: {cls.SURFACE_PANEL_BASE};
            color: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            selection-background-color: {cls.SURFACE_RAISED};
            selection-color: {cls.ACCENT_SAFETY_ORANGE};
        }}

        /* Tables & Lists */
        QTableWidget, QTableView, QListWidget, QListView {{
            background-color: {cls.BG_DEEP_OBSIDIAN};
            color: {cls.TEXT_HIGH_CONTRAST};
            gridline-color: {cls.GRID_STRUCTURAL_BORDER};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-size: 11px;
        }}
        QHeaderView::section {{
            background-color: {cls.SURFACE_PANEL_BASE};
            color: {cls.TEXT_SECONDARY_DIM};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            padding: 4px 6px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-weight: bold;
            font-size: 10px;
        }}

        /* Scrollbars */
        QScrollBar:vertical {{
            background: {cls.BG_DEEP_OBSIDIAN};
            width: 8px;
            margin: 0px;
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
        }}
        QScrollBar::handle:vertical {{
            background: {cls.GRID_STRUCTURAL_BORDER};
            min-height: 20px;
            border-radius: 0px;
        }}
        QScrollBar::handle:vertical:hover {{
            background: {cls.GRID_LINE_ACTIVE};
        }}
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
            height: 0px;
        }}
        """

    @classmethod
    def primary_button_stylesheet(cls) -> str:
        return f"""
        QPushButton {{
            background-color: {cls.SURFACE_PANEL_BASE};
            color: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            padding: 4px 10px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-weight: bold;
            font-size: 11px;
        }}
        QPushButton:hover {{
            background-color: {cls.SURFACE_RAISED};
            border: 1px solid {cls.GRID_LINE_ACTIVE};
        }}
        """

    @classmethod
    def accent_orange_button_stylesheet(cls) -> str:
        return f"""
        QPushButton {{
            background-color: {cls.ACCENT_SAFETY_ORANGE};
            color: {cls.TEXT_HIGH_CONTRAST};
            border: 1px solid {cls.ACCENT_SAFETY_ORANGE};
            border-radius: 0px;
            padding: 4px 12px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-weight: bold;
            font-size: 11px;
        }}
        QPushButton:hover {{
            background-color: #FF5511;
            border: 1px solid #FF5511;
        }}
        """

    @classmethod
    def accent_lime_button_stylesheet(cls) -> str:
        return f"""
        QPushButton {{
            background-color: {cls.ACCENT_SIGNAL_LIME};
            color: {cls.BG_DEEP_OBSIDIAN};
            border: 1px solid {cls.ACCENT_SIGNAL_LIME};
            border-radius: 0px;
            padding: 4px 12px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-weight: bold;
            font-size: 11px;
        }}
        QPushButton:hover {{
            background-color: #DDFF22;
            border: 1px solid #DDFF22;
        }}
        """

    @classmethod
    def panel_stylesheet(cls) -> str:
        return f"""
        QFrame, QWidget#Panel {{
            background-color: {cls.SURFACE_PANEL_BASE};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
        }}
        """

    @classmethod
    def table_stylesheet(cls) -> str:
        return f"""
        QTableView, QTableWidget {{
            background-color: {cls.BG_DEEP_OBSIDIAN};
            color: {cls.TEXT_HIGH_CONTRAST};
            gridline-color: {cls.GRID_STRUCTURAL_BORDER};
            border: 1px solid {cls.GRID_STRUCTURAL_BORDER};
            border-radius: 0px;
            selection-background-color: {cls.SURFACE_RAISED};
            selection-color: {cls.ACCENT_SIGNAL_LIME};
        }}
        """

    @classmethod
    def pill_button_stylesheet(cls) -> str:
        return f"""
        QPushButton {{
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(26, 26, 34, 0.9), stop:1 rgba(14, 14, 20, 0.95));
            color: #FFFFFF;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 12px;
            padding: 4px 12px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-weight: bold;
            font-size: 10px;
        }}
        QPushButton:hover {{
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(36, 36, 48, 0.95), stop:1 rgba(20, 20, 28, 0.98));
            border: 1px solid rgba(0, 229, 255, 0.6);
            color: #00E5FF;
        }}
        QPushButton:pressed {{
            background: {cls.ACCENT_SAFETY_ORANGE};
            border: 1px solid {cls.ACCENT_SAFETY_ORANGE};
            color: {cls.BG_DEEP_OBSIDIAN};
        }}
        """

    @classmethod
    def transport_dock_pill_stylesheet(cls) -> str:
        return """
        QWidget#TransportDock {{
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(20, 20, 28, 0.92), stop:1 rgba(10, 10, 14, 0.96));
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 20px;
        }}
        """

    @classmethod
    def floating_header_stylesheet(cls) -> str:
        return """
        QWidget#FloatingHeader {{
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(18, 18, 24, 0.90), stop:1 rgba(10, 10, 14, 0.94));
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 12px;
        }}
        """

    @classmethod
    def search_input_stylesheet(cls) -> str:
        return f"""
        QLineEdit {{
            background-color: rgba(16, 16, 22, 0.85);
            color: #FFFFFF;
            border: 1px solid rgba(255, 255, 255, 0.10);
            border-radius: 13px;
            padding: 4px 12px;
            font-family: {MONOSPACE_FONT_FAMILY};
            font-size: 11px;
        }}
        QLineEdit:focus {{
            border: 1px solid {cls.ACCENT_TELEMETRY_CYAN};
            background-color: rgba(20, 20, 28, 0.95);
        }}
        """

    @classmethod
    def video_equalizer_dialog_stylesheet(cls) -> str:
        return f"""
        QDialog {{
            background-color: {cls.SURFACE_PANEL_BASE};
            border: 1px solid {cls.GRID_LINE_ACTIVE};
            border-radius: 8px;
        }}
        """

