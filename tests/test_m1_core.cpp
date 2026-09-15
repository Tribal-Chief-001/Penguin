#include <QtTest>
#include <QCoreApplication>
#include <clocale>
#include <cmath>

#include "TimecodeFormatter.h"
#include "LrcParser.h"
#include "EqualizerDSP.h"
#include "VUMeterDSP.h"
#include "SubtitleLoader.h"
#include "MpvBackend.h"
#include "PlaybackEngine.h"

using namespace Penguin::Core;

class TestM1Core : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 1. TimecodeFormatter Tests
    void testTimecodeNdfCalculation();
    void testTimecodeDropFrameCalculation();
    void testTimecodeRoundtrip();
    void testTimecodeRemaining();
    void testTimecodeSimpleFormat();
    void testTimecodeEdgeCases();

    // 2. LrcParser Tests
    void testLrcParsingBasic();
    void testLrcMultiTimestamp();
    void testLrcMetadataAndOffset();
    void testLrcBinarySearch();
    void testLrcEmptyAndMalformed();

    // 3. EqualizerDSP Tests
    void testEqualizerBandFrequenciesAndGains();
    void testEqualizerBiquadMath();
    void testEqualizerPresets();
    void testEqualizerFilterStringSerialization();
    void testEqualizerJsonSerialization();
    void testEqualizerBufferProcessing();

    // 4. VUMeterDSP Tests
    void testVUMeterDecibelConversions();
    void testVUMeterStereoProcessing();
    void testVUMeterBallisticsDecay();
    void testVUMeterPeakHold();
    void testVUMeterClipping();

    // 5. SubtitleLoader Tests
    void testSubtitleLoaderSrt();
    void testSubtitleLoaderVtt();
    void testSubtitleLoaderAss();
    void testSubtitleLoaderActiveLookup();

    // 6. PlaybackEngine & MpvBackend Tests
    void testMpvBackendLifecycle();
    void testPlaybackEngineInitialization();
    void testPlaybackEngineControlsAndSignals();
    void testPlaybackEngineEqualizerIntegration();
    void testPlaybackEngineMediaLoadAndSeek();
    void testPlaybackEngineSidecarAutoDiscovery();
};

void TestM1Core::initTestCase()
{
    setlocale(LC_NUMERIC, "C");
    qDebug() << "Initializing M1 Core Test Suite...";
}

void TestM1Core::cleanupTestCase()
{
    qDebug() << "M1 Core Test Suite Finished.";
}

// ----------------------------------------------------------------------------
// 1. TimecodeFormatter Tests
// ----------------------------------------------------------------------------

void TestM1Core::testTimecodeNdfCalculation()
{
    // Test 24 fps
    QCOMPARE(TimecodeFormatter::formatTimecode(0, 24.0), QString("00:00:00:00"));
    QCOMPARE(TimecodeFormatter::formatTimecode(1000, 24.0), QString("00:00:01:00"));
    QCOMPARE(TimecodeFormatter::formatTimecode(1500, 24.0), QString("00:00:01:12"));
    QCOMPARE(TimecodeFormatter::formatTimecode(3661000, 24.0), QString("01:01:01:00"));

    // Test 25 fps (PAL)
    QCOMPARE(TimecodeFormatter::formatTimecode(1000, 25.0), QString("00:00:01:00"));
    QCOMPARE(TimecodeFormatter::formatTimecode(1040, 25.0), QString("00:00:01:01"));

    // Test 30 fps
    QCOMPARE(TimecodeFormatter::formatTimecode(1000, 30.0), QString("00:00:01:00"));
    QCOMPARE(TimecodeFormatter::formatTimecode(500, 30.0), QString("00:00:00:15"));

    // Test 60 fps
    QCOMPARE(TimecodeFormatter::formatTimecode(1000, 60.0), QString("00:00:01:00"));
    QCOMPARE(TimecodeFormatter::formatTimecode(500, 60.0), QString("00:00:00:30"));
}

void TestM1Core::testTimecodeDropFrameCalculation()
{
    // Drop frame for 29.97 fps uses ';' separator
    QString dfTc0 = TimecodeFormatter::formatTimecode(0, 29.97, true);
    QCOMPARE(dfTc0, QString("00:00:00;00"));

    QString dfTc1s = TimecodeFormatter::formatTimecode(1000, 29.97, true);
    QCOMPARE(dfTc1s, QString("00:00:01;00"));

    // 10 minutes should not drop frames at 10-min boundary
    QString dfTc10m = TimecodeFormatter::formatTimecode(600000, 29.97, true);
    QVERIFY(dfTc10m.contains(";"));
    QCOMPARE(dfTc10m.left(8), QString("00:10:00"));
}

void TestM1Core::testTimecodeRoundtrip()
{
    double fpsList[] = {24.0, 25.0, 30.0, 50.0, 60.0};
    qint64 testMsList[] = {0, 1000, 45200, 125800, 3600000, 7325000};

    for (double fps : fpsList) {
        for (qint64 ms : testMsList) {
            QString formatted = TimecodeFormatter::formatTimecode(ms, fps, false);
            qint64 parsedMs = TimecodeFormatter::timecodeToMs(formatted, fps);
            // Must be accurate within 1 frame duration
            double frameDurationMs = 1000.0 / fps;
            QVERIFY2(std::abs(ms - parsedMs) <= std::ceil(frameDurationMs) + 1,
                     qPrintable(QString("Roundtrip mismatch: ms=%1, parsed=%2, tc=%3, fps=%4")
                                    .arg(ms).arg(parsedMs).arg(formatted).arg(fps)));
        }
    }
}

void TestM1Core::testTimecodeRemaining()
{
    qint64 durationMs = 100000; // 100s
    qint64 positionMs = 20000;  // 20s
    QString remaining = TimecodeFormatter::formatRemaining(positionMs, durationMs, 30.0);
    QVERIFY(remaining.startsWith("-"));
    QCOMPARE(remaining, QString("-00:01:20:00"));

    // Position >= duration
    QString zeroRem = TimecodeFormatter::formatRemaining(100000, 100000, 30.0);
    QCOMPARE(zeroRem, QString("-00:00:00:00"));
}

void TestM1Core::testTimecodeSimpleFormat()
{
    QCOMPARE(TimecodeFormatter::formatSimpleTime(0), QString("00:00"));
    QCOMPARE(TimecodeFormatter::formatSimpleTime(65000), QString("01:05"));
    QCOMPARE(TimecodeFormatter::formatSimpleTime(3665000), QString("01:01:05"));
}

void TestM1Core::testTimecodeEdgeCases()
{
    // Negative ms
    QString negTc = TimecodeFormatter::formatTimecode(-1000, 30.0);
    QCOMPARE(negTc, QString("-00:00:01:00"));

    // 0 FPS defaults safely to 30 FPS
    QString zeroFpsTc = TimecodeFormatter::formatTimecode(1000, 0.0);
    QCOMPARE(zeroFpsTc, QString("00:00:01:00"));

    // Validation
    QVERIFY(TimecodeFormatter::isValidTimecode("00:14:22:15"));
    QVERIFY(TimecodeFormatter::isValidTimecode("00:14:22;15"));
    QVERIFY(!TimecodeFormatter::isValidTimecode("invalid"));
}

// ----------------------------------------------------------------------------
// 2. LrcParser Tests
// ----------------------------------------------------------------------------

void TestM1Core::testLrcParsingBasic()
{
    QString lrc =
        "[ti:Synthetic Horizon]\n"
        "[ar:Penguin Core]\n"
        "[al:Brutalist Media]\n"
        "[00:01.50]Line 1: Initializing system\n"
        "[00:04.20]Line 2: Observing signals\n"
        "[00:08.80]Line 3: Stream nominal\n";

    LrcParser parser;
    bool ok = parser.parse(lrc);
    QVERIFY(ok);
    QCOMPARE(parser.title(), QString("Synthetic Horizon"));
    QCOMPARE(parser.artist(), QString("Penguin Core"));
    QCOMPARE(parser.album(), QString("Brutalist Media"));
    QCOMPARE(parser.count(), 3);

    QCOMPARE(parser.cues()[0].timestampMs, 1500LL);
    QCOMPARE(parser.cues()[0].text, QString("Line 1: Initializing system"));

    QCOMPARE(parser.cues()[1].timestampMs, 4200LL);
    QCOMPARE(parser.cues()[1].text, QString("Line 2: Observing signals"));

    QCOMPARE(parser.cues()[2].timestampMs, 8800LL);
    QCOMPARE(parser.cues()[2].text, QString("Line 3: Stream nominal"));
}

void TestM1Core::testLrcMultiTimestamp()
{
    QString lrc =
        "[00:02.00][00:10.00][00:20.00]Chorus: Penguin Power\n"
        "[00:05.00]Verse 1\n";

    LrcParser parser;
    bool ok = parser.parse(lrc);
    QVERIFY(ok);
    QCOMPARE(parser.count(), 4);

    // Cues must be chronologically sorted
    QCOMPARE(parser.cues()[0].timestampMs, 2000LL);
    QCOMPARE(parser.cues()[0].text, QString("Chorus: Penguin Power"));

    QCOMPARE(parser.cues()[1].timestampMs, 5000LL);
    QCOMPARE(parser.cues()[1].text, QString("Verse 1"));

    QCOMPARE(parser.cues()[2].timestampMs, 10000LL);
    QCOMPARE(parser.cues()[2].text, QString("Chorus: Penguin Power"));

    QCOMPARE(parser.cues()[3].timestampMs, 20000LL);
    QCOMPARE(parser.cues()[3].text, QString("Chorus: Penguin Power"));
}

void TestM1Core::testLrcMetadataAndOffset()
{
    QString lrc =
        "[offset:500]\n"
        "[00:01.00]Shifted line\n";

    LrcParser parser;
    parser.parse(lrc);
    QCOMPARE(parser.offsetMs(), 500LL);
    QCOMPARE(parser.cues()[0].timestampMs, 1500LL); // 1000 + 500
}

void TestM1Core::testLrcBinarySearch()
{
    QString lrc =
        "[00:02.00]Line 1\n"
        "[00:05.00]Line 2\n"
        "[00:10.00]Line 3\n";

    LrcParser parser;
    parser.parse(lrc);

    // Before first cue
    QCOMPARE(parser.findActiveCueIndex(1000), -1);
    QCOMPARE(parser.activeCue(1000).timestampMs, -1LL);

    // Exactly at first cue
    QCOMPARE(parser.findActiveCueIndex(2000), 0);
    QCOMPARE(parser.activeCue(2000).text, QString("Line 1"));

    // Between cue 1 and cue 2
    QCOMPARE(parser.findActiveCueIndex(3500), 0);
    QCOMPARE(parser.activeCue(3500).text, QString("Line 1"));

    // At cue 2
    QCOMPARE(parser.findActiveCueIndex(5000), 1);
    QCOMPARE(parser.activeCue(5000).text, QString("Line 2"));

    // At or beyond last cue
    QCOMPARE(parser.findActiveCueIndex(10000), 2);
    QCOMPARE(parser.findActiveCueIndex(50000), 2);
    QCOMPARE(parser.activeCue(50000).text, QString("Line 3"));

    // Click-to-seek timestamp extraction
    QCOMPARE(parser.cueTimestampAt(1), 5000LL);
}

void TestM1Core::testLrcEmptyAndMalformed()
{
    LrcParser parser;
    QVERIFY(!parser.parse(""));
    QVERIFY(!parser.parse("   \n\n  "));
    QVERIFY(!parser.parse("No timestamps here at all"));
}

// ----------------------------------------------------------------------------
// 3. EqualizerDSP Tests
// ----------------------------------------------------------------------------

void TestM1Core::testEqualizerBandFrequenciesAndGains()
{
    EqualizerDSP eq;
    QCOMPARE(eq.bandFrequency(0), 31.25);
    QCOMPARE(eq.bandFrequency(4), 500.0);
    QCOMPARE(eq.bandFrequency(9), 16000.0);

    // Default flat
    QVERIFY(eq.isFlat());
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        QCOMPARE(eq.bandGain(i), 0.0);
    }

    // Set gain with clamping
    eq.setBandGain(0, 6.0);
    QCOMPARE(eq.bandGain(0), 6.0);
    QVERIFY(!eq.isFlat());

    eq.setBandGain(1, 20.0); // Should clamp to +12.0
    QCOMPARE(eq.bandGain(1), 12.0);

    eq.setBandGain(2, -25.0); // Should clamp to -12.0
    QCOMPARE(eq.bandGain(2), -12.0);

    // Reset flat
    eq.resetFlat();
    QVERIFY(eq.isFlat());
    QCOMPARE(eq.bandGain(0), 0.0);
}

void TestM1Core::testEqualizerBiquadMath()
{
    // Test peaking filter transfer function calculation
    // At center frequency f0 = 1000Hz, gain = +6.0dB, response should be approx +6.0dB
    EqualizerDSP eq;
    eq.setBandGain(5, 6.0); // Band 5 is 1000Hz

    double respAtCenter = eq.calculateMagnitudeResponse(1000.0, 48000.0);
    QVERIFY2(std::abs(respAtCenter - 6.0) < 0.3,
             qPrintable(QString("Magnitude response at 1kHz expected ~6.0dB, got %1dB").arg(respAtCenter)));

    // Far from center frequency (e.g. 50Hz), response should be close to 0dB
    double respFar = eq.calculateMagnitudeResponse(50.0, 48000.0);
    QVERIFY2(std::abs(respFar) < 0.5,
             qPrintable(QString("Magnitude response at 50Hz expected ~0dB, got %1dB").arg(respFar)));
}

void TestM1Core::testEqualizerPresets()
{
    EqualizerDSP eq;
    QStringList presets = EqualizerDSP::availablePresets();
    QVERIFY(presets.contains("Flat"));
    QVERIFY(presets.contains("Rock"));
    QVERIFY(presets.contains("Pop"));
    QVERIFY(presets.contains("Jazz"));
    QVERIFY(presets.contains("Electronic"));
    QVERIFY(presets.contains("Vocal Boost"));
    QVERIFY(presets.contains("Bass Boost"));
    QVERIFY(presets.contains("Treble Boost"));

    eq.setPreset("Rock");
    QCOMPARE(eq.currentPreset(), QString("Rock"));
    QCOMPARE(eq.bandGain(0), 4.5);
    QCOMPARE(eq.bandGain(9), 4.5);

    eq.setPreset("Bass Boost");
    QCOMPARE(eq.bandGain(0), 6.0);
    QCOMPARE(eq.bandGain(9), 0.0);

    eq.setPreset("Flat");
    QVERIFY(eq.isFlat());
}

void TestM1Core::testEqualizerFilterStringSerialization()
{
    EqualizerDSP eq;
    eq.resetFlat();
    // When flat, mpv filter string should be empty
    QCOMPARE(eq.toMpvFilterString(), QString(""));

    eq.setPreset("Rock");
    QString mpvFilter = eq.toMpvFilterString();
    QVERIFY(mpvFilter.startsWith("lavfi=[equalizer="));
    QVERIFY(mpvFilter.contains("f=31.25"));
    QVERIFY(mpvFilter.contains("f=16000.00"));
    QVERIFY(mpvFilter.contains("g=4.5"));

    QString gstFilter = eq.toGStreamerFilterString();
    QVERIFY(gstFilter.startsWith("equalizer-10bands"));
    QVERIFY(gstFilter.contains("band0=4.5"));
}

void TestM1Core::testEqualizerJsonSerialization()
{
    EqualizerDSP eq;
    eq.setPreset("Jazz");
    QJsonObject json = eq.toJson();

    EqualizerDSP eq2;
    bool ok = eq2.fromJson(json);
    QVERIFY(ok);
    QCOMPARE(eq2.currentPreset(), QString("Jazz"));
    QCOMPARE(eq2.bandGain(0), eq.bandGain(0));
    QCOMPARE(eq2.bandGain(5), eq.bandGain(5));
}

void TestM1Core::testEqualizerBufferProcessing()
{
    EqualizerDSP eq;
    eq.setPreset("Rock");

    constexpr int NUM_SAMPLES = 256;
    float buffer[NUM_SAMPLES * 2]; // Stereo
    for (int i = 0; i < NUM_SAMPLES * 2; ++i) {
        buffer[i] = static_cast<float>(std::sin(2.0 * M_PI * 1000.0 * (i / 2) / 48000.0));
    }

    eq.processInterleavedBuffer(buffer, NUM_SAMPLES, 2, 48000.0);

    // Verify samples are finite and not NaN
    for (int i = 0; i < NUM_SAMPLES * 2; ++i) {
        QVERIFY(!std::isnan(buffer[i]));
        QVERIFY(!std::isinf(buffer[i]));
    }
}

// ----------------------------------------------------------------------------
// 4. VUMeterDSP Tests
// ----------------------------------------------------------------------------

void TestM1Core::testVUMeterDecibelConversions()
{
    QCOMPARE(VUMeterDSP::linearToDb(1.0), 0.0);
    QVERIFY(std::abs(VUMeterDSP::linearToDb(0.5) - (-6.02)) < 0.1);
    QCOMPARE(VUMeterDSP::linearToDb(0.0), -60.0);

    QCOMPARE(VUMeterDSP::dbToNormalized(-60.0), 0.0);
    QCOMPARE(VUMeterDSP::dbToNormalized(0.0), 1.0);
    QCOMPARE(VUMeterDSP::dbToNormalized(-30.0), 0.5);
}

void TestM1Core::testVUMeterStereoProcessing()
{
    VUMeterDSP vu;
    constexpr int NUM_FRAMES = 512;
    float samples[NUM_FRAMES * 2];

    // Left channel: 0.5 amplitude (-6dB)
    // Right channel: 1.0 amplitude (0dB)
    for (int i = 0; i < NUM_FRAMES; ++i) {
        samples[i * 2] = 0.5f;
        samples[i * 2 + 1] = 1.0f;
    }

    vu.processBuffer(samples, NUM_FRAMES, 2);
    auto levels = vu.currentLevels();

    QVERIFY2(std::abs(levels.leftPeakDb - (-6.02)) < 0.5,
             qPrintable(QString("Left peak expected ~ -6dB, got %1dB").arg(levels.leftPeakDb)));
    QVERIFY2(std::abs(levels.rightPeakDb - 0.0) < 0.1,
             qPrintable(QString("Right peak expected 0dB, got %1dB").arg(levels.rightPeakDb)));
    QCOMPARE(levels.rightPeakNorm, 1.0);
}

void TestM1Core::testVUMeterBallisticsDecay()
{
    VUMeterDSP vu;
    vu.simulateSignal(0.0, 0.0, 0.0); // Instant attack to 0dB

    auto initial = vu.currentLevels();
    QCOMPARE(initial.leftPeakDb, 0.0);

    // Decay over 0.5 seconds at 20 dB/s should result in ~ -10 dB
    vu.updateTick(0.5);
    auto decayed = vu.currentLevels();
    QVERIFY2(std::abs(decayed.leftPeakDb - (-10.0)) < 0.5,
             qPrintable(QString("Peak after 0.5s decay expected ~ -10dB, got %1dB").arg(decayed.leftPeakDb)));
}

void TestM1Core::testVUMeterPeakHold()
{
    VUMeterDSP vu;
    vu.simulateSignal(0.0, 0.0, 0.0); // Jump to 0dB

    // Within peak hold window (1.0s), peak-hold value should remain at 0dB
    vu.updateTick(0.5);
    auto levels1 = vu.currentLevels();
    QCOMPARE(levels1.leftPeakHoldDb, 0.0);

    // After hold time expires (additional 1.0s), peak hold should begin decay
    vu.updateTick(1.0);
    auto levels2 = vu.currentLevels();
    QVERIFY(levels2.leftPeakHoldDb < 0.0);
}

void TestM1Core::testVUMeterClipping()
{
    VUMeterDSP vu;
    vu.simulateSignal(+1.5, +2.0, 0.0); // Clipping signal (> 0dB)

    auto levels = vu.currentLevels();
    QVERIFY(levels.leftClipped);
    QVERIFY(levels.rightClipped);

    // Remains clipped during clip hold time
    vu.updateTick(0.5);
    QVERIFY(vu.currentLevels().leftClipped);

    // Unclips after clip hold timer (1.5s total)
    vu.updateTick(1.5);
    QVERIFY(!vu.currentLevels().leftClipped);
}

// ----------------------------------------------------------------------------
// 5. SubtitleLoader Tests
// ----------------------------------------------------------------------------

void TestM1Core::testSubtitleLoaderSrt()
{
    QString srtContent =
        "1\n"
        "00:00:01,000 --> 00:00:04,500\n"
        "Hello <i>world</i> from SRT!\n"
        "\n"
        "2\n"
        "00:00:05,200 --> 00:00:08,000\n"
        "Second subtitle line\n";

    SubtitleLoader loader;
    bool ok = loader.parseSrt(srtContent);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 2);

    QCOMPARE(loader.cues()[0].startMs, 1000LL);
    QCOMPARE(loader.cues()[0].endMs, 4500LL);
    QCOMPARE(loader.cues()[0].plainText, QString("Hello world from SRT!"));

    QCOMPARE(loader.cues()[1].startMs, 5200LL);
    QCOMPARE(loader.cues()[1].endMs, 8000LL);
    QCOMPARE(loader.cues()[1].plainText, QString("Second subtitle line"));
}

void TestM1Core::testSubtitleLoaderVtt()
{
    QString vttContent =
        "WEBVTT\n"
        "\n"
        "00:01.000 --> 00:03.500\n"
        "WebVTT line 1\n"
        "\n"
        "00:04.000 --> 00:07.000\n"
        "WebVTT line 2\n";

    SubtitleLoader loader;
    bool ok = loader.parseVtt(vttContent);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 2);
    QCOMPARE(loader.cues()[0].startMs, 1000LL);
    QCOMPARE(loader.cues()[0].endMs, 3500LL);
    QCOMPARE(loader.cues()[0].plainText, QString("WebVTT line 1"));
}

void TestM1Core::testSubtitleLoaderAss()
{
    QString assContent =
        "[Script Info]\n"
        "Title: Test Script\n"
        "\n"
        "[Events]\n"
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
        "Dialogue: 0,0:00:02.00,0:00:05.50,Default,,0,0,0,,{\\b1}ASS Subtitle Text{\\b0}\n";

    SubtitleLoader loader;
    bool ok = loader.parseAss(assContent);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 1);
    QCOMPARE(loader.cues()[0].startMs, 2000LL);
    QCOMPARE(loader.cues()[0].endMs, 5500LL);
    QCOMPARE(loader.cues()[0].plainText, QString("ASS Subtitle Text"));
}

void TestM1Core::testSubtitleLoaderActiveLookup()
{
    QString srt =
        "1\n00:00:02,000 --> 00:00:05,000\nCue 1\n\n"
        "2\n00:00:06,000 --> 00:00:09,000\nCue 2\n";

    SubtitleLoader loader;
    loader.parseSrt(srt);

    QCOMPARE(loader.activeSubtitleText(1000), QString());
    QCOMPARE(loader.activeSubtitleText(3000), QString("Cue 1"));
    QCOMPARE(loader.activeSubtitleText(5500), QString());
    QCOMPARE(loader.activeSubtitleText(7000), QString("Cue 2"));
}

// ----------------------------------------------------------------------------
// 6. PlaybackEngine & MpvBackend Tests
// ----------------------------------------------------------------------------

void TestM1Core::testMpvBackendLifecycle()
{
    MpvBackend backend;
    bool ok = backend.initialize(true); // Headless offscreen
    QVERIFY(ok);
    QVERIFY(backend.isInitialized());

    // Test volume controls
    backend.setVolume(75);
    QCOMPARE(backend.volume(), 75);

    // Test mute controls
    backend.setMuted(true);
    QVERIFY(backend.isMuted());
    backend.setMuted(false);
    QVERIFY(!backend.isMuted());

    // Test speed control
    backend.setSpeed(1.5);
    QCOMPARE(backend.speed(), 1.5);

    // Test command and property access
    int res = backend.command({"set", "pause", "yes"});
    QVERIFY(res >= 0);
    QString pauseVal = backend.getPropertyString("pause");
    QCOMPARE(pauseVal, QString("yes"));

    backend.shutdown();
    QVERIFY(!backend.isInitialized());
}

void TestM1Core::testPlaybackEngineInitialization()
{
    PlaybackEngine engine;
    bool ok = engine.initialize(true);
    QVERIFY(ok);
    QVERIFY(engine.isInitialized());
    QCOMPARE(engine.playbackState(), PlaybackState::Stopped);

    engine.shutdown();
}

void TestM1Core::testPlaybackEngineControlsAndSignals()
{
    PlaybackEngine engine;
    engine.initialize(true);

    QSignalSpy stateSpy(&engine, &PlaybackEngine::playbackStateChanged);
    QSignalSpy volSpy(&engine, &PlaybackEngine::volumeChanged);
    QSignalSpy speedSpy(&engine, &PlaybackEngine::speedChanged);

    engine.setVolume(80);
    engine.setSpeed(1.25);

    QCOMPARE(engine.volume(), 80);
    QCOMPARE(engine.speed(), 1.25);

    // Formatted timecode check
    QString posTc = engine.smptePosition();
    QVERIFY(posTc.contains(":"));

    QString remTc = engine.smpteRemaining();
    QVERIFY(remTc.startsWith("-"));

    engine.shutdown();
}

void TestM1Core::testPlaybackEngineEqualizerIntegration()
{
    PlaybackEngine engine;
    engine.initialize(true);

    QSignalSpy eqSpy(&engine, &PlaybackEngine::equalizerChanged);

    engine.setEqualizerPreset("Electronic");
    QCOMPARE(engine.currentEqualizerPreset(), QString("Electronic"));
    QVERIFY(eqSpy.count() >= 1);

    engine.setEqualizerBand(0, 5.0);
    QCOMPARE(engine.equalizerBand(0), 5.0);

    engine.resetEqualizer();
    QVERIFY(engine.equalizer().isFlat());

    engine.shutdown();
}

void TestM1Core::testPlaybackEngineMediaLoadAndSeek()
{
    QString fixture = "tests/fixtures/test_video.mp4";
    if (!QFile::exists(fixture)) {
        fixture = "../tests/fixtures/test_video.mp4";
    }
    QVERIFY(QFile::exists(fixture));

    PlaybackEngine engine;
    bool ok = engine.initialize(true);
    QVERIFY(ok);

    QSignalSpy loadSpy(&engine, &PlaybackEngine::mediaLoaded);
    bool loadOk = engine.loadMedia(fixture, false);
    QVERIFY(loadOk);
    QVERIFY(loadSpy.count() >= 1);

    // Frame stepping
    engine.frameStep(1);
    engine.frameStep(-1);

    // Seeking
    engine.seek(1500);
    engine.seekRelative(-500);

    // Controls
    engine.play();
    engine.pause();
    engine.togglePlayPause();
    engine.stop();

    engine.shutdown();
}

void TestM1Core::testPlaybackEngineSidecarAutoDiscovery()
{
    QString audioFixture = "tests/fixtures/test_audio.flac";
    if (!QFile::exists(audioFixture)) {
        audioFixture = "../tests/fixtures/test_audio.flac";
    }
    QVERIFY(QFile::exists(audioFixture));

    PlaybackEngine engine;
    engine.initialize(true);

    bool loadOk = engine.loadMedia(audioFixture, false);
    QVERIFY(loadOk);

    // Sidecar .lrc auto-discovered
    QVERIFY(!engine.lrcParser().isEmpty());
    QCOMPARE(engine.lrcParser().title(), QString("Test Track"));
    QCOMPARE(engine.lrcParser().artist(), QString("Test Artist"));
    QCOMPARE(engine.lrcParser().count(), 2);

    // Sidecar .srt auto-discovered
    QVERIFY(engine.subtitleLoader().isValid());
    QCOMPARE(engine.subtitleLoader().cueCount(), 2);

    engine.stop();
    engine.shutdown();
}

QTEST_MAIN(TestM1Core)
#include "test_m1_core.moc"
