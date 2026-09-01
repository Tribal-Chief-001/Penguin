"""
test_lrc_parser.py - Synchronized .LRC Lyric Parser and Teleprompter Query Test Suite.

Tests parsing of standard LRC timestamps ([mm:ss.xx], [mm:ss.xxx]), metadata ID tags,
multiple timestamps per line, chronological sorting, offset adjustments, binary search
active line lookup, click-to-seek timestamp extraction, and malformed cue recovery.
"""

import bisect
from dataclasses import dataclass
import re
import unittest
from typing import Dict, List, Optional, Tuple


@dataclass
class LRCCue:
    time_ms: int
    text: str

    def time_seconds(self) -> float:
        return self.time_ms / 1000.0


class LRCParser:
    """Synchronized LRC lyric parser and teleprompter lookup engine."""

    # Matches metadata tags like [ti:Title]
    TAG_REGEX = re.compile(r"^\[([a-zA-Z]+):([^\]]*)\]$")
    # Matches timestamp tags like [01:23.45] or [01:23.456]
    TIMESTAMP_REGEX = re.compile(r"\[(\d{1,3}):(\d{2})(?:\.(\d{1,3}))?\]")

    def __init__(self):
        self.metadata: Dict[str, str] = {}
        self.cues: List[LRCCue] = []
        self.offset_ms: int = 0

    @classmethod
    def parse_string(cls, lrc_text: str) -> "LRCParser":
        parser = cls()
        lines = lrc_text.splitlines()

        raw_cues: List[LRCCue] = []

        for line in lines:
            line_str = line.strip()
            if not line_str:
                continue

            # Check for metadata tags
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

            # Find all timestamps at the beginning of the line
            timestamps = []
            last_end = 0
            for m in cls.TIMESTAMP_REGEX.finditer(line_str):
                mm = int(m.group(1))
                ss = int(m.group(2))
                ms_str = m.group(3)
                if ms_str is None:
                    ms = 0
                elif len(ms_str) == 1:
                    ms = int(ms_str) * 100
                elif len(ms_str) == 2:
                    ms = int(ms_str) * 10
                else:
                    ms = int(ms_str[:3])
                total_ms = (mm * 60 * 1000) + (ss * 1000) + ms
                timestamps.append(total_ms)
                last_end = m.end()

            if timestamps:
                cue_text = line_str[last_end:].strip()
                for t in timestamps:
                    raw_cues.append(LRCCue(time_ms=t, text=cue_text))

        # Apply global offset and sort chronologically
        adjusted_cues = []
        for cue in raw_cues:
            adj_time = max(0, cue.time_ms + parser.offset_ms)
            adjusted_cues.append(LRCCue(time_ms=adj_time, text=cue.text))

        adjusted_cues.sort(key=lambda c: c.time_ms)
        parser.cues = adjusted_cues
        return parser

    def get_active_cue_index(self, current_time_ms: int) -> int:
        """
        Binary search for active cue index at current_time_ms.
        Returns:
            -1 if current_time_ms is before the first cue
            index of the active cue (0 <= index < len(cues))
        """
        if not self.cues:
            return -1
        if current_time_ms < self.cues[0].time_ms:
            return -1

        # Use bisect_right on time_ms
        times = [c.time_ms for c in self.cues]
        idx = bisect.bisect_right(times, current_time_ms) - 1
        return max(0, min(len(self.cues) - 1, idx))

    def get_active_cue(self, current_time_ms: int) -> Optional[LRCCue]:
        idx = self.get_active_cue_index(current_time_ms)
        if idx == -1:
            return None
        return self.cues[idx]


class TestLRCParser(unittest.TestCase):
    """Comprehensive test suite for LRC parsing and teleprompter tracking."""

    def test_standard_two_decimal_timestamps(self):
        sample = """
        [ti:Test Title]
        [ar:Test Artist]
        [00:01.00]First line
        [00:03.50]Second line
        [01:10.25]Third line
        """
        parser = LRCParser.parse_string(sample)
        self.assertEqual(parser.metadata.get("ti"), "Test Title")
        self.assertEqual(parser.metadata.get("ar"), "Test Artist")
        self.assertEqual(len(parser.cues), 3)

        self.assertEqual(parser.cues[0].time_ms, 1000)
        self.assertEqual(parser.cues[0].text, "First line")

        self.assertEqual(parser.cues[1].time_ms, 3500)
        self.assertEqual(parser.cues[1].text, "Second line")

        self.assertEqual(parser.cues[2].time_ms, 70250)
        self.assertEqual(parser.cues[2].text, "Third line")

    def test_three_decimal_timestamps(self):
        sample = """
        [00:00.123]Line A
        [00:02.456]Line B
        """
        parser = LRCParser.parse_string(sample)
        self.assertEqual(len(parser.cues), 2)
        self.assertEqual(parser.cues[0].time_ms, 123)
        self.assertEqual(parser.cues[1].time_ms, 2456)

    def test_multiple_timestamps_per_line(self):
        sample = "[00:02.00][00:06.00][00:10.00]Chorus repeating phrase"
        parser = LRCParser.parse_string(sample)
        self.assertEqual(len(parser.cues), 3)
        self.assertEqual(parser.cues[0].time_ms, 2000)
        self.assertEqual(parser.cues[1].time_ms, 6000)
        self.assertEqual(parser.cues[2].time_ms, 10000)
        for cue in parser.cues:
            self.assertEqual(cue.text, "Chorus repeating phrase")

    def test_out_of_order_cues_chronological_sort(self):
        sample = """
        [00:15.00]Line 3
        [00:05.00]Line 1
        [00:10.00]Line 2
        """
        parser = LRCParser.parse_string(sample)
        self.assertEqual(len(parser.cues), 3)
        self.assertEqual(parser.cues[0].text, "Line 1")
        self.assertEqual(parser.cues[1].text, "Line 2")
        self.assertEqual(parser.cues[2].text, "Line 3")

    def test_offset_metadata_adjustment(self):
        sample_pos_offset = """
        [offset:+500]
        [00:01.00]Adjusted line
        """
        p1 = LRCParser.parse_string(sample_pos_offset)
        self.assertEqual(p1.cues[0].time_ms, 1500)

        sample_neg_offset = """
        [offset:-300]
        [00:01.00]Adjusted line
        """
        p2 = LRCParser.parse_string(sample_neg_offset)
        self.assertEqual(p2.cues[0].time_ms, 700)

    def test_active_cue_query_states(self):
        sample = """
        [00:02.00]Cue 1 (2.0s)
        [00:05.00]Cue 2 (5.0s)
        [00:08.00]Cue 3 (8.0s)
        """
        parser = LRCParser.parse_string(sample)

        # 1. Before first cue (< 2000ms)
        self.assertEqual(parser.get_active_cue_index(0), -1)
        self.assertIsNone(parser.get_active_cue(1500))

        # 2. Exactly on first cue (2000ms)
        self.assertEqual(parser.get_active_cue_index(2000), 0)
        self.assertEqual(parser.get_active_cue(2000).text, "Cue 1 (2.0s)")

        # 3. Between Cue 1 and Cue 2 (3500ms)
        self.assertEqual(parser.get_active_cue_index(3500), 0)
        self.assertEqual(parser.get_active_cue(3500).text, "Cue 1 (2.0s)")

        # 4. Exactly on Cue 2 (5000ms)
        self.assertEqual(parser.get_active_cue_index(5000), 1)

        # 5. After last cue (> 8000ms)
        self.assertEqual(parser.get_active_cue_index(8000), 2)
        self.assertEqual(parser.get_active_cue_index(12000), 2)
        self.assertEqual(parser.get_active_cue(12000).text, "Cue 3 (8.0s)")

    def test_empty_and_corrupt_files_resilience(self):
        p_empty = LRCParser.parse_string("")
        self.assertEqual(len(p_empty.cues), 0)
        self.assertEqual(p_empty.get_active_cue_index(1000), -1)

        p_garbage = LRCParser.parse_string("This is not LRC\n[invalid]\n[99:99:99] Bad tag")
        self.assertEqual(len(p_garbage.cues), 0)

    def test_unicode_and_special_characters(self):
        sample = """
        [00:01.00] Kraftwerk — ♫ Synthèse Vocale ♫
        [00:02.00] 触覚的ブルータリズム (Tactile Brutalism)
        [00:03.00] ⚡ <c.lime>Active Reticle</c.lime> & Gain 0.0dB ⚡
        """
        parser = LRCParser.parse_string(sample)
        self.assertEqual(len(parser.cues), 3)
        self.assertIn("Synthèse Vocale", parser.cues[0].text)
        self.assertIn("触覚的ブルータリズム", parser.cues[1].text)
        self.assertIn("⚡", parser.cues[2].text)


if __name__ == "__main__":
    unittest.main()
