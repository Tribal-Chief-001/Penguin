#include <QtTest>
#include <QCoreApplication>
#include <clocale>
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>

#include "TimecodeFormatter.h"
#include "LrcParser.h"

using namespace Penguin::Core;

class TestChallengerM1 : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 1. Timecode Fractional Frame Rates & Drop-Frame Math
    void testFractionalFrameRatesNominalAndDropDetection();
    void testDropFrameMath2997FullHourScan();
    void testDropFrameMath5994FullHourScan();
    void testDropFrame10MinuteBoundaries();
    void testNonDropFrameFractionalRates();

    // 2. Extreme and Negative Millisecond Conversions
    void testExtremeMillisecondValues();
    void testNegativeMillisecondValues();
    void testTimecodeParserStressAndFuzz();
    void testTimecodeRemainingEdgeCases();

    // 3. Frame Stepping & Jumps
    void testFrameSteppingForwardBackwardMath();
    void testFrameSteppingAcrossDropPoints();

    // 4. LrcParser Comprehensive Stress Testing
    void testLrcTimestampFormatVariants();
    void testLrcMultiTimestampPerLine();
    void testLrcOutOfOrderSorting();
    void testLrcMetadataAndOffsets();
    void testLrcBinarySearchStress();
    void testLrcMalformedAndFuzzing();
    void testLrcUnicodeAndSpecialChars();
    void testLrcLargeFilePerformance();
};

void TestChallengerM1::initTestCase()
{
    setlocale(LC_NUMERIC, "C");
    qDebug() << "=== Starting Challenger 1 Empirical Stress Test Suite ===";
}

void TestChallengerM1::cleanupTestCase()
{
    qDebug() << "=== Challenger 1 Stress Test Suite Completed ===";
}

// ----------------------------------------------------------------------------
// 1. Timecode Fractional Frame Rates & Drop-Frame Math
// ----------------------------------------------------------------------------

void TestChallengerM1::testFractionalFrameRatesNominalAndDropDetection()
{
    // Test nominal FPS mapping
    QCOMPARE(TimecodeFormatter::nominalFps(23.976), 24);
    QCOMPARE(TimecodeFormatter::nominalFps(24000.0 / 1001.0), 24);
    QCOMPARE(TimecodeFormatter::nominalFps(24.0), 24);
    QCOMPARE(TimecodeFormatter::nominalFps(25.0), 25);
    QCOMPARE(TimecodeFormatter::nominalFps(29.97), 30);
    QCOMPARE(TimecodeFormatter::nominalFps(30000.0 / 1001.0), 30);
    QCOMPARE(TimecodeFormatter::nominalFps(30.0), 30);
    QCOMPARE(TimecodeFormatter::nominalFps(50.0), 50);
    QCOMPARE(TimecodeFormatter::nominalFps(59.94), 60);
    QCOMPARE(TimecodeFormatter::nominalFps(60000.0 / 1001.0), 60);
    QCOMPARE(TimecodeFormatter::nominalFps(60.0), 60);
    QCOMPARE(TimecodeFormatter::nominalFps(120.0), 120);

    // Negative / zero FPS fallback
    QCOMPARE(TimecodeFormatter::nominalFps(0.0), 30);
    QCOMPARE(TimecodeFormatter::nominalFps(-24.0), 30);

    // Drop frame rate detection (only 29.97 and 59.94 are drop frame)
    QVERIFY(TimecodeFormatter::isDropFrameRate(29.97));
    QVERIFY(TimecodeFormatter::isDropFrameRate(30000.0 / 1001.0));
    QVERIFY(TimecodeFormatter::isDropFrameRate(59.94));
    QVERIFY(TimecodeFormatter::isDropFrameRate(60000.0 / 1001.0));

    QVERIFY(!TimecodeFormatter::isDropFrameRate(23.976));
    QVERIFY(!TimecodeFormatter::isDropFrameRate(24000.0 / 1001.0));
    QVERIFY(!TimecodeFormatter::isDropFrameRate(24.0));
    QVERIFY(!TimecodeFormatter::isDropFrameRate(25.0));
    QVERIFY(!TimecodeFormatter::isDropFrameRate(30.0));
    QVERIFY(!TimecodeFormatter::isDropFrameRate(60.0));
}

void TestChallengerM1::testDropFrameMath2997FullHourScan()
{
    // In 29.97 DF, there are 107,892 frames in 1 hour (3600 * 30 - 108 = 107892).
    // Let's test 108,000 frame steps (1 hour of real time).
    const double fps = 30000.0 / 1001.0;
    const int totalFrames = 108000;
    
    QString previousTc = "";
    int droppedFramesEncountered = 0;

    for (int frame = 0; frame < totalFrames; ++frame) {
        qint64 ms = TimecodeFormatter::framesToMs(frame, fps);
        QString tc = TimecodeFormatter::formatTimecode(ms, fps, true);

        // 1. Must contain semicolon ';' for drop-frame
        QVERIFY2(tc.contains(';'), qPrintable(QString("Missing ';' in DF timecode at frame %1: %2").arg(frame).arg(tc)));

        // 2. Extract components
        QStringList parts = tc.split(QRegularExpression("[:;]"));
        QCOMPARE(parts.size(), 4);
        int hh = parts[0].toInt();
        int mm = parts[1].toInt();
        int ss = parts[2].toInt();
        int ff = parts[3].toInt();

        // 3. Drop-frame invariant: Frames 00 and 01 do NOT exist at minute boundaries EXCEPT minutes divisible by 10.
        if (ss == 0 && (ff == 0 || ff == 1)) {
            if (mm % 10 != 0) {
                // Violation! Dropped frame was generated!
                droppedFramesEncountered++;
            }
        }

        // 4. Frames must be in [0, 29]
        QVERIFY2(ff >= 0 && ff < 30, qPrintable(QString("Frame out of range at frame %1: %2").arg(frame).arg(tc)));

        // 5. Bidirectional roundtrip check every 100 frames
        if (frame % 100 == 0) {
            qint64 parsedMs = TimecodeFormatter::timecodeToMs(tc, fps);
            qint64 frameDurationMs = static_cast<qint64>(std::ceil(1000.0 / fps));
            QVERIFY2(std::abs(ms - parsedMs) <= frameDurationMs + 2,
                     qPrintable(QString("Roundtrip error at frame %1: ms=%2, parsedMs=%3, tc=%4")
                                    .arg(frame).arg(ms).arg(parsedMs).arg(tc)));
        }

        previousTc = tc;
    }

    QCOMPARE(droppedFramesEncountered, 0);
}

void TestChallengerM1::testDropFrameMath5994FullHourScan()
{
    // In 59.94 DF, 4 frames are dropped every minute except every 10th minute.
    const double fps = 60000.0 / 1001.0;
    const int totalFrames = 216000; // 1 hour

    int droppedFramesEncountered = 0;

    for (int frame = 0; frame < totalFrames; frame += 5) { // Sample every 5 frames for speed
        qint64 ms = TimecodeFormatter::framesToMs(frame, fps);
        QString tc = TimecodeFormatter::formatTimecode(ms, fps, true);

        QVERIFY(tc.contains(';'));
        QStringList parts = tc.split(QRegularExpression("[:;]"));
        int mm = parts[1].toInt();
        int ss = parts[2].toInt();
        int ff = parts[3].toInt();

        // For 59.94 DF, frames 00, 01, 02, 03 are dropped at mm % 10 != 0
        if (ss == 0 && (ff >= 0 && ff <= 3)) {
            if (mm % 10 != 0) {
                droppedFramesEncountered++;
            }
        }

        QVERIFY(ff >= 0 && ff < 60);

        if (frame % 500 == 0) {
            qint64 parsedMs = TimecodeFormatter::timecodeToMs(tc, fps);
            qint64 frameDurationMs = static_cast<qint64>(std::ceil(1000.0 / fps));
            QVERIFY2(std::abs(ms - parsedMs) <= frameDurationMs + 2,
                     qPrintable(QString("Roundtrip error at 59.94 DF frame %1: ms=%2, parsedMs=%3, tc=%4")
                                    .arg(frame).arg(ms).arg(parsedMs).arg(tc)));
        }
    }

    QCOMPARE(droppedFramesEncountered, 0);
}

void TestChallengerM1::testDropFrame10MinuteBoundaries()
{
    const double fps2997 = 30000.0 / 1001.0;

    // Minute 0: 00:00:00;00 to 00:00:00;01 must exist
    qint64 ms0 = TimecodeFormatter::framesToMs(0, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(ms0, fps2997, true), QString("00:00:00;00"));
    qint64 ms1 = TimecodeFormatter::framesToMs(1, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(ms1, fps2997, true), QString("00:00:00;01"));

    // Minute 1: first frame must be 00:01:00;02 (frame index 1800)
    qint64 msMin1 = TimecodeFormatter::framesToMs(1800, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(msMin1, fps2997, true), QString("00:01:00;02"));

    // Minute 9 last frame: 00:09:59;29 (frame index 17981)
    qint64 msMin9Last = TimecodeFormatter::framesToMs(17981, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(msMin9Last, fps2997, true), QString("00:09:59;29"));

    // Minute 10 first frame: 00:10:00;00 (frame index 17982) - NO frames dropped on 10th minute!
    qint64 msMin10 = TimecodeFormatter::framesToMs(17982, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(msMin10, fps2997, true), QString("00:10:00;00"));

    // Minute 10 second frame: 00:10:00;01 (frame index 17983)
    qint64 msMin10_1 = TimecodeFormatter::framesToMs(17983, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(msMin10_1, fps2997, true), QString("00:10:00;01"));

    // Minute 11 first frame: 00:11:00;02 (frame index 17982 + 1800 = 19782)
    qint64 msMin11 = TimecodeFormatter::framesToMs(19782, fps2997);
    QCOMPARE(TimecodeFormatter::formatTimecode(msMin11, fps2997, true), QString("00:11:00;02"));
}

void TestChallengerM1::testNonDropFrameFractionalRates()
{
    // 23.976 fps (standard cinema/TV 24p NTSC)
    const double fps23976 = 24000.0 / 1001.0;
    
    // 0s -> 00:00:00:00
    QCOMPARE(TimecodeFormatter::formatTimecode(0, fps23976, false), QString("00:00:00:00"));

    // 1000s -> ~23976 frames
    qint64 ms1000s = 1000000;
    QString tc1000s = TimecodeFormatter::formatTimecode(ms1000s, fps23976, false);
    QVERIFY(!tc1000s.contains(';')); // Must be ':'
    qint64 roundtripMs = TimecodeFormatter::timecodeToMs(tc1000s, fps23976);
    QVERIFY(std::abs(ms1000s - roundtripMs) <= 45); // Within 1 frame (~41.7ms)

    // 1 hour of 23.976
    qint64 ms1h = 3600000;
    QString tc1h = TimecodeFormatter::formatTimecode(ms1h, fps23976, false);
    // Note: at 23.976 fps, 1 hour of real time (3600s) has 86313.6 frames = ~00:59:56:09
    QVERIFY(tc1h.startsWith("00:59:56"));
}

// ----------------------------------------------------------------------------
// 2. Extreme and Negative Millisecond Conversions
// ----------------------------------------------------------------------------

void TestChallengerM1::testExtremeMillisecondValues()
{
    // 24 hours
    qint64 ms24h = 86400000LL;
    QString tc24h = TimecodeFormatter::formatTimecode(ms24h, 30.0);
    QCOMPARE(tc24h, QString("24:00:00:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tc24h, 30.0), ms24h);

    // 100 hours
    qint64 ms100h = 360000000LL;
    QString tc100h = TimecodeFormatter::formatTimecode(ms100h, 30.0);
    QCOMPARE(tc100h, QString("100:00:00:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tc100h, 30.0), ms100h);

    // 1000 hours
    qint64 ms1000h = 3600000000LL;
    QString tc1000h = TimecodeFormatter::formatTimecode(ms1000h, 30.0);
    QCOMPARE(tc1000h, QString("1000:00:00:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tc1000h, 30.0), ms1000h);

    // 10,000 hours
    qint64 ms10000h = 36000000000LL;
    QString tc10000h = TimecodeFormatter::formatTimecode(ms10000h, 30.0);
    QCOMPARE(tc10000h, QString("10000:00:00:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tc10000h, 30.0), ms10000h);

    // Very large timestamp (10^12 ms ~ 31.7 years)
    qint64 ms1e12 = 1000000000000LL;
    QString tc1e12 = TimecodeFormatter::formatTimecode(ms1e12, 30.0);
    QVERIFY(!tc1e12.isEmpty());
    qint64 parsed1e12 = TimecodeFormatter::timecodeToMs(tc1e12, 30.0);
    QVERIFY(std::abs(ms1e12 - parsed1e12) <= 34);

    // Simple time extreme values
    QCOMPARE(TimecodeFormatter::formatSimpleTime(ms24h), QString("24:00:00"));
    QCOMPARE(TimecodeFormatter::formatSimpleTime(ms100h), QString("100:00:00"));
}

void TestChallengerM1::testNegativeMillisecondValues()
{
    // -1 second (-1000ms)
    QString tcNeg1s = TimecodeFormatter::formatTimecode(-1000, 30.0);
    QCOMPARE(tcNeg1s, QString("-00:00:01:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tcNeg1s, 30.0), -1000LL);

    // -1 hour (-3600000ms)
    QString tcNeg1h = TimecodeFormatter::formatTimecode(-3600000, 30.0);
    QCOMPARE(tcNeg1h, QString("-01:00:00:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tcNeg1h, 30.0), -3600000LL);

    // -24 hours
    QString tcNeg24h = TimecodeFormatter::formatTimecode(-86400000, 30.0);
    QCOMPARE(tcNeg24h, QString("-24:00:00:00"));
    QCOMPARE(TimecodeFormatter::timecodeToMs(tcNeg24h, 30.0), -86400000LL);

    // Negative Simple time
    QCOMPARE(TimecodeFormatter::formatSimpleTime(-5000), QString("-00:05"));
    QCOMPARE(TimecodeFormatter::formatSimpleTime(-65000), QString("-01:05"));
    QCOMPARE(TimecodeFormatter::formatSimpleTime(-3665000), QString("-01:01:05"));
}

void TestChallengerM1::testTimecodeParserStressAndFuzz()
{
    // Test various formatting variations
    // 4 parts: HH:MM:SS:FF
    QCOMPARE(TimecodeFormatter::timecodeToMs("00:01:02:15", 30.0), 62500LL);
    // 3 parts: HH:MM:SS
    QCOMPARE(TimecodeFormatter::timecodeToMs("01:02:03", 30.0), 3723000LL);
    // 2 parts: MM:SS
    QCOMPARE(TimecodeFormatter::timecodeToMs("02:30", 30.0), 150000LL);
    // Dot separator: MM:SS.FF
    QCOMPARE(TimecodeFormatter::timecodeToMs("00:01:02.15", 30.0), 62500LL);

    // Malformed inputs - must not crash or hang
    QStringList malformed = {
        "", "   ", ":::", "invalid", "ab:cd:ef:gh",
        "00:00:00:00:00:00", "-:-:-:-", "??:??:??:??",
        "\t\n\r", "00:00:", ":00:00", "00::00",
        QString(10000, '9'), QString(10000, ':')
    };

    for (const QString &bad : malformed) {
        qint64 res = TimecodeFormatter::timecodeToMs(bad, 30.0);
        // Must return 0 or integer safely without throwing
        Q_UNUSED(res);
    }
}

void TestChallengerM1::testTimecodeRemainingEdgeCases()
{
    // Position == Duration
    QCOMPARE(TimecodeFormatter::formatRemaining(5000, 5000, 30.0), QString("-00:00:00:00"));
    // Position > Duration (overrun)
    QCOMPARE(TimecodeFormatter::formatRemaining(6000, 5000, 30.0), QString("-00:00:00:00"));
    // Position == 0
    QCOMPARE(TimecodeFormatter::formatRemaining(0, 5000, 30.0), QString("-00:00:05:00"));
    // Duration == 0
    QCOMPARE(TimecodeFormatter::formatRemaining(0, 0, 30.0), QString("-00:00:00:00"));
    // Drop-frame remaining time
    QString dfRem = TimecodeFormatter::formatRemaining(0, 60000, 29.97, true);
    QVERIFY(dfRem.startsWith("-"));
    QVERIFY(dfRem.contains(';'));
}

// ----------------------------------------------------------------------------
// 3. Frame Stepping & Jumps
// ----------------------------------------------------------------------------

void TestChallengerM1::testFrameSteppingForwardBackwardMath()
{
    double fpsList[] = {23.976, 24.0, 25.0, 29.97, 30.0, 50.0, 59.94, 60.0};
    qint64 durationMs = 60000; // 60 seconds

    for (double fps : fpsList) {
        qint64 stepDelta = TimecodeFormatter::framesToMs(1, fps);
        QVERIFY(stepDelta > 0);

        // Step forward from 0
        qint64 pos = 0;
        int frameCount = static_cast<int>(std::round(fps));
        for (int i = 0; i < frameCount; ++i) {
            qint64 nextFrame = TimecodeFormatter::msToFrames(pos, fps) + 1;
            pos = std::min(durationMs, TimecodeFormatter::framesToMs(nextFrame, fps));
        }
        // After 1 second of frames, position should be ~1000ms
        QVERIFY2(std::abs(pos - 1000) <= stepDelta,
                 qPrintable(QString("Frame step forward drift at fps %1: pos=%2").arg(fps).arg(pos)));

        // Step backward to 0
        for (int i = 0; i < frameCount; ++i) {
            qint64 prevFrame = std::max(0LL, TimecodeFormatter::msToFrames(pos, fps) - 1);
            pos = TimecodeFormatter::framesToMs(prevFrame, fps);
        }
        QCOMPARE(pos, 0LL);

        // Stepping backward at 0 remains 0
        qint64 zeroPrev = std::max(0LL, TimecodeFormatter::msToFrames(0, fps) - 1);
        QCOMPARE(zeroPrev, 0LL);

        // Stepping forward at duration remains duration
        qint64 endNext = std::min(durationMs, TimecodeFormatter::framesToMs(TimecodeFormatter::msToFrames(durationMs, fps) + 1, fps));
        QVERIFY(endNext >= durationMs);
    }
}

void TestChallengerM1::testFrameSteppingAcrossDropPoints()
{
    const double fps = 30000.0 / 1001.0;

    // Frame 1799: 00:00:59;29
    qint64 ms1799 = TimecodeFormatter::framesToMs(1799, fps);
    QString tc1799 = TimecodeFormatter::formatTimecode(ms1799, fps, true);
    QCOMPARE(tc1799, QString("00:00:59;29"));

    // Next frame (1800): 00:01:00;02 (skipping ;00 and ;01)
    qint64 ms1800 = TimecodeFormatter::framesToMs(1800, fps);
    QString tc1800 = TimecodeFormatter::formatTimecode(ms1800, fps, true);
    QCOMPARE(tc1800, QString("00:01:00;02"));

    // Previous frame from 1800 steps back to 1799
    qint64 stepBackMs = TimecodeFormatter::framesToMs(1800 - 1, fps);
    QString tcStepBack = TimecodeFormatter::formatTimecode(stepBackMs, fps, true);
    QCOMPARE(tcStepBack, QString("00:00:59;29"));
}

// ----------------------------------------------------------------------------
// 4. LrcParser Comprehensive Stress Testing
// ----------------------------------------------------------------------------

void TestChallengerM1::testLrcTimestampFormatVariants()
{
    QString lrc =
        "[00:01.50] Centiseconds 2 digits\n"
        "[00:02.123] Milliseconds 3 digits\n"
        "[00:03.4] Deciseconds 1 digit\n"
        "[00:04:75] Colon separator instead of dot\n"
        "[00:00.00] Zero timestamp\n"
        "[120:30.00] High minutes (2 hours+)\n";

    LrcParser parser;
    bool ok = parser.parse(lrc);
    QVERIFY(ok);
    QCOMPARE(parser.count(), 6);

    // Chronologically sorted:
    // 0: [00:00.00] -> 0 ms
    QCOMPARE(parser.cues()[0].timestampMs, 0LL);
    QCOMPARE(parser.cues()[0].text, QString("Zero timestamp"));

    // 1: [00:01.50] -> 1500 ms
    QCOMPARE(parser.cues()[1].timestampMs, 1500LL);

    // 2: [00:02.123] -> 2123 ms
    QCOMPARE(parser.cues()[2].timestampMs, 2123LL);

    // 3: [00:03.4] -> 3400 ms
    QCOMPARE(parser.cues()[3].timestampMs, 3400LL);

    // 4: [00:04:75] -> 4750 ms
    QCOMPARE(parser.cues()[4].timestampMs, 4750LL);

    // 5: [120:30.00] -> (120*60 + 30)*1000 = 7230000 ms
    QCOMPARE(parser.cues()[5].timestampMs, 7230000LL);
}

void TestChallengerM1::testLrcMultiTimestampPerLine()
{
    // Single line with 5 timestamps
    QString lrc = "[00:01.00][00:03.00][00:05.00][00:07.00][00:09.00] Repeating refrain";
    LrcParser parser;
    QVERIFY(parser.parse(lrc));
    QCOMPARE(parser.count(), 5);

    for (int i = 0; i < 5; ++i) {
        QCOMPARE(parser.cues()[i].timestampMs, (1 + i * 2) * 1000LL);
        QCOMPARE(parser.cues()[i].text, QString("Repeating refrain"));
    }

    // Out-of-order timestamps on same line
    QString lrcOoo = "[00:10.00][00:02.00][00:06.00] Out of order line";
    LrcParser p2;
    QVERIFY(p2.parse(lrcOoo));
    QCOMPARE(p2.count(), 3);
    QCOMPARE(p2.cues()[0].timestampMs, 2000LL);
    QCOMPARE(p2.cues()[1].timestampMs, 6000LL);
    QCOMPARE(p2.cues()[2].timestampMs, 10000LL);
}

void TestChallengerM1::testLrcOutOfOrderSorting()
{
    // Lines provided in completely reverse order
    QString lrc =
        "[00:50.00] Line 5\n"
        "[00:40.00] Line 4\n"
        "[00:30.00] Line 3\n"
        "[00:20.00] Line 2\n"
        "[00:10.00] Line 1\n";

    LrcParser parser;
    QVERIFY(parser.parse(lrc));
    QCOMPARE(parser.count(), 5);

    for (int i = 0; i < 5; ++i) {
        QCOMPARE(parser.cues()[i].timestampMs, (i + 1) * 10000LL);
        QCOMPARE(parser.cues()[i].text, QString("Line %1").arg(i + 1));
    }
}

void TestChallengerM1::testLrcMetadataAndOffsets()
{
    // Case-insensitive tags & positive offset
    QString lrc =
        "[TI: Tactile Brutalism ]\n"
        "[AR: Penguin System ]\n"
        "[AL: Studio Core ]\n"
        "[BY: Chief Engineer ]\n"
        "[OFFSET: +1500]\n"
        "[00:01.00] First shifted line\n";

    LrcParser parser;
    QVERIFY(parser.parse(lrc));
    QCOMPARE(parser.title(), QString("Tactile Brutalism"));
    QCOMPARE(parser.artist(), QString("Penguin System"));
    QCOMPARE(parser.album(), QString("Studio Core"));
    QCOMPARE(parser.author(), QString("Chief Engineer"));
    QCOMPARE(parser.offsetMs(), 1500LL);
    QCOMPARE(parser.cues()[0].timestampMs, 2500LL); // 1000 + 1500

    // Negative offset with clamping at 0
    QString lrcNeg =
        "[offset:-2000]\n"
        "[00:01.00] Clamped line\n"
        "[00:05.00] Positive shifted line\n";

    LrcParser p2;
    QVERIFY(p2.parse(lrcNeg));
    QCOMPARE(p2.offsetMs(), -2000LL);
    QCOMPARE(p2.cues()[0].timestampMs, 0LL); // 1000 - 2000 clamped to 0
    QCOMPARE(p2.cues()[1].timestampMs, 3000LL); // 5000 - 2000 = 3000
}

void TestChallengerM1::testLrcBinarySearchStress()
{
    // Build an LRC with 1000 cues, 1 second apart
    QString lrc;
    for (int i = 0; i < 1000; ++i) {
        int mm = i / 60;
        int ss = i % 60;
        lrc += QString("[%1:%2.00] Cue %3\n")
                   .arg(mm, 2, 10, QChar('0'))
                   .arg(ss, 2, 10, QChar('0'))
                   .arg(i);
    }

    LrcParser parser;
    QVERIFY(parser.parse(lrc));
    QCOMPARE(parser.count(), 1000);

    // Test exact lookups
    for (int i = 0; i < 1000; ++i) {
        qint64 targetMs = i * 1000LL;
        QCOMPARE(parser.findActiveCueIndex(targetMs), i);
        QCOMPARE(parser.activeCue(targetMs).text, QString("Cue %1").arg(i));
        // Test halfway points
        if (i < 999) {
            QCOMPARE(parser.findActiveCueIndex(targetMs + 500), i);
        }
    }

    // Before first cue
    QCOMPARE(parser.findActiveCueIndex(-500), -1);
    QCOMPARE(parser.activeCue(-500).timestampMs, -1LL);

    // Way past last cue
    QCOMPARE(parser.findActiveCueIndex(5000000), 999);
    QCOMPARE(parser.activeCue(5000000).text, QString("Cue 999"));

    // Stress: 100,000 random queries
    std::mt19937 rng(1337);
    std::uniform_int_distribution<qint64> dist(-1000, 1200000);

    for (int q = 0; q < 100000; ++q) {
        qint64 queryMs = dist(rng);
        int idx = parser.findActiveCueIndex(queryMs);
        if (queryMs < 0) {
            QCOMPARE(idx, -1);
        } else if (queryMs >= 999000) {
            QCOMPARE(idx, 999);
        } else {
            int expected = static_cast<int>(queryMs / 1000);
            QCOMPARE(idx, expected);
        }
    }
}

void TestChallengerM1::testLrcMalformedAndFuzzing()
{
    // Incomplete brackets, unclosed tags, brackets in lyrics
    QString lrc =
        "[00:01.00] Normal line\n"
        "[00:02.00 Unclosed timestamp\n"
        "No opening bracket 00:03.00]\n"
        "[00:04.00] Line with [guitar solo] in text\n"
        "[[00:05.00]] Double bracketed\n"
        "[unrecognized:tag] Unknown header\n"
        "[00:06.00] Line with <00:06.50> enhanced LRC tags\n"
        "\n\n   \t\r\n"
        "[00:07.00] Valid final line\n";

    LrcParser parser;
    bool ok = parser.parse(lrc);
    QVERIFY(ok);

    // Must have successfully extracted valid cues without crashing
    QVERIFY(parser.count() >= 3);
    QCOMPARE(parser.cues()[0].text, QString("Normal line"));
    QVERIFY(parser.cues()[1].text.contains("guitar solo"));

    // Completely garbage / binary-like input
    QString garbage = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>Not LRC</body></html>";
    LrcParser pGarbage;
    QVERIFY(!pGarbage.parse(garbage));
    QCOMPARE(pGarbage.count(), 0);

    // Empty parser index queries
    QCOMPARE(pGarbage.findActiveCueIndex(5000), -1);
    QCOMPARE(pGarbage.activeCue(5000).timestampMs, -1LL);
    QCOMPARE(pGarbage.cueTimestampAt(0), 0LL);
    QCOMPARE(pGarbage.cueTextAt(0), QString());
}

void TestChallengerM1::testLrcUnicodeAndSpecialChars()
{
    QString lrc =
        "[ti: ♪ 触覚的ブルータリズム — Penguin Sound ♫ ]\n"
        "[ar: 🐧 Penguin Crew ⚡ & Co. ]\n"
        "[00:01.00] 触覚的ブルータリズム (Japanese)\n"
        "[00:02.00] Тестирование синхронизации (Russian)\n"
        "[00:03.00] עִבְרִית וְעַרְבִית (RTL text)\n"
        "[00:04.00] 🚀 Special Emojis 🎧 🎛️ 🎚️\n";

    LrcParser parser;
    QVERIFY(parser.parse(lrc));
    QCOMPARE(parser.count(), 4);
    QVERIFY(parser.title().contains("触覚的ブルータリズム"));
    QVERIFY(parser.artist().contains("Penguin Crew"));
    QCOMPARE(parser.cues()[0].text, QString("触覚的ブルータリズム (Japanese)"));
    QCOMPARE(parser.cues()[1].text, QString("Тестирование синхронизации (Russian)"));
    QCOMPARE(parser.cues()[2].text, QString("עִבְרִית וְעַרְבִית (RTL text)"));
    QVERIFY(parser.cues()[3].text.contains("🚀"));
}

void TestChallengerM1::testLrcLargeFilePerformance()
{
    // Stress test with 10,000 lines
    QString lrc;
    lrc.reserve(500000);
    for (int i = 0; i < 10000; ++i) {
        int mm = i / 60;
        int ss = i % 60;
        lrc += QString("[%1:%2.00] Line %3 of large benchmark dataset\n")
                   .arg(mm, 2, 10, QChar('0'))
                   .arg(ss, 2, 10, QChar('0'))
                   .arg(i);
    }

    QElapsedTimer timer;
    timer.start();

    LrcParser parser;
    bool ok = parser.parse(lrc);

    qint64 elapsedMs = timer.elapsed();
    QVERIFY(ok);
    QCOMPARE(parser.count(), 10000);
    qDebug() << "Parsed 10,000 LRC lines in" << elapsedMs << "ms";
    // Must parse comfortably fast (< 500ms)
    QVERIFY(elapsedMs < 500);
}

QTEST_MAIN(TestChallengerM1)
#include "test_challenger_m1.moc"
