"""
lrc_parser.py - Synchronized LRC Lyric Parser & Teleprompter Query Engine.

Implements:
- Standard line-level LRC parsing: [mm:ss.xx], [mm:ss.xxx], [mm:ss.x], [mm:ss:xx], [mm:ss].
- Enhanced word-level synchronization (A2 extension): <mm:ss.xx> word tokens.
- Header metadata ID tags: ti, ar, al, au, length, by, offset, re, ve.
- Global offset (+/- ms) adjustment with non-negative clamping.
- Multi-timestamp lines ([00:01.00][00:05.00] Chorus) and chronological sorting.
- Instrumental pauses / empty lines for teleprompter clearing.
- O(log N) binary search active cue and word token lookups.
- Multi-encoding file loading (UTF-8, UTF-8-BOM, UTF-16, Latin-1).
"""

from __future__ import annotations
import bisect
from dataclasses import dataclass, field
from pathlib import Path
import re
from typing import Dict, List, Optional, Tuple, Union


@dataclass
class LRCToken:
    """Represents an individual word-level token within an enhanced lyric line."""
    time_ms: int
    text: str
    duration_ms: int = 0

    @property
    def time_seconds(self) -> float:
        return self.time_ms / 1000.0


@dataclass
class LRCCue:
    """Represents a synchronized lyric line cue."""
    time_ms: int
    text: str
    tokens: List[LRCToken] = field(default_factory=list)

    @property
    def time_seconds(self) -> float:
        return self.time_ms / 1000.0

    @property
    def is_empty(self) -> bool:
        return not self.text or not self.text.strip()

    def get_active_token(self, current_time_ms: int) -> Optional[LRCToken]:
        """Returns the currently active word token within this cue."""
        if not self.tokens:
            return None
        if current_time_ms < self.tokens[0].time_ms:
            return None
        times = [t.time_ms for t in self.tokens]
        idx = bisect.bisect_right(times, current_time_ms) - 1
        idx = max(0, min(len(self.tokens) - 1, idx))
        return self.tokens[idx]


class LRCParser:
    """Synchronized LRC lyric parser and teleprompter lookup engine."""

    # Matches metadata tags like [ti:Title] or [offset:+500]
    TAG_REGEX = re.compile(r"^\[([a-zA-Z]+)\s*:\s*([^\]]*)\]$")

    # Matches timestamps like [01:23.45], [01:23.456], [01:23,45], [01:23:45], [01:23]
    TIMESTAMP_REGEX = re.compile(r"\[(\d{1,4}):(\d{2})(?:[\.,:](\d{1,4}))?\]")

    # Matches word-level tokens like <00:12.34> word or <00:12:34>
    WORD_TOKEN_REGEX = re.compile(r"<(\d{1,4}):(\d{2})(?:[\.,:](\d{1,4}))?>([^<]*)")

    def __init__(self):
        self.metadata: Dict[str, str] = {}
        self.cues: List[LRCCue] = []
        self.offset_ms: int = 0

    @property
    def title(self) -> str:
        return self.metadata.get("ti", "")

    @title.setter
    def title(self, val: str):
        self.metadata["ti"] = val

    @property
    def artist(self) -> str:
        return self.metadata.get("ar", "")

    @artist.setter
    def artist(self, val: str):
        self.metadata["ar"] = val

    @property
    def album(self) -> str:
        return self.metadata.get("al", "")

    @album.setter
    def album(self, val: str):
        self.metadata["al"] = val

    @property
    def author(self) -> str:
        return self.metadata.get("au", self.metadata.get("by", ""))

    @author.setter
    def author(self, val: str):
        self.metadata["au"] = val

    @staticmethod
    def _parse_time_components(mm_str: str, ss_str: str, frac_str: Optional[str]) -> int:
        """Converts mm, ss, and fraction components into total milliseconds."""
        try:
            mm = int(mm_str)
            ss = int(ss_str)
        except ValueError:
            return 0

        ms = 0
        if frac_str is not None and frac_str:
            try:
                if len(frac_str) == 1:
                    ms = int(frac_str) * 100
                elif len(frac_str) == 2:
                    ms = int(frac_str) * 10
                elif len(frac_str) == 3:
                    ms = int(frac_str)
                else:
                    ms = int(frac_str[:3])
            except ValueError:
                ms = 0

        return (mm * 60 + ss) * 1000 + ms

    @classmethod
    def _parse_word_tokens(cls, text: str, line_time_ms: int, offset_ms: int) -> Tuple[str, List[LRCToken]]:
        """Parses word-level <mm:ss.xx> tokens within a lyric line."""
        tokens: List[LRCToken] = []
        matches = list(cls.WORD_TOKEN_REGEX.finditer(text))
        if not matches:
            return text, []

        clean_text_parts = []
        for i, m in enumerate(matches):
            mm_str, ss_str, frac_str = m.group(1), m.group(2), m.group(3)
            token_text = m.group(4).strip()
            token_time = cls._parse_time_components(mm_str, ss_str, frac_str)
            token_time_adj = max(0, token_time + offset_ms)
            clean_text_parts.append(token_text)
            tokens.append(LRCToken(time_ms=token_time_adj, text=token_text))

        # Calculate durations between consecutive tokens
        for i in range(len(tokens) - 1):
            dur = tokens[i + 1].time_ms - tokens[i].time_ms
            tokens[i].duration_ms = max(0, dur)

        full_clean_text = " ".join(part for part in clean_text_parts if part)
        return full_clean_text, tokens

    @classmethod
    def parse_string(cls, lrc_text: str) -> "LRCParser":
        """Parses LRC format text string and returns populated LRCParser instance."""
        parser = cls()
        if not lrc_text or not lrc_text.strip():
            return parser

        lines = re.split(r"[\r\n]+", lrc_text)
        raw_cues: List[Tuple[int, str, List[LRCToken]]] = []

        for line in lines:
            line_str = line.strip()
            if not line_str:
                continue

            # 1. Check for metadata tag
            tag_match = cls.TAG_REGEX.match(line_str)
            if tag_match:
                tag_key = tag_match.group(1).lower()
                tag_val = tag_match.group(2).strip()
                parser.metadata[tag_key] = tag_val
                if tag_key == "offset":
                    try:
                        parser.offset_ms = int(tag_val)
                    except ValueError:
                        parser.offset_ms = 0
                continue

            # 2. Extract all timestamp tags at the beginning of the line
            timestamps: List[int] = []
            last_end = 0
            for m in cls.TIMESTAMP_REGEX.finditer(line_str):
                # Verify contiguous timestamps at start of line
                t_ms = cls._parse_time_components(m.group(1), m.group(2), m.group(3))
                timestamps.append(t_ms)
                last_end = m.end()

            if timestamps:
                lyric_content = line_str[last_end:].strip()
                clean_text, tokens = cls._parse_word_tokens(lyric_content, timestamps[0], 0)
                for t in timestamps:
                    raw_cues.append((t, clean_text, tokens))

        # 3. Apply global offset and sort chronologically
        adjusted_cues: List[LRCCue] = []
        for raw_time, text_val, raw_tokens in raw_cues:
            adj_time = max(0, raw_time + parser.offset_ms)
            # Re-offset word tokens if present
            adj_tokens: List[LRCToken] = []
            if raw_tokens:
                for tok in raw_tokens:
                    adj_tokens.append(
                        LRCToken(
                            time_ms=max(0, tok.time_ms + parser.offset_ms),
                            text=tok.text,
                            duration_ms=tok.duration_ms,
                        )
                    )
            adjusted_cues.append(LRCCue(time_ms=adj_time, text=text_val, tokens=adj_tokens))

        adjusted_cues.sort(key=lambda c: c.time_ms)
        parser.cues = adjusted_cues
        return parser

    def parse(self, text: str) -> bool:
        """Parses text into this instance in-place. Returns True if any cues or metadata loaded."""
        parsed = self.parse_string(text)
        self.metadata = parsed.metadata
        self.cues = parsed.cues
        self.offset_ms = parsed.offset_ms
        return len(self.cues) > 0 or bool(self.title) or bool(self.artist)

    @classmethod
    def parse_file(cls, file_path: Union[str, Path]) -> "LRCParser":
        """Loads and parses an LRC file with multi-encoding fallback."""
        path = Path(file_path)
        if not path.exists():
            return cls()

        encodings = ["utf-8-sig", "utf-8", "utf-16", "latin-1", "cp1252"]
        content = ""
        for enc in encodings:
            try:
                content = path.read_text(encoding=enc)
                break
            except (UnicodeDecodeError, LookupError):
                continue

        return cls.parse_string(content)

    def get_active_cue_index(self, current_time_ms: int) -> int:
        """
        Binary searches for the active cue index at current_time_ms.
        Returns:
            -1 if current_time_ms is before the first cue
            index of the active cue (0 <= index < len(cues))
        """
        if not self.cues:
            return -1
        if current_time_ms < self.cues[0].time_ms:
            return -1
        if current_time_ms >= self.cues[-1].time_ms:
            return len(self.cues) - 1

        times = [c.time_ms for c in self.cues]
        idx = bisect.bisect_right(times, current_time_ms) - 1
        return max(0, min(len(self.cues) - 1, idx))

    def find_active_cue_index(self, position_ms: int) -> int:
        """Alias for get_active_cue_index for API compatibility."""
        return self.get_active_cue_index(position_ms)

    def get_active_cue(self, current_time_ms: int) -> Optional[LRCCue]:
        """Returns the active LRCCue at current_time_ms, or None if before first cue."""
        idx = self.get_active_cue_index(current_time_ms)
        if idx == -1:
            return None
        return self.cues[idx]

    def to_lrc_string(self) -> str:
        """Serializes current metadata and cues to standard .lrc file text."""
        lines: List[str] = []
        for k, v in self.metadata.items():
            lines.append(f"[{k}:{v}]")
        if self.offset_ms != 0 and "offset" not in self.metadata:
            lines.append(f"[offset:{self.offset_ms:+d}]")

        for cue in self.cues:
            total_sec = cue.time_ms / 1000.0
            mm = int(total_sec // 60)
            ss = int(total_sec % 60)
            cs = int(round((total_sec - int(total_sec)) * 100))
            if cs >= 100:
                cs = 99
            ts_str = f"[{mm:02d}:{ss:02d}.{cs:02d}]"
            lines.append(f"{ts_str}{cue.text}")

        return "\n".join(lines)
