#include <QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <clocale>

#include "BrutalistTheme.h"
#include "TickScrubberWidget.h"
#include "VUMeterWidget.h"
#include "EqualizerRackWidget.h"
#include "TeleprompterWidget.h"
#include "PlaylistMatrixWidget.h"
#include "DiagnosticsHUDWidget.h"
#include "ViewfinderWidget.h"
#include "AudioDeckWidget.h"
#include "MainWindow.h"

using namespace Penguin::UI;
using namespace Penguin::Core;

class TestM2UI : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 1. Theme & Design Tokens
    void testBrutalistThemeTokensAndFonts();

    // 2. Tick Scrubber Widget
    void testTickScrubberWidgetMathAndSignals();

    // 3. VU Meter Widget
    void testVUMeterWidgetLevelsAndPeakHold();

    // 4. Equalizer Rack Widget
    void testEqualizerRackWidgetBandsAndPresets();

    // 5. Teleprompter Widget
    void testTeleprompterWidgetSyncAndClickToSeek();

    // 6. Playlist Matrix Widget
    void testPlaylistMatrixWidgetQueueAndOperations();

    // 7. Diagnostics HUD Widget
    void testDiagnosticsHUDWidgetTelemetryDisplay();

    // 8. Viewfinder Widget
    void testViewfinderWidgetControlsAndReticles();

    // 9. Audio Deck Widget
    void testAudioDeckWidgetMastheadAndNavigation();

    // 10. MainWindow Mode Switching & Shortcuts
    void testMainWindowSeamlessModeSwitchingAndShortcuts();
};

void TestM2UI::initTestCase()
{
    setlocale(LC_NUMERIC, "C");
    qDebug() << "=== Starting Penguin M2 UI Subsystem Automated Test Suite ===";
}

void TestM2UI::cleanupTestCase()
{
    qDebug() << "=== Penguin M2 UI Subsystem Automated Test Suite Finished ===";
}

// ----------------------------------------------------------------------------
// 1. Theme & Design Tokens
// ----------------------------------------------------------------------------

void TestM2UI::testBrutalistThemeTokensAndFonts()
{
    // Verify exact Brutalist color palette tokens
    QCOMPARE(BrutalistTheme::BG_DEEP_OBSIDIAN.name().toUpper(), QString("#070709"));
    QCOMPARE(BrutalistTheme::SURFACE_PANEL_BASE.name().toUpper(), QString("#0B0B0E"));
    QCOMPARE(BrutalistTheme::SURFACE_RAISED.name().toUpper(), QString("#141418"));
    QCOMPARE(BrutalistTheme::GRID_STRUCTURAL_BORDER.name().toUpper(), QString("#1E1E24"));
    QCOMPARE(BrutalistTheme::GRID_LINE_ACTIVE.name().toUpper(), QString("#333342"));
    QCOMPARE(BrutalistTheme::TEXT_HIGH_CONTRAST.name().toUpper(), QString("#FFFFFF"));
    QCOMPARE(BrutalistTheme::TEXT_SECONDARY_DIM.name().toUpper(), QString("#777788"));
    QCOMPARE(BrutalistTheme::TEXT_MUTED.name().toUpper(), QString("#444455"));
    QCOMPARE(BrutalistTheme::ACCENT_SAFETY_ORANGE.name().toUpper(), QString("#FF4400"));
    QCOMPARE(BrutalistTheme::ACCENT_SIGNAL_LIME.name().toUpper(), QString("#CCFF00"));
    QCOMPARE(BrutalistTheme::ACCENT_TELEMETRY_CYAN.name().toUpper(), QString("#00E5FF"));
    QCOMPARE(BrutalistTheme::ACCENT_CLIP_RED.name().toUpper(), QString("#FF2200"));

    // Typography checks
    QFont mono = BrutalistTheme::monospaceFont(11, QFont::Bold);
    QCOMPARE(mono.pointSize(), 11);
    QCOMPARE(mono.weight(), QFont::Bold);
    QCOMPARE(mono.styleHint(), QFont::Monospace);

    QFont sans = BrutalistTheme::sansFont(12, QFont::Normal);
    QCOMPARE(sans.pointSize(), 12);
    QCOMPARE(sans.weight(), QFont::Normal);
    QCOMPARE(sans.styleHint(), QFont::SansSerif);

    // QSS inspection
    QString qss = BrutalistTheme::globalStyleSheet();
    QVERIFY(qss.contains("border-radius: 0px;"));
    QVERIFY(qss.contains("#070709"));
    QVERIFY(qss.contains("#FF4400"));
}

// ----------------------------------------------------------------------------
// 2. Tick Scrubber Widget
// ----------------------------------------------------------------------------

void TestM2UI::testTickScrubberWidgetMathAndSignals()
{
    TickScrubberWidget scrubber;
    scrubber.resize(800, 38);

    QSignalSpy spySeek(&scrubber, &TickScrubberWidget::seekRequested);
    QSignalSpy spyStart(&scrubber, &TickScrubberWidget::scrubbingStarted);
    QSignalSpy spyEnd(&scrubber, &TickScrubberWidget::scrubbingEnded);

    scrubber.setDurationMs(120000); // 2 minutes
    scrubber.setPositionMs(30000);  // 30 seconds
    scrubber.setFps(30.0);

    QCOMPARE(scrubber.durationMs(), 120000LL);
    QCOMPARE(scrubber.positionMs(), 30000LL);
    QCOMPARE(scrubber.fps(), 30.0);

    // Chapter markers
    scrubber.addChapter(0, "Intro");
    scrubber.addChapter(60000, "Drop");
    QCOMPARE(scrubber.chapters().size(), 2);
    QCOMPARE(scrubber.chapters()[1].title, QString("Drop"));
    QCOMPARE(scrubber.chapters()[1].timestampMs, 60000LL);

    // Mouse Interaction (Click in track center)
    // Track region is roughly [110, 800-110 = 690], center is ~400 -> fraction ~ 0.5 -> 60000ms
    QTest::mousePress(&scrubber, Qt::LeftButton, Qt::NoModifier, QPoint(400, 19));
    QCOMPARE(spyStart.count(), 1);
    QVERIFY(spySeek.count() >= 1);
    qint64 targetMs = spySeek.last().at(0).toLongLong();
    QVERIFY(std::abs(targetMs - 60000) <= 2000);

    // Mouse Drag
    QTest::mouseMove(&scrubber, QPoint(500, 19));
    QVERIFY(spySeek.count() >= 2);

    // Mouse Release
    QTest::mouseRelease(&scrubber, Qt::LeftButton, Qt::NoModifier, QPoint(500, 19));
    QCOMPARE(spyEnd.count(), 1);

    // Headless paint event test
    QPixmap pix(scrubber.size());
    scrubber.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 3. VU Meter Widget
// ----------------------------------------------------------------------------

void TestM2UI::testVUMeterWidgetLevelsAndPeakHold()
{
    VUMeterWidget vu;
    vu.resize(360, 72);

    // Set nominal audio level
    vu.setLevels(-12.0, -6.0, -15.0, -9.0);
    QCOMPARE(vu.leftPeakDb(), -12.0);
    QCOMPARE(vu.rightPeakDb(), -6.0);
    QVERIFY(!vu.isLeftClipped());
    QVERIFY(!vu.isRightClipped());

    // Set clipping level (> 0dB)
    vu.setLevels(1.5, -20.0);
    QCOMPARE(vu.leftPeakDb(), 1.5);
    QVERIFY(vu.isLeftClipped());
    QVERIFY(!vu.isRightClipped());

    // Reset
    vu.reset();
    QCOMPARE(vu.leftPeakDb(), VU_MIN_DB);
    QVERIFY(!vu.isLeftClipped());

    // Render offscreen
    QPixmap pix(vu.size());
    vu.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 4. Equalizer Rack Widget
// ----------------------------------------------------------------------------

void TestM2UI::testEqualizerRackWidgetBandsAndPresets()
{
    EqualizerRackWidget eq;
    eq.resize(600, 160);

    QSignalSpy spyBand(&eq, &EqualizerRackWidget::bandGainChanged);
    QSignalSpy spyPreset(&eq, &EqualizerRackWidget::presetChanged);
    QSignalSpy spyAll(&eq, &EqualizerRackWidget::allGainsChanged);

    // Set band 0 (32Hz) to +6.0 dB
    eq.setBandGain(0, 6.0);
    QCOMPARE(eq.bandGain(0), 6.0);
    QVERIFY(spyBand.count() >= 1);

    // Set Preset Rock
    eq.setPreset("Rock");
    QCOMPARE(eq.currentPreset(), QString("Rock"));
    QVERIFY(spyPreset.count() >= 1);
    QVERIFY(spyAll.count() >= 1);
    QCOMPARE(eq.bandGain(0), 4.5);
    QCOMPARE(eq.bandGain(9), 4.5);

    // Reset Flat
    eq.resetFlat();
    QCOMPARE(eq.currentPreset(), QString("Flat"));
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        QCOMPARE(eq.bandGain(i), 0.0);
    }

    // Render offscreen
    QPixmap pix(eq.size());
    eq.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 5. Teleprompter Widget
// ----------------------------------------------------------------------------

void TestM2UI::testTeleprompterWidgetSyncAndClickToSeek()
{
    TeleprompterWidget tp;
    tp.resize(400, 300);

    QString lrc =
        "[00:01.00] Line 1: Signal Start\n"
        "[00:03.00] Line 2: Monolith Active\n"
        "[00:05.00] Line 3: Telemetry Stream\n"
        "[00:08.00] Line 4: System Nominal\n";

    bool ok = tp.loadLrcContent(lrc);
    QVERIFY(ok);
    QCOMPARE(tp.cueCount(), 4);

    QSignalSpy spyActive(&tp, &TeleprompterWidget::activeCueChanged);
    QSignalSpy spySeek(&tp, &TeleprompterWidget::seekRequested);

    // Position before line 1
    tp.setPositionMs(500);
    QCOMPARE(tp.activeCueIndex(), -1);

    // Position at line 2 (3000ms - 4999ms)
    tp.setPositionMs(3500);
    QCOMPARE(tp.activeCueIndex(), 1);
    QVERIFY(spyActive.count() >= 1);

    // Position at line 4 (>= 8000ms)
    tp.setPositionMs(9000);
    QCOMPARE(tp.activeCueIndex(), 3);

    // Test Click to Seek
    // Simulate click on a cue row
    QTest::mousePress(&tp, Qt::LeftButton, Qt::NoModifier, QPoint(100, 50));
    // Click dispatched seekRequested signal
    QVERIFY(spySeek.count() >= 1);

    // Render offscreen
    QPixmap pix(tp.size());
    tp.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 6. Playlist Matrix Widget
// ----------------------------------------------------------------------------

void TestM2UI::testPlaylistMatrixWidgetQueueAndOperations()
{
    PlaylistMatrixWidget matrix;
    matrix.resize(500, 300);

    QSignalSpy spyChanged(&matrix, &PlaylistMatrixWidget::playlistChanged);
    QSignalSpy spyTrack(&matrix, &PlaylistMatrixWidget::trackDoubleClicked);

    // Add Items
    matrix.addItem("/path/track1.flac", "Track One", "Artist A", "Album X", 180000, "FLAC");
    matrix.addItem("/path/track2.mp3", "Track Two", "Artist B", "Album Y", 210000, "MP3");
    matrix.addItem("/path/video.mp4", "Video One", "Director C", "Film Z", 3600000, "MP4");

    QCOMPARE(matrix.count(), 3);
    QCOMPARE(spyChanged.count(), 3);

    PlaylistItem item1 = matrix.itemAt(0);
    QCOMPARE(item1.title, QString("Track One"));
    QCOMPARE(item1.format, QString("FLAC"));

    // Set current index
    matrix.setCurrentIndex(1);
    QCOMPARE(matrix.currentIndex(), 1);

    // Move Item
    matrix.moveItem(0, 2);
    QCOMPARE(matrix.itemAt(2).title, QString("Track One"));

    // Shuffle
    matrix.shuffle();
    QCOMPARE(matrix.count(), 3);

    // Remove Item
    matrix.removeItem(0);
    QCOMPARE(matrix.count(), 2);

    // Clear
    matrix.clearPlaylist();
    QCOMPARE(matrix.count(), 0);

    // Render offscreen
    QPixmap pix(matrix.size());
    matrix.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 7. Diagnostics HUD Widget
// ----------------------------------------------------------------------------

void TestM2UI::testDiagnosticsHUDWidgetTelemetryDisplay()
{
    DiagnosticsHUDWidget hud;
    hud.resize(380, 110);

    DiagnosticsData diag;
    diag.fps = 59.94;
    diag.nominalFps = 60.00;
    diag.droppedFrames = 2;
    diag.videoCodec = "hevc";
    diag.audioCodec = "flac";
    diag.videoBitrate = 8500000;
    diag.videoWidth = 3840;
    diag.videoHeight = 2160;
    diag.renderTimeMs = 1.85;
    diag.avSkewMs = 0.003;

    hud.updateDiagnostics(diag);
    QCOMPARE(hud.diagnostics().fps, 59.94);
    QCOMPARE(hud.diagnostics().droppedFrames, 2LL);
    QCOMPARE(hud.diagnostics().videoCodec, QString("hevc"));

    // Render offscreen
    QPixmap pix(hud.size());
    hud.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 8. Viewfinder Widget
// ----------------------------------------------------------------------------

void TestM2UI::testViewfinderWidgetControlsAndReticles()
{
    PlaybackEngine engine;
    engine.initialize(true);

    ViewfinderWidget vf(&engine);
    vf.resize(800, 600);

    QVERIFY(vf.isOsdVisible());
    QVERIFY(vf.isReticlesVisible());

    // Toggle Reticles & OSD
    vf.toggleReticles();
    QVERIFY(!vf.isReticlesVisible());
    vf.toggleReticles();
    QVERIFY(vf.isReticlesVisible());

    vf.toggleOsd();
    QVERIFY(!vf.isOsdVisible());
    vf.toggleOsd();
    QVERIFY(vf.isOsdVisible());

    // Render offscreen
    QPixmap pix(vf.size());
    vf.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 9. Audio Deck Widget
// ----------------------------------------------------------------------------

void TestM2UI::testAudioDeckWidgetMastheadAndNavigation()
{
    PlaybackEngine engine;
    engine.initialize(true);

    AudioDeckWidget deck(&engine);
    deck.resize(900, 600);

    // Add items to playlist matrix
    deck.playlistMatrix()->addItem("/path/a.flac", "Track A", "Artist A", "Album A", 120000, "FLAC");
    deck.playlistMatrix()->addItem("/path/b.flac", "Track B", "Artist B", "Album B", 180000, "FLAC");

    QCOMPARE(deck.playlistMatrix()->count(), 2);

    // Repeat mode cycle: Off -> RepeatAll -> RepeatOne -> Off
    QCOMPARE(deck.repeatMode(), RepeatMode::Off);
    deck.cycleRepeatMode();
    QCOMPARE(deck.repeatMode(), RepeatMode::RepeatAll);
    deck.cycleRepeatMode();
    QCOMPARE(deck.repeatMode(), RepeatMode::RepeatOne);
    deck.cycleRepeatMode();
    QCOMPARE(deck.repeatMode(), RepeatMode::Off);

    // Shuffle toggle
    QVERIFY(!deck.isShuffleEnabled());
    deck.toggleShuffle();
    QVERIFY(deck.isShuffleEnabled());
    deck.toggleShuffle();
    QVERIFY(!deck.isShuffleEnabled());

    // Render offscreen
    QPixmap pix(deck.size());
    deck.render(&pix);
    QVERIFY(!pix.isNull());
}

// ----------------------------------------------------------------------------
// 10. MainWindow Mode Switching & Shortcuts
// ----------------------------------------------------------------------------

void TestM2UI::testMainWindowSeamlessModeSwitchingAndShortcuts()
{
    MainWindow win;
    win.resize(1280, 720);

    // Default mode is Video Viewfinder
    QCOMPARE(win.currentMode(), UIMode::VideoViewfinder);

    // Switch to Audio Deck
    win.setMode(UIMode::HiFiAudioDeck);
    QCOMPARE(win.currentMode(), UIMode::HiFiAudioDeck);

    // Toggle Mode
    win.toggleMode();
    QCOMPARE(win.currentMode(), UIMode::VideoViewfinder);

    // Keyboard Shortcuts Simulation
    // Space (Play/Pause)
    QKeyEvent spaceEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(&win, &spaceEvent);

    // Comma & Period (Frame Stepping)
    QKeyEvent commaEvent(QEvent::KeyPress, Qt::Key_Comma, Qt::NoModifier);
    QApplication::sendEvent(&win, &commaEvent);
    QKeyEvent periodEvent(QEvent::KeyPress, Qt::Key_Period, Qt::NoModifier);
    QApplication::sendEvent(&win, &periodEvent);

    // Tab (Mode toggle)
    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QApplication::sendEvent(&win, &tabEvent);
    QCOMPARE(win.currentMode(), UIMode::HiFiAudioDeck);

    // Left / Right seeking
    QKeyEvent leftEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(&win, &leftEvent);
    QKeyEvent rightEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(&win, &rightEvent);

    // Up / Down volume
    QKeyEvent upEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(&win, &upEvent);
    QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QApplication::sendEvent(&win, &downEvent);

    // Mute toggle
    QKeyEvent muteEvent(QEvent::KeyPress, Qt::Key_M, Qt::NoModifier);
    QApplication::sendEvent(&win, &muteEvent);

    // Speed controls ([ / ] / Backspace)
    QKeyEvent speedDown(QEvent::KeyPress, Qt::Key_BracketLeft, Qt::NoModifier);
    QApplication::sendEvent(&win, &speedDown);
    QKeyEvent speedUp(QEvent::KeyPress, Qt::Key_BracketRight, Qt::NoModifier);
    QApplication::sendEvent(&win, &speedUp);
    QKeyEvent speedReset(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(&win, &speedReset);
    QCOMPARE(win.playbackEngine()->speed(), 1.0);

    // Open synthetic test audio fixture and check mode heuristic
    QString testAudio = "tests/fixtures/test_audio.flac";
    if (QFileInfo::exists(testAudio)) {
        win.openMedia(testAudio, false);
        // Audio file auto-selects HiFiAudioDeck
        QCOMPARE(win.currentMode(), UIMode::HiFiAudioDeck);
    }

    // Open synthetic test video fixture and check mode heuristic
    QString testVideo = "tests/fixtures/test_video.mp4";
    if (QFileInfo::exists(testVideo)) {
        win.openMedia(testVideo, false);
        // Video file auto-selects VideoViewfinder
        QCOMPARE(win.currentMode(), UIMode::VideoViewfinder);
    }

    // Render entire MainWindow offscreen
    QPixmap pix(win.size());
    win.render(&pix);
    QVERIFY(!pix.isNull());
}

QTEST_MAIN(TestM2UI)
#include "test_m2_ui.moc"
