"""
src/desktop/packaging.py - Desktop Packaging, Icon Validation, and FreeDesktop Spec Tools.
"""

from __future__ import annotations
import configparser
import os
from pathlib import Path
from typing import Dict, List, Optional
import xml.etree.ElementTree as ET


class DesktopPackager:
    """Helper to generate, install, and validate desktop integration assets."""

    DESKTOP_ENTRY_TEMPLATE = """[Desktop Entry]
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
Categories=AudioVideo;Audio;Video;Player;Qt;
MimeType=audio/mpeg;audio/flac;audio/ogg;audio/opus;audio/x-wav;audio/aac;video/mp4;video/x-matroska;video/webm;video/quicktime;video/x-msvideo;
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
"""

    SVG_ICON_TEMPLATE = """<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" width="512" height="512">
  <rect width="512" height="512" fill="#070709"/>
  <rect x="32" y="32" width="448" height="448" fill="#0B0B0E" stroke="#1E1E24" stroke-width="4"/>
  <!-- Brutalist Penguin Silhouette with Safety Orange Accent -->
  <polygon points="256,96 160,320 200,416 312,416 352,320" fill="#FFFFFF"/>
  <polygon points="256,120 280,180 232,180" fill="#FF4400"/>
  <circle cx="256" cy="240" r="16" fill="#CCFF00"/>
  <!-- Technical Reticle Overlay -->
  <line x1="128" y1="256" x2="384" y2="256" stroke="#00E5FF" stroke-width="2" stroke-dasharray="8,8"/>
  <line x1="256" y1="128" x2="256" y2="384" stroke="#00E5FF" stroke-width="2" stroke-dasharray="8,8"/>
</svg>"""

    @classmethod
    def write_desktop_file(cls, path: Path):
        """Writes standard FreeDesktop .desktop entry file."""
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(cls.DESKTOP_ENTRY_TEMPLATE, encoding="utf-8")

    @classmethod
    def write_svg_icon(cls, path: Path):
        """Writes Brutalist scalable SVG icon."""
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(cls.SVG_ICON_TEMPLATE, encoding="utf-8")

    @classmethod
    def validate_desktop_file(cls, path: Path) -> Tuple[bool, List[str]]:
        """Validates FreeDesktop .desktop entry syntax and required fields."""
        errors: List[str] = []
        if not path.exists():
            return False, [f"File {path} does not exist"]

        config = configparser.ConfigParser(interpolation=None)
        try:
            config.read(str(path), encoding="utf-8")
        except Exception as e:
            return False, [f"Parse error: {e}"]

        if "Desktop Entry" not in config.sections():
            errors.append("Missing [Desktop Entry] section")
            return False, errors

        entry = config["Desktop Entry"]
        required_fields = ["Type", "Name", "Exec", "Icon", "Categories", "MimeType"]
        for rf in required_fields:
            if rf not in entry:
                errors.append(f"Missing required field: {rf}")

        return (len(errors) == 0), errors

    @classmethod
    def validate_svg_icon(cls, path: Path) -> Tuple[bool, List[str]]:
        """Validates SVG XML well-formedness and Brutalist design tokens."""
        errors: List[str] = []
        if not path.exists():
            return False, [f"File {path} does not exist"]

        try:
            content = path.read_text(encoding="utf-8")
            root = ET.fromstring(content)
            if not root.tag.endswith("svg"):
                errors.append(f"Root tag is not svg: {root.tag}")
        except Exception as e:
            return False, [f"XML parse error: {e}"]

        required_tokens = ["#070709", "#FF4400", "#CCFF00"]
        for token in required_tokens:
            if token not in content:
                errors.append(f"Missing Brutalist design token: {token}")

        return (len(errors) == 0), errors
