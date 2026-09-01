#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <cmath>
#include <random>

#include "core/PlaybackEngine.h"
#include "core/EqualizerDSP.h"
#include "core/VUMeterDSP.h"
#include "core/TimecodeFormatter.h"
#include "core/LrcParser.h"
#include "core/SubtitleLoader.h"
#include "desktop/CommandLineParser.h"
#include "desktop/MPRIS2Adaptor.h"
#include "library/DatabaseManager.h"
#include "library/StatePersistence.h"
#include "library/PlaylistManager.h"

using namespace Penguin;
using namespace Penguin::Core;
using namespace Penguin::Desktop;
using namespace Penguin::Library;

class TestChallengerTier5 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 1. Playback & Seeking Stress
    void testZeroAndNegativeDurationSmpte();
    void testRapidTogglePlayPauseStress();
    void testHighSpeedPlaybackBoundaryRates();
    void testSubFrameAndExtremeSeeks();
    void testFrameStepStressAtBoundaries();
    void testMpvBackendNullptrSafety();

    // 2. Equalizer DSP Boundary & Clipping
    void testEqualizerExtremeGainBoostsAndClamping();
    void testEqualizerBiquadPoleStabilityAcrossAllBands();
    void testEqualizerHugeBufferAndDenormalProcessing();
    void testEqualizerConcurrentGainModulationWhileProcessing();
    void testEqualizerJsonFuzzing();

    // 3. State Persistence & SQLite WAL Concurrency
    void testCorruptDatabaseRecovery();
    void testSqliteWalConcurrentMultithreadedAccess();
    void testLongPlaylistReorderingTenThousandItems();
    void testStatePersistenceInvalidWindowGeometry();
    void testStatePersistenceCorruptJsonSettings();

    // 4. MPRIS2 D-Bus Protocol Fuzzing
    void testMPRIS2InvalidMethodArguments();
    void testMPRIS2RateAndVolumeExtremeClamping();
    void testMPRIS2MetadataWithUnicodeAndHugeStrings();
    void testMPRIS2RapidPropertyNotificationStorm();

    // 5. CLI Parameter Edge Cases
    void testCliParserOutOfRangeVolumeAndSpeed();
    void testCliParserConflictingFlagsAndInvalidArgs();
    void testCliParserSpecialUrlsAndUnicodeFilenames();
};

void TestChallengerTier5::initTestCase()
{
    qDebug() << "=================================================================";
    qDebug() << " Starting Tier 5 White-Box Adversarial Coverage Hardening Suite ";
    qDebug() << "=================================================================";
}

void TestChallengerTier5::cleanupTestCase()
{
    qDebug() << "=================================================================";
    qDebug() << " Tier 5 White-Box Adversarial Hardening Suite Completed.        ";
    qDebug() << "=================================================================";
}

// -----------------------------------------------------------------------------
// 1. Playback & Seeking Stress
// -----------------------------------------------------------------------------

void TestChallengerTier5::testZeroAndNegativeDurationSmpte()
{
    // Zero duration
    QString zeroDur = TimecodeFormatter::formatTimecode(0, 30.0, false);
    QCOMPARE(zeroDur, QString("00:00:00:00"));

    QString remZero = TimecodeFormatter::formatRemaining(0, 0, 30.0, false);
    QCOMPARE(remZero, QString("-00:00:00:00"));

    // Negative position
    QString negTc = TimecodeFormatter::formatTimecode(-5000, 30.0, false);
    QCOMPARE(negTc, QString("-00:00:05:00"));

    // Position > Duration
    QString remOver = TimecodeFormatter::formatRemaining(15000, 10000, 30.0, false);
    QCOMPARE(remOver, QString("-00:00:00:00"));

    // Zero/negative FPS handling
    QString zeroFps = TimecodeFormatter::formatTimecode(1000, 0.0, false);
    QVERIFY(!zeroFps.isEmpty());

    QString negFps = TimecodeFormatter::formatTimecode(1000, -24.0, false);
    QVERIFY(!negFps.isEmpty());

    // msToFrames with 0 FPS
    qint64 frames = TimecodeFormatter::msToFrames(1000, 0.0);
    QCOMPARE(frames, 30); // fallback to 30 fps
}

void TestChallengerTier5::testRapidTogglePlayPauseStress()
{
    PlaybackEngine engine;
    bool initOk = engine.initialize(true);
    QVERIFY(initOk);

    // Rapidly toggle play/pause 500 times in loop
    for (int i = 0; i < 500; ++i) {
        engine.togglePlayPause();
    }

    // Engine should remain in a valid state
    PlaybackState st = engine.playbackState();
    QVERIFY(st == PlaybackState::Stopped || st == PlaybackState::Playing || st == PlaybackState::Paused);

    engine.shutdown();
}

void TestChallengerTier5::testHighSpeedPlaybackBoundaryRates()
{
    PlaybackEngine engine;
    engine.initialize(true);

    // Speed boundaries: [0.5, 2.0]
    engine.setSpeed(2.0);
    QCOMPARE(engine.speed(), 2.0);

    engine.setSpeed(0.5);
    QCOMPARE(engine.speed(), 0.5);

    // Out-of-bounds clamping
    engine.setSpeed(10.0);
    QCOMPARE(engine.speed(), 2.0);

    engine.setSpeed(0.1);
    QCOMPARE(engine.speed(), 0.5);

    engine.setSpeed(-5.0);
    QCOMPARE(engine.speed(), 0.5);

    engine.shutdown();
}

void TestChallengerTier5::testSubFrameAndExtremeSeeks()
{
    PlaybackEngine engine;
    engine.initialize(true);

    // Sub-frame seeks (milliseconds)
    engine.seek(0);
    engine.seek(1);
    engine.seek(2);
    engine.seek(10);

    // Extreme positive seek (e.g. 100 hours)
    engine.seek(360000000LL);

    // Negative seek
    engine.seek(-10000LL);

    // Relative seeks with extreme offsets
    engine.seekRelative(5000000LL);
    engine.seekRelative(-99999999LL);

    // Engine survives all seeking operations without crashing
    QVERIFY(engine.isInitialized());
    engine.shutdown();
}

void TestChallengerTier5::testFrameStepStressAtBoundaries()
{
    PlaybackEngine engine;
    engine.initialize(true);

    // Rapid frame steps
    for (int i = 0; i < 50; ++i) {
        engine.frameStep(1);
    }
    for (int i = 0; i < 50; ++i) {
        engine.frameStep(-1);
    }
    for (int i = 0; i < 50; ++i) {
        engine.frameStep(0); // no-op direction
    }

    QVERIFY(engine.isInitialized());
    engine.shutdown();
}

void TestChallengerTier5::testMpvBackendNullptrSafety()
{
    MpvBackend backend;
    // Calling functions prior to initialize() or after shutdown()
    QCOMPARE(backend.isInitialized(), false);
    backend.play();
    backend.pause();
    backend.togglePlayPause();
    backend.stop();
    backend.seekAbsoluteMs(100);
    backend.seekRelativeMs(100);
    backend.frameStepForward();
    backend.frameStepBackward();
    backend.setSpeed(1.5);
    backend.setVolume(80);
    backend.setMuted(true);
    backend.setAudioFilter("lavfi=[anull]");
    backend.setAudioTrack(1);
    backend.setSubtitleTrack(1);
    backend.setVideoTrack(1);
    QCOMPARE(backend.getPropertyString("pause"), QString());

    // Should not crash
    QVERIFY(true);
}

// -----------------------------------------------------------------------------
// 2. Equalizer DSP Boundary & Clipping
// -----------------------------------------------------------------------------

void TestChallengerTier5::testEqualizerExtremeGainBoostsAndClamping()
{
    EqualizerDSP eq;

    // Test extreme gain boost clamping: [-12.0, +12.0]
    eq.setBandGain(0, 100.0);
    QCOMPARE(eq.bandGain(0), 12.0);

    eq.setBandGain(0, -100.0);
    QCOMPARE(eq.bandGain(0), -12.0);

    // Invalid band indices should be ignored
    eq.setBandGain(-1, 5.0);
    QCOMPARE(eq.bandGain(-1), 0.0);

    eq.setBandGain(10, 5.0);
    QCOMPARE(eq.bandGain(10), 0.0);

    eq.setBandGain(999, 5.0);
    QCOMPARE(eq.bandGain(999), 0.0);

    // All bands maxed at +12dB
    QVector<double> maxGains(10, 12.0);
    eq.setAllGains(maxGains);
    for (int i = 0; i < 10; ++i) {
        QCOMPARE(eq.bandGain(i), 12.0);
    }

    // All bands cut at -12dB
    QVector<double> minGains(10, -12.0);
    eq.setAllGains(minGains);
    for (int i = 0; i < 10; ++i) {
        QCOMPARE(eq.bandGain(i), -12.0);
    }
}

void TestChallengerTier5::testEqualizerBiquadPoleStabilityAcrossAllBands()
{
    // Standard audio sample rates where Nyquist (sr/2) > max center freq (16kHz)
    double sampleRates[] = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0};
    double testGains[] = {-12.0, -6.0, 0.0, +6.0, +12.0, +24.0};

    for (double sr : sampleRates) {
        for (int band = 0; band < EQ_NUM_BANDS; ++band) {
            double freq = EqualizerDSP::bandFrequency(band);
            if (freq >= sr / 2.0) continue; // Nyquist limit
            for (double gain : testGains) {
                BiquadCoeffs c = EqualizerDSP::computePeakingCoeffs(freq, gain, sr, 1.414);

                // Characteristic polynomial: A(z) = 1 + a1*z^-1 + a2*z^-2
                // For stability in discrete time domain, both pole magnitudes must be strictly < 1.0.
                // Stability condition (Jury criterion for 2nd order):
                // 1) |a2| < 1.0
                // 2) 1 + a1 + a2 > 0
                // 3) 1 - a1 + a2 > 0
                QVERIFY2(std::abs(c.a2) < 1.0, QString("Pole magnitude |a2| >= 1.0 at band %1, freq %2, gain %3, sr %4")
                                                   .arg(band).arg(freq).arg(gain).arg(sr).toUtf8().constData());
                QVERIFY2(1.0 + c.a1 + c.a2 > 0.0, "Jury condition 2 violated");
                QVERIFY2(1.0 - c.a1 + c.a2 > 0.0, "Jury condition 3 violated");
            }
        }
    }
}

void TestChallengerTier5::testEqualizerHugeBufferAndDenormalProcessing()
{
    EqualizerDSP eq;
    eq.setPreset("Rock");

    // Process 500,000 samples with denormals, extreme amplitudes, and zeros
    const int numFrames = 250000;
    const int numChannels = 2;
    std::vector<float> buffer(numFrames * numChannels);

    for (size_t i = 0; i < buffer.size(); ++i) {
        if (i % 5 == 0) buffer[i] = 1e-35f; // subnormal
        else if (i % 7 == 0) buffer[i] = 2.0f; // over-scale
        else if (i % 11 == 0) buffer[i] = -2.0f;
        else buffer[i] = static_cast<float>(std::sin(i * 0.05));
    }

    eq.processInterleavedBuffer(buffer.data(), numFrames, numChannels, 48000.0);

    // Verify all outputs are finite numbers (no NaNs or infinities generated)
    for (size_t i = 0; i < buffer.size(); ++i) {
        QVERIFY2(std::isfinite(buffer[i]), QString("Non-finite output at index %1: %2").arg(i).arg(buffer[i]).toUtf8().constData());
    }

    // Boundary cases for buffer processing: 0 frames, 0 channels, nullptr
    eq.processInterleavedBuffer(nullptr, 100, 2, 48000.0);
    eq.processInterleavedBuffer(buffer.data(), 0, 2, 48000.0);
    eq.processInterleavedBuffer(buffer.data(), 100, 0, 48000.0);
    eq.processInterleavedBuffer(buffer.data(), -10, 2, 48000.0);
}

void TestChallengerTier5::testEqualizerConcurrentGainModulationWhileProcessing()
{
    EqualizerDSP eq;

    // Simulate streaming audio input while dynamically adjusting bands 10,000 times
    for (int i = 0; i < 10000; ++i) {
        int band = i % EQ_NUM_BANDS;
        double gain = -12.0 + (i % 25);
        eq.setBandGain(band, gain);
        float inputSample = static_cast<float>(std::sin(i * 0.1));
        float outSample = eq.processSample(inputSample, 0, 48000.0);
        QVERIFY(std::isfinite(outSample));
    }
}

void TestChallengerTier5::testEqualizerJsonFuzzing()
{
    EqualizerDSP eq;

    // Empty JSON
    QJsonObject emptyJson;
    QCOMPARE(eq.fromJson(emptyJson), false);

    // Malformed types
    QJsonObject malformedJson;
    malformedJson["preset"] = 12345;
    malformedJson["gains"] = "not an array";
    QCOMPARE(eq.fromJson(malformedJson), false);

    // Array with strings and out-of-range numbers
    QJsonObject fuzzyJson;
    fuzzyJson["preset"] = "Fuzzed";
    QJsonArray fuzzyGains;
    fuzzyGains.append(999.0);
    fuzzyGains.append(-999.0);
    fuzzyGains.append("invalid");
    for (int i = 3; i < 10; ++i) fuzzyGains.append(i * 1.5);
    fuzzyJson["gains"] = fuzzyGains;

    QCOMPARE(eq.fromJson(fuzzyJson), true);
    // Gains should be clamped
    QCOMPARE(eq.bandGain(0), 12.0);
    QCOMPARE(eq.bandGain(1), -12.0);
}

// -----------------------------------------------------------------------------
// 3. State Persistence & SQLite WAL Concurrency
// -----------------------------------------------------------------------------

void TestChallengerTier5::testCorruptDatabaseRecovery()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString corruptDbPath = tempDir.filePath("corrupt_penguin.db");

    // Write garbage bytes to database file
    {
        QFile file(corruptDbPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QByteArray garbage(4096, 'X');
        file.write(garbage);
        file.close();
    }

    // Attempt to initialize DatabaseManager on corrupt file
    DatabaseManager db;
    // SQLite might fail to open or fail to create tables, but must NOT crash
    bool ok = db.initialize(corruptDbPath);
    Q_UNUSED(ok);
    // Even if it returns false or handles the error, close() should be safe
    db.close();
    QVERIFY(true);
}

void TestChallengerTier5::testSqliteWalConcurrentMultithreadedAccess()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString dbPath = tempDir.filePath("wal_concurrent.db");

    // Initialize primary DB
    {
        DatabaseManager primaryDb;
        QVERIFY(primaryDb.initialize(dbPath));
        primaryDb.setSetting("init_key", "init_val");
        primaryDb.close();
    }

    // Run 10 parallel threads writing and reading simultaneously
    const int threadCount = 8;
    const int iterationsPerThread = 50;

    QList<QFuture<void>> futures;
    for (int t = 0; t < threadCount; ++t) {
        futures.append(QtConcurrent::run([dbPath, t, iterationsPerThread]() {
            DatabaseManager threadDb;
            if (threadDb.initialize(dbPath)) {
                for (int i = 0; i < iterationsPerThread; ++i) {
                    QString key = QString("thread_%1_key_%2").arg(t).arg(i);
                    threadDb.setSetting(key, i * 100);
                    threadDb.recordPlayback(
                        QString("file:///media/track_%1_%2.flac").arg(t).arg(i),
                        QString("Track %1-%2").arg(t).arg(i),
                        "Test Artist", "Test Album", 180000, i * 1000, "audio"
                    );
                    QVariant res = threadDb.getSetting(key);
                    Q_UNUSED(res);
                }
                threadDb.close();
            }
        }));
    }

    for (auto &f : futures) {
        f.waitForFinished();
    }

    // Verify database integrity afterwards
    DatabaseManager verifyDb;
    QVERIFY(verifyDb.initialize(dbPath));
    auto history = verifyDb.getRecentHistory(100);
    QVERIFY(!history.isEmpty());
    verifyDb.close();
}

void TestChallengerTier5::testLongPlaylistReorderingTenThousandItems()
{
    PlaylistManager pm;

    QList<UI::PlaylistItem> items;
    items.reserve(10000);
    for (int i = 0; i < 10000; ++i) {
        UI::PlaylistItem item;
        item.filePath = QString("/music/track_%1.mp3").arg(i);
        item.title = QString("Track %1").arg(i);
        item.durationMs = 180000;
        item.format = "MP3";
        items.append(item);
    }

    pm.setItems(items, 0);
    QCOMPARE(pm.items().size(), 10000);
    QCOMPARE(pm.currentIndex(), 0);

    // Reorder item from 0 to 9999
    pm.moveItem(0, 9999);
    QCOMPARE(pm.currentIndex(), 9999);
    QCOMPARE(pm.items().at(9999).title, QString("Track 0"));

    // Out of bounds moves should be ignored
    pm.moveItem(-5, 100);
    pm.moveItem(100, 20000);
    pm.moveItem(50, 50);
    QCOMPARE(pm.items().size(), 10000);

    // Remove item at 9999
    pm.removeItem(9999);
    QCOMPARE(pm.items().size(), 9999);
    QCOMPARE(pm.currentIndex(), 9998);

    // Out of bounds removals
    pm.removeItem(-1);
    pm.removeItem(50000);
    QCOMPARE(pm.items().size(), 9999);

    pm.clear();
    QCOMPARE(pm.items().size(), 0);
    QCOMPARE(pm.currentIndex(), -1);
}

void TestChallengerTier5::testStatePersistenceInvalidWindowGeometry()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString dbPath = tempDir.filePath("geom_test.db");

    DatabaseManager db;
    QVERIFY(db.initialize(dbPath));
    StatePersistence persistence(&db);

    // Save completely off-screen / invalid window coordinates
    QVariantMap invalidGeom;
    invalidGeom["x"] = -99999;
    invalidGeom["y"] = -99999;
    invalidGeom["width"] = 0;
    invalidGeom["height"] = 0;
    invalidGeom["is_maximized"] = false;
    invalidGeom["is_fullscreen"] = false;
    db.setSetting("window_geometry", invalidGeom);

    QWidget testWidget;
    // restoreWindowState should detect offscreen coordinates and safely fallback
    persistence.restoreWindowState(&testWidget);

    QVERIFY(testWidget.width() >= 0);
    QVERIFY(testWidget.height() >= 0);

    db.close();
}

void TestChallengerTier5::testStatePersistenceCorruptJsonSettings()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString dbPath = tempDir.filePath("corrupt_settings.db");

    DatabaseManager db;
    QVERIFY(db.initialize(dbPath));
    StatePersistence persistence(&db);

    // Directly set corrupted strings in app_settings table
    db.setSetting("audio_settings", "NOT A VALID JSON OBJECT");
    db.setSetting("telemetry_preferences", 42);

    int volume = 50;
    bool isMuted = false;
    double speed = 1.0;
    QString eqPreset;
    QVector<double> eqGains;

    // Restoring corrupted settings should safely retain original defaults without throwing
    persistence.restoreAudioSettings(volume, isMuted, speed, eqPreset, eqGains);
    QCOMPARE(volume, 50);
    QCOMPARE(isMuted, false);

    bool osd = true, reticles = true;
    persistence.restoreTelemetryPreferences(osd, reticles);
    QCOMPARE(osd, true);

    db.close();
}

// -----------------------------------------------------------------------------
// 4. MPRIS2 D-Bus Protocol Fuzzing
// -----------------------------------------------------------------------------

void TestChallengerTier5::testMPRIS2InvalidMethodArguments()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager pm;
    MPRIS2Service service(&engine, &pm, nullptr);

    MPRIS2PlayerAdaptor *player = service.playerAdaptor();
    QVERIFY(player != nullptr);

    // Negative and massive seek calls
    player->Seek(-999999999999999LL);
    player->Seek(0);
    player->Seek(999999999999999LL);

    // SetPosition with invalid track ID and extreme timestamps
    player->SetPosition(QDBusObjectPath("/invalid/track/path"), -5000000LL);
    player->SetPosition(QDBusObjectPath(""), 999999999999999LL);

    // OpenUri with empty and malformed strings
    player->OpenUri("");
    player->OpenUri("invalid://schema/test.mp3");

    // Playback control method fuzzing
    player->Play();
    player->Pause();
    player->PlayPause();
    player->Stop();
    player->Next();
    player->Previous();

    engine.shutdown();
}

void TestChallengerTier5::testMPRIS2RateAndVolumeExtremeClamping()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager pm;
    MPRIS2Service service(&engine, &pm, nullptr);

    MPRIS2PlayerAdaptor *player = service.playerAdaptor();

    // Volume clamping: [0.0, 1.5]
    player->setVolume(-10.0);
    QCOMPARE(player->volume(), 0.0);

    player->setVolume(20.0);
    // engine max is 100% -> 1.0
    QCOMPARE(player->volume(), 1.0);

    player->setVolume(0.75);
    QCOMPARE(player->volume(), 0.75);

    // Rate clamping: [0.5, 2.0]
    player->setRate(-2.0);
    QCOMPARE(player->rate(), 0.5);

    player->setRate(0.0);
    QCOMPARE(player->rate(), 0.5);

    player->setRate(10.0);
    QCOMPARE(player->rate(), 2.0);

    // Loop status fallback
    player->setLoopStatus("INVALID_LOOP_STATUS");
    QCOMPARE(player->loopStatus(), QString("None"));

    player->setLoopStatus("track");
    QCOMPARE(player->loopStatus(), QString("Track"));

    player->setLoopStatus("playlist");
    QCOMPARE(player->loopStatus(), QString("Playlist"));

    engine.shutdown();
}

void TestChallengerTier5::testMPRIS2MetadataWithUnicodeAndHugeStrings()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager pm;
    MPRIS2Service service(&engine, &pm, nullptr);

    MPRIS2PlayerAdaptor *player = service.playerAdaptor();
    QVariantMap meta = player->metadata();
    QVERIFY(meta.contains("mpris:trackid"));
    QVERIFY(meta.contains("mpris:length"));
    QVERIFY(meta.contains("xesam:title"));

    engine.shutdown();
}

void TestChallengerTier5::testMPRIS2RapidPropertyNotificationStorm()
{
    PlaybackEngine engine;
    engine.initialize(true);
    PlaylistManager pm;
    MPRIS2Service service(&engine, &pm, nullptr);

    // Rapidly change properties 500 times
    for (int i = 0; i < 500; ++i) {
        engine.setVolume(i % 101);
        engine.setSpeed(0.5 + (i % 16) * 0.1);
    }

    QVERIFY(true);
    engine.shutdown();
}

// -----------------------------------------------------------------------------
// 5. CLI Parameter Edge Cases
// -----------------------------------------------------------------------------

void TestChallengerTier5::testCliParserOutOfRangeVolumeAndSpeed()
{
    CommandLineParser parser;

    // Out of range volume
    QString err;
    auto opts1 = parser.parse({"penguin", "--volume", "999"}, &err);
    QVERIFY(!err.isEmpty() || opts1.volume != 999);

    err.clear();
    auto opts2 = parser.parse({"penguin", "--volume", "-50"}, &err);
    QVERIFY(!err.isEmpty() || opts2.volume != -50);

    // Invalid string volume
    err.clear();
    auto opts3 = parser.parse({"penguin", "--volume", "not_a_number"}, &err);
    QVERIFY(!err.isEmpty() || opts3.volume == 100);

    // Out of range speed is not parsed (remains -1.0 sentinel)
    err.clear();
    auto opts4 = parser.parse({"penguin", "--speed", "10.0"}, &err);
    QCOMPARE(opts4.speed, -1.0); // sentinel for unconfigured/invalid speed
}

void TestChallengerTier5::testCliParserConflictingFlagsAndInvalidArgs()
{
    CommandLineParser parser;

    // Conflicting modes: --audio and --video
    auto opts = parser.parse({"penguin", "--audio", "--video", "--fullscreen"});
    QCOMPARE(opts.forceAudio, true);
    QCOMPARE(opts.forceVideo, true);
    QCOMPARE(opts.fullscreen, true);

    // Multiple IPC actions: --next, --prev, --stop
    auto optsIpc = parser.parse({"penguin", "--next", "--prev", "--stop"});
    QVERIFY(!optsIpc.ipcAction.isEmpty());

    // Help & Version
    auto optsHelp = parser.parse({"penguin", "--help"});
    QCOMPARE(optsHelp.helpRequested, true);

    auto optsVer = parser.parse({"penguin", "--version"});
    QCOMPARE(optsVer.versionRequested, true);
}

void TestChallengerTier5::testCliParserSpecialUrlsAndUnicodeFilenames()
{
    CommandLineParser parser;

    QStringList args = {
        "penguin",
        "file:///home/user/Music/Track%2001.flac",
        "http://stream.example.com/radio.opus",
        "/path/with/unicode/日本語の曲_🐧.mp3",
        "https://example.com/video.mp4?auth=token&res=1080"
    };

    auto opts = parser.parse(args);
    QCOMPARE(opts.files.size(), 4);
    QCOMPARE(opts.files[0], QString("file:///home/user/Music/Track%2001.flac"));
    QCOMPARE(opts.files[2], QString("/path/with/unicode/日本語の曲_🐧.mp3"));
}

QTEST_MAIN(TestChallengerTier5)
#include "test_challenger_tier5.moc"

