#include <QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <clocale>

#include "DatabaseManager.h"
#include "StatePersistence.h"
#include "PlaylistManager.h"
#include "CommandLineParser.h"
#include "MPRIS2Adaptor.h"
#include "DBusService.h"
#include "PlaybackEngine.h"
#include "MainWindow.h"

using namespace Penguin::Library;
using namespace Penguin::Desktop;
using namespace Penguin::Core;
using namespace Penguin::UI;

class TestM3M4 : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 1. DatabaseManager & SQLite WAL Tests
    void testDatabaseInitializationAndWalMode();
    void testAppSettingsCrud();
    void testMediaHistoryUpsertAndResume();
    void testPlaylistItemsCrudAndReordering();
    void testEqualizerPresetsCrudAndFactorySeeding();

    // 2. StatePersistence Tests
    void testStatePersistenceWindowState();
    void testStatePersistenceAudioSettings();
    void testStatePersistenceUIModeAndTelemetry();
    void testStatePersistencePlaylistQueue();

    // 3. PlaylistManager Tests
    void testPlaylistManagerFileAndDirectoryLoading();
    void testPlaylistManagerQueueNavigationAndLoopModes();
    void testPlaylistManagerShuffleOrder();
    void testPlaylistManagerItemReorderingAndRemoval();

    // 4. CommandLineParser Tests
    void testCommandLineParserPositionalAndModeFlags();
    void testCommandLineParserAudioParameters();
    void testCommandLineParserIpcActions();
    void testCommandLineParserDiagnosticsAndInfo();

    // 5. MPRIS2 D-Bus Adaptors & Service Tests
    void testMPRIS2RootInterfaceProperties();
    void testMPRIS2PlayerPlaybackStateTransitions();
    void testMPRIS2PlayerVolumeAndRateClamping();
    void testMPRIS2PlayerMicrosecondSeeking();
    void testMPRIS2PlayerMetadataDictionaryFormatting();
    void testMPRIS2DBusServiceRegistrationAndFallback();

    // 6. Packaging & Assets Tests
    void testDesktopEntryFileSpecCompliance();
    void testSvgIconValidXmlAndBranding();
    void testPngIconFilesExist();
};

void TestM3M4::initTestCase()
{
    setlocale(LC_NUMERIC, "C");
    qDebug() << "=== Starting Penguin M3 & M4 Subsystem Automated Test Suite ===";
}

void TestM3M4::cleanupTestCase()
{
    qDebug() << "=== Penguin M3 & M4 Subsystem Automated Test Suite Finished ===";
}

// ----------------------------------------------------------------------------
// 1. DatabaseManager & SQLite WAL Tests
// ----------------------------------------------------------------------------

void TestM3M4::testDatabaseInitializationAndWalMode()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString dbPath = tempDir.path() + "/test_wal.db";

    DatabaseManager db;
    QVERIFY(db.initialize(dbPath));
    QVERIFY(db.isOpen());
    QCOMPARE(db.databasePath(), dbPath);

    // Verify WAL mode via direct query
    {
        QSqlDatabase sqlDb = QSqlDatabase::database(QString("penguin_db_conn_%1").arg(reinterpret_cast<quintptr>(&db)));
        QSqlQuery q(sqlDb);
        QVERIFY(q.exec("PRAGMA journal_mode;"));
        QVERIFY(q.next());
        QString journalMode = q.value(0).toString().toLower();
        QCOMPARE(journalMode, QString("wal"));
    }

    db.close();
    QVERIFY(!db.isOpen());
}

void TestM3M4::testAppSettingsCrud()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_settings.db"));

    // Primitive int/double/string
    QVERIFY(db.setSetting("volume", 85));
    QCOMPARE(db.getSetting("volume").toInt(), 85);

    QVERIFY(db.setSetting("speed", 1.25));
    QCOMPARE(db.getSetting("speed").toDouble(), 1.25);

    QVERIFY(db.setSetting("theme", "tactile_dark"));
    QCOMPARE(db.getSetting("theme").toString(), QString("tactile_dark"));

    // Map object
    QVariantMap geom;
    geom["x"] = 150;
    geom["y"] = 200;
    geom["width"] = 1920;
    geom["height"] = 1080;
    QVERIFY(db.setSetting("geometry", geom));

    QVariantMap restoredGeom = db.getSetting("geometry").toMap();
    QCOMPARE(restoredGeom["width"].toInt(), 1920);
    QCOMPARE(restoredGeom["height"].toInt(), 1080);

    // Default value for non-existent key
    QCOMPARE(db.getSetting("non_existent", "default_val").toString(), QString("default_val"));

    // Remove setting
    QVERIFY(db.removeSetting("volume"));
    QCOMPARE(db.getSetting("volume", 50).toInt(), 50);
}

void TestM3M4::testMediaHistoryUpsertAndResume()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_history.db"));

    QString uri = "file:///music/cybernetic_horizon.flac";

    // 1st play
    QVERIFY(db.recordPlayback(uri, "Cybernetic Horizon", "Kraftwerk", "Computer World", 240000, 45000, "audio", 45000000));
    QList<HistoryItem> history = db.getRecentHistory(10);
    QCOMPARE(history.size(), 1);
    QCOMPARE(history[0].title, QString("Cybernetic Horizon"));
    QCOMPARE(history[0].lastPositionMs, 45000LL);
    QCOMPARE(history[0].playCount, 1);

    // 2nd play (upsert with updated position)
    QVERIFY(db.recordPlayback(uri, "Cybernetic Horizon", "Kraftwerk", "Computer World", 240000, 120000, "audio", 45000000));
    history = db.getRecentHistory(10);
    QCOMPARE(history.size(), 1);
    QCOMPARE(history[0].lastPositionMs, 120000LL);
    QCOMPARE(history[0].playCount, 2);

    // Single item query
    HistoryItem single = db.getHistoryItem(uri);
    QCOMPARE(single.artist, QString("Kraftwerk"));
    QCOMPARE(single.durationMs, 240000LL);

    // Remove item & clear
    QVERIFY(db.removeHistoryItem(uri));
    QCOMPARE(db.getRecentHistory(10).size(), 0);
}

void TestM3M4::testPlaylistItemsCrudAndReordering()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_playlist.db"));

    QList<PlaylistItemRecord> items;
    for (int i = 0; i < 3; ++i) {
        PlaylistItemRecord it;
        it.playlistName = "default";
        it.positionOrder = i;
        it.uri = QString("file:///media/track_%1.mp3").arg(i + 1);
        it.title = QString("Track %1").arg(i + 1);
        it.artist = "Synth Lab";
        it.album = "Matrix";
        it.durationMs = 180000 + i * 10000;
        it.format = "MP3";
        items.append(it);
    }

    QVERIFY(db.savePlaylistItems("default", items));
    QList<PlaylistItemRecord> loaded = db.getPlaylistItems("default");
    QCOMPARE(loaded.size(), 3);
    QCOMPARE(loaded[0].title, QString("Track 1"));
    QCOMPARE(loaded[2].title, QString("Track 3"));

    // Reorder: swap 0 and 2
    std::swap(items[0], items[2]);
    QVERIFY(db.savePlaylistItems("default", items));
    loaded = db.getPlaylistItems("default");
    QCOMPARE(loaded[0].title, QString("Track 3"));
    QCOMPARE(loaded[2].title, QString("Track 1"));

    QVERIFY(db.clearPlaylist("default"));
    QCOMPARE(db.getPlaylistItems("default").size(), 0);
}

void TestM3M4::testEqualizerPresetsCrudAndFactorySeeding()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_eq.db"));

    // Verify factory presets seeded
    auto allPresets = db.getAllEqualizerPresets();
    QVERIFY(allPresets.size() >= 8);

    EqualizerPresetRecord flat = db.getEqualizerPreset("Flat");
    QCOMPARE(flat.name, QString("Flat"));
    QCOMPARE(flat.gains.size(), 10);
    QCOMPARE(flat.gains[0], 0.0);
    QVERIFY(flat.isBuiltin);

    EqualizerPresetRecord rock = db.getEqualizerPreset("Rock / Metal");
    QCOMPARE(rock.name, QString("Rock / Metal"));
    QCOMPARE(rock.gains[0], 4.5);

    // Save custom user preset
    QVector<double> customGains = {+3.0, +2.0, +1.0, 0.0, -1.0, -2.0, +1.0, +2.0, +3.0, +4.0};
    QVERIFY(db.saveEqualizerPreset("Custom Warmth", customGains, -1.0, false));

    EqualizerPresetRecord custom = db.getEqualizerPreset("Custom Warmth");
    QCOMPARE(custom.name, QString("Custom Warmth"));
    QCOMPARE(custom.gains, customGains);
    QCOMPARE(custom.preamp, -1.0);
    QVERIFY(!custom.isBuiltin);

    // Delete custom preset
    QVERIFY(db.deleteEqualizerPreset("Custom Warmth"));
    QVERIFY(db.getEqualizerPreset("Custom Warmth").name.isEmpty());

    // Deleting built-in factory preset should be protected
    db.deleteEqualizerPreset("Flat");
    QVERIFY(!db.getEqualizerPreset("Flat").name.isEmpty());
}

// ----------------------------------------------------------------------------
// 2. StatePersistence Tests
// ----------------------------------------------------------------------------

void TestM3M4::testStatePersistenceWindowState()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_state.db"));

    StatePersistence persistence(&db);
    QWidget dummyWindow;
    dummyWindow.setGeometry(120, 140, 1024, 768);

    persistence.saveWindowState(&dummyWindow);

    QWidget restoreWindow;
    persistence.restoreWindowState(&restoreWindow);

    QCOMPARE(restoreWindow.width(), 1024);
    QCOMPARE(restoreWindow.height(), 768);
}

void TestM3M4::testStatePersistenceAudioSettings()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_state_audio.db"));

    StatePersistence persistence(&db);

    QVector<double> testGains = {4.5, 3.0, 1.5, 0.0, -1.0, -0.5, 1.5, 3.0, 4.0, 4.5};
    persistence.saveAudioSettings(78, true, 1.5, "Rock", testGains);

    int vol = 0;
    bool muted = false;
    double speed = 1.0;
    QString preset;
    QVector<double> restoredGains;

    persistence.restoreAudioSettings(vol, muted, speed, preset, restoredGains);

    QCOMPARE(vol, 78);
    QCOMPARE(muted, true);
    QCOMPARE(speed, 1.5);
    QCOMPARE(preset, QString("Rock"));
    QCOMPARE(restoredGains, testGains);
}

void TestM3M4::testStatePersistenceUIModeAndTelemetry()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_state_ui.db"));

    StatePersistence persistence(&db);

    persistence.saveUIMode(1); // HiFiAudioDeck
    QCOMPARE(persistence.restoreUIMode(0), 1);

    persistence.saveTelemetryPreferences(false, true);
    bool osd = true, reticles = false;
    persistence.restoreTelemetryPreferences(osd, reticles);
    QCOMPARE(osd, false);
    QCOMPARE(reticles, true);
}

void TestM3M4::testStatePersistencePlaylistQueue()
{
    QTemporaryDir tempDir;
    DatabaseManager db;
    QVERIFY(db.initialize(tempDir.path() + "/test_state_queue.db"));

    StatePersistence persistence(&db);

    QList<PlaylistItem> queue;
    for (int i = 0; i < 2; ++i) {
        PlaylistItem it;
        it.filePath = QString("/music/song_%1.flac").arg(i);
        it.title = QString("Song %1").arg(i);
        it.format = "FLAC";
        queue.append(it);
    }

    persistence.saveCurrentPlaylist(queue, 1);

    int restoredIndex = -1;
    QList<PlaylistItem> restoredQueue = persistence.restoreCurrentPlaylist(restoredIndex);

    QCOMPARE(restoredQueue.size(), 2);
    QCOMPARE(restoredIndex, 1);
    QCOMPARE(restoredQueue[0].title, QString("Song 0"));
}

// ----------------------------------------------------------------------------
// 3. PlaylistManager Tests
// ----------------------------------------------------------------------------

void TestM3M4::testPlaylistManagerFileAndDirectoryLoading()
{
    PlaylistManager mgr;
    QSignalSpy spyUpdated(&mgr, &PlaylistManager::playlistUpdated);

    QVERIFY(PlaylistManager::isSupportedMedia("test.mp4"));
    QVERIFY(PlaylistManager::isSupportedMedia("test.flac"));
    QVERIFY(!PlaylistManager::isSupportedMedia("test.txt"));

    QString testAudio = "tests/fixtures/test_audio.flac";
    if (QFileInfo::exists(testAudio)) {
        mgr.addFile(testAudio);
        QCOMPARE(mgr.count(), 1);
        QVERIFY(spyUpdated.count() >= 1);
        QCOMPARE(mgr.itemAt(0).format, QString("FLAC"));
    }

    QString fixturesDir = "tests/fixtures";
    if (QDir(fixturesDir).exists()) {
        mgr.clear();
        mgr.addDirectory(fixturesDir, false);
        QVERIFY(mgr.count() > 0);
    }
}

void TestM3M4::testPlaylistManagerQueueNavigationAndLoopModes()
{
    PlaylistManager mgr;
    QList<PlaylistItem> items;
    for (int i = 0; i < 3; ++i) {
        PlaylistItem it;
        it.filePath = QString("/path/%1.mp3").arg(i);
        it.title = QString("Track %1").arg(i);
        items.append(it);
    }
    mgr.setItems(items, 0);

    QSignalSpy spyTrack(&mgr, &PlaylistManager::currentTrackChanged);
    QSignalSpy spyEnded(&mgr, &PlaylistManager::queueEnded);

    // Loop None: Advance 0 -> 1 -> 2 -> End
    QCOMPARE(mgr.currentIndex(), 0);
    QVERIFY(mgr.hasNext());
    QVERIFY(!mgr.hasPrevious());

    QVERIFY(mgr.next());
    QCOMPARE(mgr.currentIndex(), 1);

    QVERIFY(mgr.next());
    QCOMPARE(mgr.currentIndex(), 2);
    QVERIFY(!mgr.hasNext());

    QVERIFY(!mgr.next());
    QCOMPARE(spyEnded.count(), 1);

    // Previous navigation
    QVERIFY(mgr.previous());
    QCOMPARE(mgr.currentIndex(), 1);

    // Loop Playlist: wraps around
    mgr.setLoopMode(LoopMode::Playlist);
    mgr.setCurrentIndex(2);
    QVERIFY(mgr.hasNext());
    QVERIFY(mgr.next());
    QCOMPARE(mgr.currentIndex(), 0);

    // Loop Track: stays on track
    mgr.setLoopMode(LoopMode::Track);
    QVERIFY(mgr.next());
    QCOMPARE(mgr.currentIndex(), 0);
}

void TestM3M4::testPlaylistManagerShuffleOrder()
{
    PlaylistManager mgr;
    QList<PlaylistItem> items;
    for (int i = 0; i < 20; ++i) {
        PlaylistItem it;
        it.filePath = QString("/path/%1.mp3").arg(i);
        items.append(it);
    }
    mgr.setItems(items, 0);

    QSignalSpy spyShuffle(&mgr, &PlaylistManager::shuffleChanged);
    QVERIFY(!mgr.isShuffleEnabled());

    mgr.setShuffle(true);
    QVERIFY(mgr.isShuffleEnabled());
    QCOMPARE(spyShuffle.count(), 1);

    // Next should advance through shuffle sequence
    int first = mgr.currentIndex();
    QVERIFY(mgr.next());
    int second = mgr.currentIndex();
    QVERIFY(first != second);

    mgr.setShuffle(false);
    QVERIFY(!mgr.isShuffleEnabled());
}

void TestM3M4::testPlaylistManagerItemReorderingAndRemoval()
{
    PlaylistManager mgr;
    QList<PlaylistItem> items;
    for (int i = 0; i < 3; ++i) {
        PlaylistItem it;
        it.filePath = QString("/path/%1.mp3").arg(i);
        it.title = QString("T%1").arg(i);
        items.append(it);
    }
    mgr.setItems(items, 0);

    // Move item 0 to 2
    mgr.moveItem(0, 2);
    QCOMPARE(mgr.itemAt(2).title, QString("T0"));
    QCOMPARE(mgr.itemAt(0).title, QString("T1"));

    // Remove item 1
    mgr.removeItem(1);
    QCOMPARE(mgr.count(), 2);
    QCOMPARE(mgr.itemAt(1).title, QString("T0"));
}

// ----------------------------------------------------------------------------
// 4. CommandLineParser Tests
// ----------------------------------------------------------------------------

void TestM3M4::testCommandLineParserPositionalAndModeFlags()
{
    CommandLineParser parser;

    QStringList args = {"penguin", "video.mp4", "audio.flac", "-v", "-f"};
    CommandLineOptions opt = parser.parse(args);

    QCOMPARE(opt.files, QStringList({"video.mp4", "audio.flac"}));
    QCOMPARE(opt.forceVideo, true);
    QCOMPARE(opt.forceAudio, false);
    QCOMPARE(opt.fullscreen, true);

    QStringList argsAudio = {"penguin", "--audio"};
    CommandLineOptions optAudio = parser.parse(argsAudio);
    QCOMPARE(optAudio.forceAudio, true);
    QCOMPARE(optAudio.forceVideo, false);
}

void TestM3M4::testCommandLineParserAudioParameters()
{
    CommandLineParser parser;

    QStringList args = {"penguin", "--volume", "75", "--speed", "1.5", "--eq", "Rock", "--sub", "subs.srt", "--loop", "playlist", "--shuffle"};
    CommandLineOptions opt = parser.parse(args);

    QCOMPARE(opt.volume, 75);
    QCOMPARE(opt.speed, 1.5);
    QCOMPARE(opt.eqPreset, QString("Rock"));
    QCOMPARE(opt.subtitlePath, QString("subs.srt"));
    QCOMPARE(opt.loopMode, QString("playlist"));
    QCOMPARE(opt.shuffle, true);
}

void TestM3M4::testCommandLineParserIpcActions()
{
    CommandLineParser parser;

    QCOMPARE(parser.parse({"penguin", "--toggle-pause"}).ipcAction, QString("play-pause"));
    QCOMPARE(parser.parse({"penguin", "--next"}).ipcAction, QString("next"));
    QCOMPARE(parser.parse({"penguin", "--prev"}).ipcAction, QString("previous"));
    QCOMPARE(parser.parse({"penguin", "--stop"}).ipcAction, QString("stop"));
    QCOMPARE(parser.parse({"penguin", "--action", "raise"}).ipcAction, QString("raise"));
}

void TestM3M4::testCommandLineParserDiagnosticsAndInfo()
{
    CommandLineParser parser;

    QCOMPARE(parser.parse({"penguin", "--test"}).testMode, true);
    QCOMPARE(parser.parse({"penguin", "--headless-test"}).testMode, true);
    QCOMPARE(parser.parse({"penguin", "--dev-telemetry"}).devTelemetry, true);
    QCOMPARE(parser.parse({"penguin", "--help"}).helpRequested, true);
    QCOMPARE(parser.parse({"penguin", "--version"}).versionRequested, true);

    QVERIFY(!parser.helpText().isEmpty());
    QVERIFY(parser.versionText().contains("Penguin 1.0.0"));
}

// ----------------------------------------------------------------------------
// 5. MPRIS2 D-Bus Adaptors & Service Tests
// ----------------------------------------------------------------------------

void TestM3M4::testMPRIS2RootInterfaceProperties()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager playlistMgr;

    MPRIS2Service service(&engine, &playlistMgr, nullptr);
    MPRIS2RootAdaptor root(&service);

    QCOMPARE(root.canQuit(), true);
    QCOMPARE(root.canRaise(), true);
    QCOMPARE(root.canSetFullscreen(), true);
    QCOMPARE(root.hasTrackList(), false);
    QCOMPARE(root.identity(), QString("Penguin Media Player"));
    QCOMPARE(root.desktopEntry(), QString("penguin"));
    QVERIFY(root.supportedUriSchemes().contains("file"));
    QVERIFY(root.supportedMimeTypes().contains("video/mp4"));
    QVERIFY(root.supportedMimeTypes().contains("audio/flac"));
}

void TestM3M4::testMPRIS2PlayerPlaybackStateTransitions()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager playlistMgr;

    MPRIS2Service service(&engine, &playlistMgr, nullptr);
    MPRIS2PlayerAdaptor player(&service);

    // Initial state
    QCOMPARE(player.playbackStatus(), QString("Stopped"));

    // Play -> Playing (or stopped if no media)
    player.Play();
    // Pause / PlayPause calls dispatch to engine without crashing
    player.Pause();
    player.PlayPause();
    player.Stop();
    QCOMPARE(player.playbackStatus(), QString("Stopped"));
}

void TestM3M4::testMPRIS2PlayerVolumeAndRateClamping()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager playlistMgr;

    MPRIS2Service service(&engine, &playlistMgr, nullptr);
    MPRIS2PlayerAdaptor player(&service);

    // Volume clamping: 0.0 to 1.5
    player.setVolume(0.85);
    QVERIFY(std::abs(player.volume() - 0.85) < 0.05);

    player.setVolume(-0.5);
    QCOMPARE(player.volume(), 0.0);

    player.setVolume(2.5);
    QCOMPARE(player.volume(), 1.0); // clamped to engine volume max (100% -> 1.0)

    // Rate clamping: 0.5 to 2.0
    player.setRate(1.5);
    QCOMPARE(player.rate(), 1.5);

    player.setRate(0.1);
    QCOMPARE(player.rate(), 0.5);

    player.setRate(3.0);
    QCOMPARE(player.rate(), 2.0);
}

void TestM3M4::testMPRIS2PlayerMicrosecondSeeking()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager playlistMgr;

    MPRIS2Service service(&engine, &playlistMgr, nullptr);
    MPRIS2PlayerAdaptor player(&service);

    QSignalSpy spySeeked(&player, &MPRIS2PlayerAdaptor::Seeked);

    // Relative seek (10 seconds = 10,000,000 us)
    player.Seek(10000000LL);
    QCOMPARE(spySeeked.count(), 1);

    // Absolute SetPosition
    player.SetPosition(QDBusObjectPath("/org/mpris/MediaPlayer2/Track/0"), 25000000LL);
    QCOMPARE(spySeeked.count(), 2);
    qlonglong pos = spySeeked.last().at(0).toLongLong();
    QCOMPARE(pos, 25000000LL);
}

void TestM3M4::testMPRIS2PlayerMetadataDictionaryFormatting()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager playlistMgr;

    MPRIS2Service service(&engine, &playlistMgr, nullptr);
    MPRIS2PlayerAdaptor player(&service);

    QVariantMap meta = player.metadata();
    QVERIFY(meta.contains("mpris:trackid"));
    QVERIFY(meta.contains("mpris:length"));
    QVERIFY(meta.contains("xesam:title"));
    QVERIFY(meta.contains("xesam:artist"));

    QCOMPARE(meta["mpris:length"].toLongLong(), 0LL);
}

void TestM3M4::testMPRIS2DBusServiceRegistrationAndFallback()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager playlistMgr;

    MPRIS2Service service(&engine, &playlistMgr, nullptr);
    DBusService dbusService(&service);

    // In offscreen/container, either registers on session bus or gracefully falls back
    bool connected = dbusService.isConnected();
    if (connected) {
        bool reg = dbusService.registerService();
        QVERIFY(reg);
        QVERIFY(dbusService.isRegistered());
        dbusService.unregisterService();
        QVERIFY(!dbusService.isRegistered());
    } else {
        QVERIFY(!dbusService.isRegistered());
    }
}

// ----------------------------------------------------------------------------
// 6. Packaging & Assets Tests
// ----------------------------------------------------------------------------

void TestM3M4::testDesktopEntryFileSpecCompliance()
{
    QFile file("penguin.desktop");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains("[Desktop Entry]"));
    QVERIFY(content.contains("Name=Penguin"));
    QVERIFY(content.contains("Exec=penguin %U"));
    QVERIFY(content.contains("Icon=penguin"));
    QVERIFY(content.contains("Categories=AudioVideo;Audio;Video;Player;Qt;"));
    QVERIFY(content.contains("MimeType="));
    QVERIFY(content.contains("[Desktop Action PlayPause]"));
    QVERIFY(content.contains("[Desktop Action Next]"));
    QVERIFY(content.contains("[Desktop Action Previous]"));
    QVERIFY(content.contains("[Desktop Action Stop]"));
}

void TestM3M4::testSvgIconValidXmlAndBranding()
{
    QFile file("icons/hicolor/scalable/apps/penguin.svg");
    QVERIFY(file.open(QIODevice::ReadOnly));

    QXmlStreamReader xml(&file);
    bool hasSvgTag = false;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name().toString() == "svg") {
            hasSvgTag = true;
            break;
        }
    }
    QVERIFY(hasSvgTag);
    QVERIFY(!xml.hasError());
    file.close();

    // Verify Brutalist colors present
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString svgStr = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(svgStr.contains("#070709"));
    QVERIFY(svgStr.contains("#FF4400"));
    QVERIFY(svgStr.contains("#CCFF00"));
    QVERIFY(svgStr.contains("#00E5FF"));
}

void TestM3M4::testPngIconFilesExist()
{
    QVERIFY(QFile::exists("icons/hicolor/48x48/apps/penguin.png"));
    QVERIFY(QFile::exists("icons/hicolor/256x256/apps/penguin.png"));
    QVERIFY(QFile::exists("icons/hicolor/512x512/apps/penguin.png"));
    QVERIFY(QFile::exists("icons/hicolor/symbolic/apps/penguin-symbolic.svg"));
}

QTEST_MAIN(TestM3M4)
#include "test_m3_m4.moc"
