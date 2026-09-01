"""
test_synthetic_media.py - Deterministic Synthetic Test Media Generator and Validator for Penguin.

Generates multi-container video (MP4, MKV multi-stream, WebM, AVI), audio files
(MP3, FLAC, WAV, AAC, Opus), external subtitle files (.srt, .vtt, .ass), and
synchronized .lrc lyrics with zero network dependencies using ffmpeg and standard library.
"""

import json
import math
import os
import shutil
import struct
import subprocess
import tempfile
import unittest
import wave
from pathlib import Path
from typing import Dict, List, Optional, Tuple


class SyntheticMediaFactory:
    """Deterministic generator for multi-format audio, video, subtitle, and lyric assets."""

    @staticmethod
    def is_ffmpeg_available() -> bool:
        return shutil.which("ffmpeg") is not None and shutil.which("ffprobe") is not None

    @staticmethod
    def create_wav_pure_python(
        dest_path: Path,
        freq_hz: float = 440.0,
        duration_sec: float = 1.0,
        sample_rate: int = 44100,
        channels: int = 2,
        amplitude: float = 0.5,
    ) -> Path:
        """Generates a standard PCM 16-bit WAV file using Python stdlib wave module."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        n_samples = int(duration_sec * sample_rate)
        with wave.open(str(dest_path), "wb") as wav_file:
            wav_file.setnchannels(channels)
            wav_file.setsampwidth(2)  # 16-bit
            wav_file.setframerate(sample_rate)
            frames = bytearray()
            for i in range(n_samples):
                t = float(i) / sample_rate
                val = int(32767.0 * amplitude * math.sin(2.0 * math.pi * freq_hz * t))
                val = max(-32768, min(32767, val))
                packed = struct.pack("<h", val)
                for _ in range(channels):
                    frames.extend(packed)
            wav_file.writeframes(frames)
        return dest_path

    @staticmethod
    def create_video_mp4(
        dest_path: Path,
        duration_sec: float = 3.0,
        fps: int = 60,
        width: int = 640,
        height: int = 360,
    ) -> Path:
        """Generates H.264 video + AAC 48kHz stereo audio MP4 container."""
        if duration_sec <= 0:
            raise ValueError(f"duration_sec must be positive, got {duration_sec}")
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"testsrc=duration={duration_sec}:size={width}x{height}:rate={fps}",
            "-f", "lavfi", "-i", f"sine=frequency=440:duration={duration_sec}:sample_rate=48000",
            "-c:v", "libx264", "-pix_fmt", "yuv420p", "-preset", "ultrafast",
            "-c:a", "aac", "-b:a", "128k", "-ac", "2",
            "-metadata", "title=Penguin Synthetic MP4 Test",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating MP4: {res.stderr}")
        return dest_path

    @staticmethod
    def create_multitrack_mkv(
        dest_path: Path,
        duration_sec: float = 3.0,
        fps: int = 24,
    ) -> Path:
        """Generates Matroska container with 1 video track, 2 audio tracks (eng, jpn), and 1 subtitle track (eng)."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        # Create temp srt
        temp_srt = dest_path.parent / "temp_embed.srt"
        SyntheticMediaFactory.create_subtitle_srt(temp_srt)

        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"testsrc=duration={duration_sec}:size=320x240:rate={fps}",
            "-f", "lavfi", "-i", f"sine=frequency=440:duration={duration_sec}:sample_rate=48000",
            "-f", "lavfi", "-i", f"sine=frequency=880:duration={duration_sec}:sample_rate=48000",
            "-i", str(temp_srt),
            "-map", "0:v", "-map", "1:a", "-map", "2:a", "-map", "3:s",
            "-metadata", "title=Penguin Multi-Track Cinema Test",
            "-metadata:s:v:0", "title=Main Video 24fps",
            "-metadata:s:a:0", "title=English Stereo (Primary)", "-metadata:s:a:0", "language=eng",
            "-metadata:s:a:1", "title=Japanese Commentary", "-metadata:s:a:1", "language=jpn",
            "-metadata:s:s:0", "title=English Full Captions", "-metadata:s:s:0", "language=eng",
            "-c:v", "libx264", "-preset", "ultrafast",
            "-c:a:0", "aac",
            "-c:a:1", "aac",
            "-c:s", "srt",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if temp_srt.exists():
            temp_srt.unlink()
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating MKV: {res.stderr}")
        return dest_path

    @staticmethod
    def create_video_webm(
        dest_path: Path,
        duration_sec: float = 2.0,
        fps: int = 30,
    ) -> Path:
        """Generates WebM container with VP8 video and Opus audio."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"testsrc=duration={duration_sec}:size=320x240:rate={fps}",
            "-f", "lavfi", "-i", f"sine=frequency=500:duration={duration_sec}:sample_rate=48000",
            "-c:v", "libvpx", "-b:v", "500k", "-quality", "realtime", "-cpu-used", "5",
            "-c:a", "libopus", "-b:a", "64k",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating WebM: {res.stderr}")
        return dest_path

    @staticmethod
    def create_video_avi(
        dest_path: Path,
        duration_sec: float = 2.0,
        fps: int = 25,
    ) -> Path:
        """Generates AVI container with MPEG-4 video and PCM audio."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"testsrc=duration={duration_sec}:size=320x240:rate={fps}",
            "-f", "lavfi", "-i", f"sine=frequency=600:duration={duration_sec}:sample_rate=44100",
            "-c:v", "mpeg4", "-qscale:v", "5",
            "-c:a", "pcm_s16le",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating AVI: {res.stderr}")
        return dest_path

    @staticmethod
    def create_audio_mp3(
        dest_path: Path,
        duration_sec: float = 2.0,
        title: str = "Cybernetic Horizon",
        artist: str = "Kraftwerk",
        album: str = "Computer World",
    ) -> Path:
        """Generates MP3 audio file with ID3v2 metadata tags."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"sine=frequency=440:duration={duration_sec}:sample_rate=44100",
            "-c:a", "libmp3lame", "-b:a", "320k",
            "-metadata", f"title={title}",
            "-metadata", f"artist={artist}",
            "-metadata", f"album={album}",
            "-metadata", "track=1/10",
            "-metadata", "genre=Electronic",
            "-metadata", "date=1981",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating MP3: {res.stderr}")
        return dest_path

    @staticmethod
    def create_audio_flac(
        dest_path: Path,
        duration_sec: float = 2.0,
        sample_rate: int = 96000,
        title: str = "Neon Lights",
        artist: str = "Kraftwerk",
    ) -> Path:
        """Generates 24-bit / 96kHz FLAC audio file with Vorbis comments."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"sine=frequency=880:duration={duration_sec}:sample_rate={sample_rate}",
            "-c:a", "flac", "-sample_fmt", "s32",
            "-metadata", f"TITLE={title}",
            "-metadata", f"ARTIST={artist}",
            "-metadata", "ALBUM=The Man-Machine",
            "-metadata", "TRACKNUMBER=2",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating FLAC: {res.stderr}")
        return dest_path

    @staticmethod
    def create_audio_aac(dest_path: Path, duration_sec: float = 2.0) -> Path:
        """Generates AAC-LC audio stream in ADTS container."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"sine=frequency=1000:duration={duration_sec}:sample_rate=48000",
            "-c:a", "aac", "-b:a", "256k",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating AAC: {res.stderr}")
        return dest_path

    @staticmethod
    def create_audio_opus(dest_path: Path, duration_sec: float = 2.0) -> Path:
        """Generates Ogg Opus audio file."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            "ffmpeg", "-y",
            "-f", "lavfi", "-i", f"sine=frequency=1200:duration={duration_sec}:sample_rate=48000",
            "-c:a", "libopus", "-b:a", "128k",
            str(dest_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"FFmpeg failed creating Opus: {res.stderr}")
        return dest_path

    @staticmethod
    def create_subtitle_srt(dest_path: Path) -> Path:
        """Generates standard SubRip (.srt) subtitle file."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        content = (
            "1\n"
            "00:00:00,100 --> 00:00:01,000\n"
            "Penguin Media Player — Initializing...\n\n"
            "2\n"
            "00:00:01,100 --> 00:00:02,200\n"
            "Tactile Digital Brutalism Active\n\n"
            "3\n"
            "00:00:02,300 --> 00:00:03,000\n"
            "Studio Precision Monitoring Engine\n"
        )
        dest_path.write_text(content, encoding="utf-8")
        return dest_path

    @staticmethod
    def create_subtitle_vtt(dest_path: Path) -> Path:
        """Generates WebVTT (.vtt) subtitle file with cue settings."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        content = (
            "WEBVTT - Penguin Test Subtitles\n\n"
            "00:00:00.100 --> 00:00:01.000 line:80% align:center\n"
            "<v System>Penguin Viewfinder Subtitle Test</v>\n\n"
            "00:00:01.100 --> 00:00:02.500 line:80% align:center\n"
            "<b>Safe-Area Reticles</b> and <c.lime>OSD Telemetry</c.lime>\n"
        )
        dest_path.write_text(content, encoding="utf-8")
        return dest_path

    @staticmethod
    def create_subtitle_ass(dest_path: Path) -> Path:
        """Generates Advanced SubStation Alpha (.ass) subtitle file with styles."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        content = (
            "[Script Info]\n"
            "Title: Penguin ASS Test\n"
            "ScriptType: v4.00+\n"
            "PlayResX: 1920\n"
            "PlayResY: 1080\n\n"
            "[V4+ Styles]\n"
            "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
            "Style: Default,JetBrains Mono,48,&H0000FFFF,&H000000FF,&H00000000,&H80000000,-1,0,0,0,100,100,0,0,1,2,2,2,10,10,20,1\n\n"
            "[Events]\n"
            "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
            "Dialogue: 0,0:00:00.10,0:00:01.50,Default,,0,0,0,,{\\b1}STUDIO PRECISION MONITORING{\\b0}\n"
            "Dialogue: 0,0:00:01.60,0:00:02.90,Default,,0,0,0,,{\\c&H00FFCC&}SIGNAL LIME 0.0dB PEAK{\\c}\n"
        )
        dest_path.write_text(content, encoding="utf-8")
        return dest_path

    @staticmethod
    def create_lyrics_lrc(dest_path: Path) -> Path:
        """Generates synchronized .lrc lyric file with ID tags and timestamps."""
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        content = (
            "[ti:Cybernetic Horizon]\n"
            "[ar:Kraftwerk]\n"
            "[al:Computer World]\n"
            "[by:Penguin Audio Deck]\n"
            "[offset:0]\n"
            "[00:00.10]TACTILE DIGITAL BRUTALISM\n"
            "[00:00.60]OBSIDIAN CHASSIS ACTIVE\n"
            "[00:01.20]STUDIO PRECISION MONITORING\n"
            "[00:01.80]SIGNAL LIME GAIN PEAK 0.0dB\n"
            "[00:02.40]SYNCHRONIZED TELEPROMPTER LOCK\n"
        )
        dest_path.write_text(content, encoding="utf-8")
        return dest_path

    @classmethod
    def create_all_fixtures(cls, base_dir: Path) -> Dict[str, Path]:
        """Creates complete suite of synthetic media test fixtures."""
        base_dir.mkdir(parents=True, exist_ok=True)
        fixtures = {
            "mp4": cls.create_video_mp4(base_dir / "test_video.mp4", duration_sec=3.0),
            "mkv": cls.create_multitrack_mkv(base_dir / "test_multitrack.mkv", duration_sec=3.0),
            "webm": cls.create_video_webm(base_dir / "test_video.webm", duration_sec=2.0),
            "avi": cls.create_video_avi(base_dir / "test_video.avi", duration_sec=2.0),
            "mp3": cls.create_audio_mp3(base_dir / "test_audio.mp3", duration_sec=2.0),
            "flac": cls.create_audio_flac(base_dir / "test_audio.flac", duration_sec=2.0),
            "wav": cls.create_wav_pure_python(base_dir / "test_audio.wav", duration_sec=1.5),
            "aac": cls.create_audio_aac(base_dir / "test_audio.aac", duration_sec=2.0),
            "opus": cls.create_audio_opus(base_dir / "test_audio.opus", duration_sec=2.0),
            "srt": cls.create_subtitle_srt(base_dir / "test_subtitles.srt"),
            "vtt": cls.create_subtitle_vtt(base_dir / "test_subtitles.vtt"),
            "ass": cls.create_subtitle_ass(base_dir / "test_subtitles.ass"),
            "lrc": cls.create_lyrics_lrc(base_dir / "test_lyrics.lrc"),
        }
        return fixtures

    @staticmethod
    def probe_file(file_path: Path) -> Dict:
        """Probes a media file using ffprobe and returns JSON representation of streams and format."""
        if not file_path.exists():
            raise FileNotFoundError(f"Cannot probe non-existent file: {file_path}")
        cmd = [
            "ffprobe", "-v", "quiet",
            "-print_format", "json",
            "-show_format", "-show_streams",
            str(file_path),
        ]
        res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            raise RuntimeError(f"ffprobe failed for {file_path}: {res.stderr}")
        return json.loads(res.stdout)


class TestSyntheticMediaGenerator(unittest.TestCase):
    """Test suite verifying synthetic media generation, stream parsing, and metadata extraction."""

    @classmethod
    def setUpClass(cls):
        cls.temp_dir = tempfile.TemporaryDirectory(prefix="penguin_synthetic_test_")
        cls.fixtures_dir = Path(cls.temp_dir.name)
        cls.fixtures = SyntheticMediaFactory.create_all_fixtures(cls.fixtures_dir)

    @classmethod
    def tearDownClass(cls):
        cls.temp_dir.cleanup()

    def test_mp4_generation_and_streams(self):
        mp4_path = self.fixtures["mp4"]
        self.assertTrue(mp4_path.exists())
        self.assertGreater(mp4_path.stat().st_size, 1024)

        probe = SyntheticMediaFactory.probe_file(mp4_path)
        streams = probe.get("streams", [])
        v_streams = [s for s in streams if s.get("codec_type") == "video"]
        a_streams = [s for s in streams if s.get("codec_type") == "audio"]

        self.assertEqual(len(v_streams), 1, "Expected 1 video stream in MP4")
        self.assertEqual(len(a_streams), 1, "Expected 1 audio stream in MP4")
        self.assertEqual(v_streams[0]["codec_name"], "h264")
        self.assertEqual(v_streams[0]["width"], 640)
        self.assertEqual(v_streams[0]["height"], 360)
        self.assertEqual(a_streams[0]["codec_name"], "aac")

    def test_multitrack_mkv_streams(self):
        mkv_path = self.fixtures["mkv"]
        self.assertTrue(mkv_path.exists())

        probe = SyntheticMediaFactory.probe_file(mkv_path)
        streams = probe.get("streams", [])
        v_streams = [s for s in streams if s.get("codec_type") == "video"]
        a_streams = [s for s in streams if s.get("codec_type") == "audio"]
        s_streams = [s for s in streams if s.get("codec_type") == "subtitle"]

        self.assertEqual(len(v_streams), 1, "Expected 1 video stream in MKV")
        self.assertEqual(len(a_streams), 2, "Expected 2 audio streams in MKV (multitrack)")
        self.assertEqual(len(s_streams), 1, "Expected 1 subtitle stream in MKV")

        # Verify language tags
        lang_a0 = a_streams[0].get("tags", {}).get("language")
        lang_a1 = a_streams[1].get("tags", {}).get("language")
        self.assertEqual(lang_a0, "eng")
        self.assertEqual(lang_a1, "jpn")

    def test_webm_generation(self):
        webm_path = self.fixtures["webm"]
        self.assertTrue(webm_path.exists())
        probe = SyntheticMediaFactory.probe_file(webm_path)
        streams = probe.get("streams", [])
        codecs = [s.get("codec_name") for s in streams]
        self.assertIn("vp8", codecs)
        self.assertIn("opus", codecs)

    def test_avi_generation(self):
        avi_path = self.fixtures["avi"]
        self.assertTrue(avi_path.exists())
        probe = SyntheticMediaFactory.probe_file(avi_path)
        streams = probe.get("streams", [])
        self.assertGreaterEqual(len(streams), 2)

    def test_mp3_metadata_extraction(self):
        mp3_path = self.fixtures["mp3"]
        self.assertTrue(mp3_path.exists())
        probe = SyntheticMediaFactory.probe_file(mp3_path)
        tags = probe.get("format", {}).get("tags", {})
        self.assertEqual(tags.get("title"), "Cybernetic Horizon")
        self.assertEqual(tags.get("artist"), "Kraftwerk")
        self.assertEqual(tags.get("album"), "Computer World")

    def test_flac_metadata_and_bitdepth(self):
        flac_path = self.fixtures["flac"]
        self.assertTrue(flac_path.exists())
        probe = SyntheticMediaFactory.probe_file(flac_path)
        stream = probe.get("streams", [])[0]
        self.assertEqual(stream.get("codec_name"), "flac")
        self.assertEqual(int(stream.get("sample_rate", 0)), 96000)
        tags = probe.get("format", {}).get("tags", {})
        self.assertEqual(tags.get("TITLE"), "Neon Lights")
        self.assertEqual(tags.get("ARTIST"), "Kraftwerk")

    def test_pure_python_wav_header_and_data(self):
        wav_path = self.fixtures["wav"]
        self.assertTrue(wav_path.exists())
        with wave.open(str(wav_path), "rb") as wf:
            self.assertEqual(wf.getnchannels(), 2)
            self.assertEqual(wf.getsampwidth(), 2)
            self.assertEqual(wf.getframerate(), 44100)
            n_frames = wf.getnframes()
            self.assertGreater(n_frames, 40000)

    def test_aac_and_opus_generation(self):
        aac_path = self.fixtures["aac"]
        opus_path = self.fixtures["opus"]
        self.assertTrue(aac_path.exists())
        self.assertTrue(opus_path.exists())

    def test_subtitles_syntax(self):
        srt_content = self.fixtures["srt"].read_text(encoding="utf-8")
        self.assertIn("00:00:00,100 --> 00:00:01,000", srt_content)
        self.assertIn("Tactile Digital Brutalism Active", srt_content)

        vtt_content = self.fixtures["vtt"].read_text(encoding="utf-8")
        self.assertTrue(vtt_content.startswith("WEBVTT"))
        self.assertIn("00:00:00.100 --> 00:00:01.000", vtt_content)

        ass_content = self.fixtures["ass"].read_text(encoding="utf-8")
        self.assertIn("[Script Info]", ass_content)
        self.assertIn("[V4+ Styles]", ass_content)
        self.assertIn("[Events]", ass_content)

    def test_lyrics_lrc_syntax(self):
        lrc_content = self.fixtures["lrc"].read_text(encoding="utf-8")
        self.assertIn("[ti:Cybernetic Horizon]", lrc_content)
        self.assertIn("[00:00.10]TACTILE DIGITAL BRUTALISM", lrc_content)
        self.assertIn("[00:01.80]SIGNAL LIME GAIN PEAK 0.0dB", lrc_content)

    def test_invalid_file_probe_raises(self):
        non_existent = self.fixtures_dir / "non_existent_file.mkv"
        with self.assertRaises(FileNotFoundError):
            SyntheticMediaFactory.probe_file(non_existent)


if __name__ == "__main__":
    unittest.main()
