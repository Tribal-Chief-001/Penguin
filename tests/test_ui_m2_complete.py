"""
test_ui_m2_complete.py - Exhaustive Automated Test Suite for Penguin M2 GUI Subsystem.

Tests:
1. BrutalistTheme & Color Palette Tokens & Typography
2. MechanicalTickScrubber & SMPTE Math & Chapter Markers
3. StereoVUMeterRack 30-Segment Ballistics & Clip Alerts
4. EqualizerRackWidget 10-Band Sliders & Presets
5. SynchronizedLyricsTeleprompter Sync & Click-to-Seek
6. PlaylistMatrixWidget Queue Matrix & Search Filtering
7. ViewfinderWidget & SafeAreaReticles & DiagnosticsHUDWidget
8. AudioDeckWidget Masthead & Subsystem Integration
9. MainWindow Mode Switching & Hotkey Routing
"""

import unittest
from unittest.mock import MagicMock

from src.ui import (
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
    MONOSPACE_FONT_FAMILY,
    SANS_FONT_FAMILY,
    BrutalistTheme,
    ChapterMarker,
    MechanicalTickScrubber,
    VU_MIN_DB,
    VU_MAX_DB,
    VU_HEADROOM_DB,
    VU_CLIP_DB,
    VU_NUM_SEGMENTS,
    VU_CALIBRATION_MARKS,
    StereoVUMeterRack,
    EQ_NUM_BANDS,
    EQ_PRESETS,
    EqualizerRackWidget,
    SynchronizedLyricsTeleprompter,
    PlaylistItem,
    PLAYLIST_COLUMNS,
    PlaylistMatrixWidget,
    DiagnosticsData,
    DiagnosticsHUDWidget,
    SafeAreaReticles,
    ViewfinderWidget,
    RepeatMode,
    AudioMasthead,
    AudioDeckWidget,
    UIMode,
    MainWindow,
)


class TestBrutalistTheme(unittest.TestCase):
    """Verifies Brutalist color tokens, typography stacks, and QSS rules."""

    def test_color_tokens(self):
        self.assertEqual(BG_DEEP_OBSIDIAN, "#070709")
        self.assertEqual(SURFACE_PANEL_BASE, "#0B0B0E")
        self.assertEqual(SURFACE_RAISED, "#141418")
        self.assertEqual(GRID_STRUCTURAL_BORDER, "#1E1E24")
        self.assertEqual(GRID_LINE_ACTIVE, "#333342")
        self.assertEqual(TEXT_HIGH_CONTRAST, "#FFFFFF")
        self.assertEqual(TEXT_SECONDARY_DIM, "#777788")
        self.assertEqual(TEXT_MUTED, "#444455")
        self.assertEqual(ACCENT_SAFETY_ORANGE, "#FF4400")
        self.assertEqual(ACCENT_SIGNAL_LIME, "#CCFF00")
        self.assertEqual(ACCENT_TELEMETRY_CYAN, "#00E5FF")
        self.assertEqual(ACCENT_CLIP_RED, "#FF2200")

    def test_theme_dict(self):
        tokens = BrutalistTheme.get_color_tokens()
        self.assertEqual(len(tokens), 12)
        self.assertIn("ACCENT_SAFETY_ORANGE", tokens)
        self.assertEqual(tokens["ACCENT_SAFETY_ORANGE"], "#FF4400")

    def test_stylesheets(self):
        global_qss = BrutalistTheme.global_stylesheet()
        self.assertIn("border-radius: 0px;", global_qss)
        self.assertIn("#070709", global_qss)
        self.assertIn("#FF4400", global_qss)

        self.assertIn("#FF4400", BrutalistTheme.accent_orange_button_stylesheet())
        self.assertIn("#CCFF00", BrutalistTheme.accent_lime_button_stylesheet())
        self.assertIn("#0B0B0E", BrutalistTheme.panel_stylesheet())
        self.assertIn("#070709", BrutalistTheme.table_stylesheet())


class TestMechanicalTickScrubber(unittest.TestCase):
    """Verifies tick scrubber SMPTE readouts, chapters, coordinates, and scrubbing."""

    def test_scrubber_initialization_and_clamping(self):
        s = MechanicalTickScrubber(duration_ms=120000, position_ms=30000, fps=30.0)
        self.assertEqual(s.duration_ms, 120000)
        self.assertEqual(s.position_ms, 30000)
        self.assertEqual(s.fps, 30.0)

        # Clamping
        s.position_ms = 999999
        self.assertEqual(s.position_ms, 120000)
        s.position_ms = -500
        self.assertEqual(s.position_ms, 0)

    def test_smpte_readouts(self):
        s = MechanicalTickScrubber(duration_ms=100000, position_ms=10000, fps=30.0)
        self.assertEqual(s.elapsed_smpte(), "00:00:10:00")
        self.assertEqual(s.remaining_smpte(), "-00:01:30:00")

    def test_chapter_markers(self):
        s = MechanicalTickScrubber(duration_ms=120000)
        s.add_chapter(60000, "Drop")
        s.add_chapter(0, "Intro")
        s.add_chapter(90000, "Outro")

        self.assertEqual(len(s.chapters), 3)
        # Should be sorted
        self.assertEqual(s.chapters[0].title, "Intro")
        self.assertEqual(s.chapters[1].title, "Drop")
        self.assertEqual(s.chapters[2].title, "Outro")

        s.clear_chapters()
        self.assertEqual(len(s.chapters), 0)

    def test_coordinates_and_ticks(self):
        s = MechanicalTickScrubber(duration_ms=100000, width=800)
        track_w = s.track_width
        self.assertEqual(track_w, 800 - 220)  # 580px

        # Center X
        center_x = s.track_left + track_w // 2
        pos = s.x_to_position(center_x)
        self.assertAlmostEqual(pos, 50000, delta=500)

        x_calc = s.position_to_x(50000)
        self.assertAlmostEqual(x_calc, center_x, delta=2)

        ticks = s.graduation_ticks()
        self.assertGreater(len(ticks), 10)
        # Verify major ticks
        major_ticks = [t for t in ticks if t[2]]
        self.assertGreater(len(major_ticks), 2)
        self.assertEqual(major_ticks[0][1], 12)

    def test_interactive_scrubbing_and_callbacks(self):
        s = MechanicalTickScrubber(duration_ms=100000, width=800)
        seek_log = []
        start_called = []
        end_called = []

        s.set_on_seek(lambda pos: seek_log.append(pos))
        s.set_on_scrub_start(lambda: start_called.append(True))
        s.set_on_scrub_end(lambda: end_called.append(True))

        # Mouse Press in middle
        mid_x = s.track_left + s.track_width // 2
        s.handle_mouse_press(mid_x, 19)
        self.assertEqual(len(start_called), 1)
        self.assertEqual(len(seek_log), 1)
        self.assertTrue(s.is_scrubbing)

        # Mouse Drag
        s.handle_mouse_move(mid_x + 50, 19)
        self.assertEqual(len(seek_log), 2)

        # Mouse Release
        s.handle_mouse_release(mid_x + 50, 19)
        self.assertEqual(len(end_called), 1)
        self.assertFalse(s.is_scrubbing)


class TestStereoVUMeterRack(unittest.TestCase):
    """Verifies VU meter 30-segment architecture, dB ranges, ballistics, and clip alerts."""

    def test_scale_constants(self):
        self.assertEqual(VU_MIN_DB, -60.0)
        self.assertEqual(VU_MAX_DB, 3.0)
        self.assertEqual(VU_HEADROOM_DB, -3.0)
        self.assertEqual(VU_CLIP_DB, 0.0)
        self.assertEqual(VU_NUM_SEGMENTS, 30)

    def test_zone_and_segment_quantization(self):
        vu = StereoVUMeterRack()
        # -60 dB -> 0 segments
        self.assertEqual(vu.db_to_segment_count(-60.0), 0)
        # +3 dB -> 30 segments
        self.assertEqual(vu.db_to_segment_count(+3.0), 30)
        # 0 dB -> ~28-29 segments
        seg_0db = vu.db_to_segment_count(0.0)
        self.assertGreaterEqual(seg_0db, 28)

        # Segment color
        self.assertEqual(vu.segment_color(0), ACCENT_SIGNAL_LIME)
        self.assertEqual(vu.segment_color(29), ACCENT_CLIP_RED)

    def test_level_updates_and_peak_hold(self):
        vu = StereoVUMeterRack()
        vu.set_levels(-12.0, -6.0)
        self.assertAlmostEqual(vu.left_peak_db, -12.0)
        self.assertAlmostEqual(vu.right_peak_db, -6.0)
        self.assertAlmostEqual(vu.left_peak_hold_db, -12.0)
        self.assertAlmostEqual(vu.right_peak_hold_db, -6.0)
        self.assertFalse(vu.is_clipped)

        # Higher spike updates peak hold
        vu.set_levels(-3.0, 1.5)
        self.assertAlmostEqual(vu.left_peak_hold_db, -3.0)
        self.assertAlmostEqual(vu.right_peak_hold_db, 1.5)
        self.assertTrue(vu.is_right_clipped)
        self.assertTrue(vu.is_clipped)

    def test_ballistic_decay(self):
        vu = StereoVUMeterRack(decay_rate_db_per_sec=20.0, peak_hold_decay_rate=10.0)
        vu.set_levels(0.0, 0.0)
        vu.update_ballistics(dt=0.5)  # 0.5 sec -> -10dB peak, -5dB hold
        self.assertAlmostEqual(vu.left_peak_db, -10.0, delta=0.5)
        self.assertAlmostEqual(vu.left_peak_hold_db, -5.0, delta=0.5)

    def test_reset(self):
        vu = StereoVUMeterRack()
        vu.set_levels(2.0, 2.0)
        self.assertTrue(vu.is_clipped)
        vu.reset()
        self.assertEqual(vu.left_peak_db, VU_MIN_DB)
        self.assertEqual(vu.left_peak_hold_db, VU_MIN_DB)
        self.assertFalse(vu.is_clipped)


class TestEqualizerRackWidget(unittest.TestCase):
    """Verifies 10-band equalizer sliders, presets, and frequency mapping."""

    def test_iso_frequencies(self):
        eq = EqualizerRackWidget()
        self.assertEqual(eq.num_bands, 10)
        self.assertEqual(eq.iso_frequencies(), [32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000])

    def test_gain_limits_and_clamping(self):
        eq = EqualizerRackWidget()
        eq.set_band_gain(0, 6.0)
        self.assertEqual(eq.band_gain(0), 6.0)

        # Clamping
        eq.set_band_gain(1, 20.0)
        self.assertEqual(eq.band_gain(1), 12.0)
        eq.set_band_gain(2, -30.0)
        self.assertEqual(eq.band_gain(2), -12.0)

    def test_presets_and_reset_flat(self):
        eq = EqualizerRackWidget()
        ok = eq.set_preset("Rock")
        self.assertTrue(ok)
        self.assertEqual(eq.current_preset, "Rock")
        self.assertEqual(eq.band_gain(0), 4.5)
        self.assertEqual(eq.band_gain(9), 4.5)

        # Modifying a band makes preset Custom
        eq.set_band_gain(0, 0.0)
        self.assertEqual(eq.current_preset, "Custom")

        # Reset Flat
        eq.reset_flat()
        self.assertEqual(eq.current_preset, "Flat")
        for i in range(10):
            self.assertEqual(eq.band_gain(i), 0.0)

    def test_labels_and_colors(self):
        eq = EqualizerRackWidget()
        self.assertEqual(eq.band_label(0), "32")
        self.assertEqual(eq.band_label(5), "1k")
        self.assertEqual(eq.band_label(9), "16k")

        eq.set_band_gain(5, 3.0)
        self.assertEqual(eq.gain_color(5), ACCENT_SIGNAL_LIME)
        eq.set_band_gain(6, -4.0)
        self.assertEqual(eq.gain_color(6), ACCENT_SAFETY_ORANGE)


class TestSynchronizedLyricsTeleprompter(unittest.TestCase):
    """Verifies synchronized LRC parsing, active cue tracking, and click-to-seek."""

    def test_load_and_cues(self):
        tp = SynchronizedLyricsTeleprompter()
        lrc = (
            "[ti:Cyberpunk Suite]\n"
            "[ar:Penguin Labs]\n"
            "[00:01.00] Line 1: Into the grid\n"
            "[00:03.50] Line 2: Monolith online\n"
            "[00:06.00] Line 3: Waveform aligned\n"
        )
        ok = tp.load_lrc_content(lrc)
        self.assertTrue(ok)
        self.assertEqual(tp.cue_count, 3)
        self.assertEqual(tp.metadata.get("ti"), "Cyberpunk Suite")

    def test_position_synchronization(self):
        tp = SynchronizedLyricsTeleprompter()
        lrc = "[00:01.00] L1\n[00:04.00] L2\n[00:08.00] L3\n"
        tp.load_lrc_content(lrc)

        # Before first cue
        tp.set_position_ms(500)
        self.assertEqual(tp.active_cue_index, -1)

        # On cue 1
        tp.set_position_ms(2000)
        self.assertEqual(tp.active_cue_index, 0)
        self.assertEqual(tp.active_cue.text, "L1")

        # On cue 2
        tp.set_position_ms(5000)
        self.assertEqual(tp.active_cue_index, 1)

        # On cue 3
        tp.set_position_ms(10000)
        self.assertEqual(tp.active_cue_index, 2)

    def test_rendered_lines_and_click_to_seek(self):
        tp = SynchronizedLyricsTeleprompter()
        lrc = "[00:02.00] Line A\n[00:05.00] Line B\n"
        tp.load_lrc_content(lrc)
        tp.set_position_ms(2500)

        lines = tp.get_rendered_lines()
        self.assertEqual(len(lines), 2)
        self.assertEqual(lines[0]["state"], "active")
        self.assertEqual(lines[0]["color"], ACCENT_SIGNAL_LIME)
        self.assertEqual(lines[1]["state"], "upcoming")

        seek_log = []
        tp.set_on_seek_requested(lambda ms: seek_log.append(ms))

        # Click line 1 (Line B -> 5000ms)
        target = tp.handle_click_line(1)
        self.assertEqual(target, 5000)
        self.assertEqual(seek_log, [5000])


class TestPlaylistMatrixWidget(unittest.TestCase):
    """Verifies 6-column matrix layout, search filtering, reordering, and shuffle."""

    def test_schema_and_insertion(self):
        matrix = PlaylistMatrixWidget()
        self.assertEqual(matrix.columns(), ["#", "TITLE", "ARTIST", "ALBUM", "DUR", "FORMAT"])

        matrix.add_item("/path/a.flac", "Track A", "Artist X", "Album 1", 180000, "FLAC")
        matrix.add_item("/path/b.mp3", "Track B", "Artist Y", "Album 2", 240000, "MP3")
        self.assertEqual(matrix.count, 2)

        item0 = matrix.item_at(0)
        self.assertEqual(item0.title, "Track A")
        self.assertEqual(item0.formatted_duration, "03:00")
        self.assertEqual(item0.format, "FLAC")

    def test_reorder_and_remove(self):
        matrix = PlaylistMatrixWidget()
        matrix.add_item("/1", "T1")
        matrix.add_item("/2", "T2")
        matrix.add_item("/3", "T3")

        # Move T1 to index 2
        matrix.move_item(0, 2)
        self.assertEqual(matrix.item_at(0).title, "T2")
        self.assertEqual(matrix.item_at(1).title, "T3")
        self.assertEqual(matrix.item_at(2).title, "T1")

        # Remove T3
        removed = matrix.remove_item(1)
        self.assertEqual(removed.title, "T3")
        self.assertEqual(matrix.count, 2)

    def test_search_filtering(self):
        matrix = PlaylistMatrixWidget()
        matrix.add_item("/1", "Cyberpunk Horizon", "Alpha", "OST", 100000, "FLAC")
        matrix.add_item("/2", "Acoustic Melody", "Beta", "Acoustic", 120000, "WAV")
        matrix.add_item("/3", "Cyberpunk Neon", "Gamma", "OST", 140000, "MP3")

        matrix.set_search_filter("cyberpunk")
        results = matrix.filtered_items()
        self.assertEqual(len(results), 2)
        self.assertEqual(results[0][1].title, "Cyberpunk Horizon")
        self.assertEqual(results[1][1].title, "Cyberpunk Neon")

        matrix.set_search_filter("")
        self.assertEqual(len(matrix.filtered_items()), 3)

    def test_shuffle(self):
        matrix = PlaylistMatrixWidget()
        for i in range(10):
            matrix.add_item(f"/{i}", f"Track {i}")
        matrix.shuffle(seed=42)
        self.assertEqual(matrix.count, 10)
        titles = [item.title for item in matrix.items]
        self.assertEqual(set(titles), {f"Track {i}" for i in range(10)})


class TestViewfinderAndReticles(unittest.TestCase):
    """Verifies reticle geometry bounds, diagnostics HUD formatting, and dock controls."""

    def test_reticles_bounds_math(self):
        reticles = SafeAreaReticles(action_safe_ratio=0.90, title_safe_ratio=0.80)
        bounds = reticles.calculate_bounds(1920, 1080)

        action = bounds["action_safe"]
        self.assertAlmostEqual(action["width"], 1920 * 0.90)
        self.assertAlmostEqual(action["height"], 1080 * 0.90)
        self.assertAlmostEqual(action["x"], (1920 - 1920 * 0.90) / 2)

        title = bounds["title_safe"]
        self.assertAlmostEqual(title["width"], 1920 * 0.80)
        self.assertAlmostEqual(title["height"], 1080 * 0.80)

        cross = bounds["center_crosshairs"]
        self.assertEqual(cross["cx"], 960)
        self.assertEqual(cross["cy"], 540)
        self.assertEqual(cross["arm_length"], 16)

    def test_diagnostics_hud_formatting(self):
        hud = DiagnosticsHUDWidget()
        diag = DiagnosticsData(
            fps=59.94,
            nominal_fps=60.00,
            dropped_frames=0,
            video_codec="hevc",
            audio_codec="flac",
            video_bitrate=4820000,
            video_width=3840,
            video_height=2160,
            render_time_ms=1.42,
            av_skew_ms=0.002,
        )
        hud.update_diagnostics(diag)
        rows = hud.format_rows()
        self.assertEqual(len(rows), 5)
        self.assertIn("TELEMETRY OSD", rows[0])
        self.assertIn("59.94", rows[1])
        self.assertIn("HEVC", rows[2])
        self.assertIn("3840x2160", rows[2])
        self.assertIn("FLAC", rows[3])
        self.assertIn("1.42ms", rows[4])

    def test_viewfinder_toggles(self):
        vf = ViewfinderWidget()
        self.assertTrue(vf.is_osd_visible())
        self.assertTrue(vf.is_reticles_visible())

        vf.toggle_osd()
        self.assertFalse(vf.is_osd_visible())
        vf.toggle_reticles()
        self.assertFalse(vf.is_reticles_visible())

        # Night mode / deband / aspect ratio
        self.assertTrue(vf.toggle_night_mode())
        self.assertTrue(vf.toggle_deband())
        ratio = vf.cycle_aspect_ratio()
        self.assertIn(ratio, ["16:9", "2.39:1", "4:3", "1:1", "21:9", "auto"])


class TestAudioDeckWidget(unittest.TestCase):
    """Verifies Audio Deck container and metadata masthead."""

    def test_masthead_formatting(self):
        mast = AudioMasthead(
            title="Ghost in the Shell",
            artist="Kenji Kawai",
            album="OST",
            sample_rate_hz=96000,
            bit_depth=24,
            bitrate_kbps=2450,
            channels="STEREO",
        )
        badge = mast.technical_badge()
        self.assertIn("24-BIT", badge)
        self.assertIn("96.0 kHz", badge)
        self.assertIn("2,450 kbps", badge)
        self.assertIn("STEREO", badge)

    def test_audio_deck_repeat_mode_cycle(self):
        deck = AudioDeckWidget()
        self.assertEqual(deck.repeat_mode, RepeatMode.Off)
        self.assertEqual(deck.cycle_repeat_mode(), RepeatMode.RepeatAll)
        self.assertEqual(deck.cycle_repeat_mode(), RepeatMode.RepeatOne)
        self.assertEqual(deck.cycle_repeat_mode(), RepeatMode.Off)

    def test_audio_deck_shuffle(self):
        deck = AudioDeckWidget()
        self.assertFalse(deck.is_shuffle_enabled())
        self.assertTrue(deck.toggle_shuffle())
        self.assertFalse(deck.toggle_shuffle())


class TestMainWindowOrchestration(unittest.TestCase):
    """Verifies MainWindow mode switching, heuristics, and hotkey handling."""

    def test_mode_switching(self):
        win = MainWindow()
        self.assertEqual(win.current_mode, UIMode.VideoViewfinder)

        win.set_mode(UIMode.HiFiAudioDeck)
        self.assertEqual(win.current_mode, UIMode.HiFiAudioDeck)

        win.toggle_mode()
        self.assertEqual(win.current_mode, UIMode.VideoViewfinder)

    def test_open_media_heuristics(self):
        win = MainWindow()
        # Audio file forces Hi-Fi Audio Deck
        win.open_media("tests/fixtures/test_audio.flac", auto_play=False)
        self.assertEqual(win.current_mode, UIMode.HiFiAudioDeck)

        # Video file forces Video Viewfinder
        win.open_media("tests/fixtures/test_video.mp4", auto_play=False)
        self.assertEqual(win.current_mode, UIMode.VideoViewfinder)

    def test_keyboard_hotkeys(self):
        win = MainWindow()
        # Tab -> mode toggle
        handled = win.handle_key_press("TAB")
        self.assertTrue(handled)
        self.assertEqual(win.current_mode, UIMode.HiFiAudioDeck)

        # Space -> play/pause
        self.assertTrue(win.handle_key_press("SPACE"))

        # Left / Right seek
        self.assertTrue(win.handle_key_press("LEFT"))
        self.assertTrue(win.handle_key_press("RIGHT"))

        # Up / Down volume
        self.assertTrue(win.handle_key_press("UP"))
        self.assertTrue(win.handle_key_press("DOWN"))

        # Mute
        self.assertTrue(win.handle_key_press("M"))

        # Frame step
        self.assertTrue(win.handle_key_press("COMMA"))
        self.assertTrue(win.handle_key_press("PERIOD"))

        # Speed
        self.assertTrue(win.handle_key_press("BRACKETRIGHT"))
        self.assertTrue(win.handle_key_press("BRACKETLEFT"))
        self.assertTrue(win.handle_key_press("BACKSPACE"))

        # Fullscreen
        self.assertTrue(win.handle_key_press("F"))
        self.assertTrue(win.is_fullscreen)
        self.assertTrue(win.handle_key_press("F"))
        self.assertFalse(win.is_fullscreen)

        # Reticles & OSD
        self.assertTrue(win.handle_key_press("R"))
        self.assertTrue(win.handle_key_press("O"))

        # DSP filters
        self.assertTrue(win.handle_key_press("N"))
        self.assertTrue(win.handle_key_press("D"))
        self.assertTrue(win.handle_key_press("C"))

        # Unhandled
        self.assertFalse(win.handle_key_press("UNKNOWN_KEY_XYZ"))


if __name__ == "__main__":
    unittest.main()
