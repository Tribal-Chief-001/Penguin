"""
conftest.py - Shared Test Fixtures, Harness Utilities & Synthetic Media Generators for Penguin.

Provides:
- Synthetic audio & video media generator fixtures (MP4, MKV multi-track, WebM, AVI, MP3, FLAC, WAV, AAC, Opus)
- Subtitle & lyrics fixtures (.srt, .vtt, .ass, .lrc with centisecond & word tokens)
- Temporary isolated SQLite WAL database fixtures
- Isolated D-Bus session harness fixtures (dbus-run-session)
- MPRIS2 mock service and protocol verification fixtures
- Pytest and unittest interoperability
"""

import os
import shutil
import tempfile
from pathlib import Path
from typing import Dict, Generator, List, Optional

# Project root setup
PROJECT_ROOT = Path(__file__).resolve().parent.parent

from tests.test_synthetic_media import SyntheticMediaFactory
from tests.test_timecode import SMPTETimecode
from tests.test_equalizer_dsp import BiquadPeakingEQ, VUMeterDSP
from tests.test_lrc_parser import LRCParser, LRCCue
from tests.test_mpris2_dbus import MPRIS2PlayerMockService, TrackMetadata
from tests.test_persistence import DatabaseManager
from tests.test_cli_desktop import PenguinCLIParser, DesktopPackager

# Pytest fixture support (conditional import if pytest is installed)
try:
    import pytest
    HAVE_PYTEST = True
except ImportError:
    HAVE_PYTEST = False


def get_fixtures_dir() -> Path:
    """Returns the path to the tests/fixtures directory, ensuring it exists."""
    fixtures_dir = PROJECT_ROOT / "tests" / "fixtures"
    fixtures_dir.mkdir(parents=True, exist_ok=True)
    return fixtures_dir


def ensure_all_synthetic_fixtures() -> Path:
    """Ensures all deterministic synthetic audio/video/sub/lyric fixtures exist."""
    fixtures_dir = get_fixtures_dir()
    SyntheticMediaFactory.create_all_fixtures(fixtures_dir)
    return fixtures_dir


def create_temp_database() -> DatabaseManager:
    """Creates a temporary DatabaseManager backed by an isolated temporary file."""
    temp_dir = tempfile.mkdtemp(prefix="penguin_test_db_")
    db_path = Path(temp_dir) / "test_penguin.db"
    return DatabaseManager(db_path)


def create_mpris2_mock() -> MPRIS2PlayerMockService:
    """Creates a fresh in-memory MPRIS2 player mock service."""
    return MPRIS2PlayerMockService()


def create_cli_parser():
    """Creates the standard Penguin CLI argument parser."""
    return PenguinCLIParser.build_parser()


# Register pytest fixtures if pytest environment is present
if HAVE_PYTEST:
    @pytest.fixture(scope="session")
    def fixtures_directory() -> Path:
        return ensure_all_synthetic_fixtures()

    @pytest.fixture
    def db_manager() -> Generator[DatabaseManager, None, None]:
        manager = create_temp_database()
        yield manager
        manager.close()
        shutil.rmtree(manager.db_path.parent, ignore_errors=True)

    @pytest.fixture
    def mpris2_service() -> MPRIS2PlayerMockService:
        return create_mpris2_mock()

    @pytest.fixture
    def cli_parser():
        return create_cli_parser()
