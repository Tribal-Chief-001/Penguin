#include <QtTest>
#include <QCoreApplication>
#include <clocale>
#include <cmath>
#include <complex>
#include <vector>
#include <random>

#include "EqualizerDSP.h"
#include "VUMeterDSP.h"
#include "SubtitleLoader.h"
#include "LrcParser.h"
#include "TimecodeFormatter.h"

using namespace Penguin::Core;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class TestChallengerM1_2 : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // =========================================================================
    // 1. EqualizerDSP Empirical Verification
    // =========================================================================
    void testEqualizerTransferFunctionsAllBandsExtremeGains();
    void testEqualizerDirectCoeffs24dBAndStability();
    void testEqualizerPoleLocationsBIBOStability();
    void testEqualizerTimeDomainSteadyStateResponse();
    void testEqualizerCascadedLongStreamStability();
    void testEqualizerAsymptoticDCNyquistResponse();
    void testEqualizerVariousSampleRates();

    // =========================================================================
    // 2. VUMeterDSP Empirical Verification
    // =========================================================================
    void testVUMeterSineRmsAndPeakAccuracy();
    void testVUMeterSilenceUnderflowHandling();
    void testVUMeterDecayRateExactPhysics();
    void testVUMeterPeakHoldDurationAndDecay();
    void testVUMeterClippingFlagAndHoldTimer();
    void testVUMeterInt16PCMConversionPrecision();
    void testVUMeterHighNoiseAndBurstStress();

    // =========================================================================
    // 3. SubtitleLoader Malformed Stress Verification
    // =========================================================================
    void testSubtitleLoaderSrtMalformedTimestamps();
    void testSubtitleLoaderSrtZeroLengthAndInvertedCues();
    void testSubtitleLoaderSrtUnclosedTagsAndFormatting();
    void testSubtitleLoaderVttMalformedAndSettings();
    void testSubtitleLoaderAssMalformedEventsAndTags();
    void testSubtitleLoaderAssCommasInDialogue();
    void testSubtitleLoaderExtremeLargeFileStress();
    void testSubtitleLoaderActiveLookupOverlaps();
};

void TestChallengerM1_2::initTestCase()
{
    setlocale(LC_NUMERIC, "C");
    qDebug() << "=========================================================";
    qDebug() << "Starting Challenger 2 Empirical Stress Test Suite (M1)...";
    qDebug() << "=========================================================";
}

void TestChallengerM1_2::cleanupTestCase()
{
    qDebug() << "=========================================================";
    qDebug() << "Challenger 2 Empirical Stress Test Suite Completed.";
    qDebug() << "=========================================================";
}

// =============================================================================
// 1. EqualizerDSP Tests
// =============================================================================

void TestChallengerM1_2::testEqualizerTransferFunctionsAllBandsExtremeGains()
{
    // Test all 10 ISO standard center frequencies
    // Gains: +12dB, -12dB, +6dB, -6dB, 0dB
    double sampleRate = 48000.0;
    std::vector<double> testGains = {+12.0, -12.0, +6.0, -6.0, 0.0};

    for (int band = 0; band < EQ_NUM_BANDS; ++band) {
        double f0 = EqualizerDSP::bandFrequency(band);

        for (double targetGain : testGains) {
            EqualizerDSP eq;
            eq.setBandGain(band, targetGain);

            double measuredGain = eq.calculateMagnitudeResponse(f0, sampleRate);

            // Verify magnitude response matches targetGain at f0 within tolerance (+- 0.05 dB)
            QVERIFY2(std::abs(measuredGain - targetGain) < 0.05,
                     qPrintable(QString("Band %1 (%2 Hz): Expected %3 dB, got %4 dB (delta=%5 dB)")
                                    .arg(band).arg(f0).arg(targetGain).arg(measuredGain)
                                    .arg(std::abs(measuredGain - targetGain))));
        }
    }
}

void TestChallengerM1_2::testEqualizerDirectCoeffs24dBAndStability()
{
    // Direct computation of peaking filter coefficients for +24dB and -24dB across all ISO frequencies
    double sampleRate = 48000.0;
    std::vector<double> extremeGains = {+24.0, -24.0, +18.0, -18.0};

    for (int band = 0; band < EQ_NUM_BANDS; ++band) {
        double f0 = EqualizerDSP::bandFrequency(band);

        for (double targetGain : extremeGains) {
            BiquadCoeffs c = EqualizerDSP::computePeakingCoeffs(f0, targetGain, sampleRate, 1.41421356);

            // Compute transfer function H(e^{j w0}) analytically
            double w0 = 2.0 * M_PI * f0 / sampleRate;
            std::complex<double> z_inv = std::polar(1.0, -w0);
            std::complex<double> z_inv2 = z_inv * z_inv;

            std::complex<double> num = c.b0 + c.b1 * z_inv + c.b2 * z_inv2;
            std::complex<double> den = 1.0 + c.a1 * z_inv + c.a2 * z_inv2;

            double mag = std::abs(num / den);
            double magDb = 20.0 * std::log10(mag);

            QVERIFY2(std::abs(magDb - targetGain) < 0.01,
                     qPrintable(QString("Direct +24dB/-24dB test at %1 Hz: target=%2 dB, evaluated=%3 dB")
                                    .arg(f0).arg(targetGain).arg(magDb)));

            // Verify coefficients are finite
            QVERIFY(!std::isnan(c.b0) && !std::isinf(c.b0));
            QVERIFY(!std::isnan(c.b1) && !std::isinf(c.b1));
            QVERIFY(!std::isnan(c.b2) && !std::isinf(c.b2));
            QVERIFY(!std::isnan(c.a1) && !std::isinf(c.a1));
            QVERIFY(!std::isnan(c.a2) && !std::isinf(c.a2));
        }
    }
}

void TestChallengerM1_2::testEqualizerPoleLocationsBIBOStability()
{
    // A biquad filter is BIBO stable iff all poles have magnitude |z| < 1.0.
    // Denominator polynomial: D(z) = 1 + a1*z^-1 + a2*z^-2 = 0
    // Equivalently: z^2 + a1*z + a2 = 0
    // Roots: z = (-a1 +- sqrt(a1^2 - 4*a2)) / 2
    double sampleRate = 48000.0;
    std::vector<double> gainsToTest = {-30.0, -24.0, -12.0, -6.0, 0.0, +6.0, +12.0, +24.0, +30.0};

    for (int band = 0; band < EQ_NUM_BANDS; ++band) {
        double f0 = EqualizerDSP::bandFrequency(band);

        for (double g : gainsToTest) {
            BiquadCoeffs c = EqualizerDSP::computePeakingCoeffs(f0, g, sampleRate, 1.41421356);

            // Discriminant
            double disc = c.a1 * c.a1 - 4.0 * c.a2;
            std::complex<double> sqrtDisc = std::sqrt(std::complex<double>(disc, 0.0));

            std::complex<double> pole1 = (-c.a1 + sqrtDisc) / 2.0;
            std::complex<double> pole2 = (-c.a1 - sqrtDisc) / 2.0;

            double poleMag1 = std::abs(pole1);
            double poleMag2 = std::abs(pole2);

            QVERIFY2(poleMag1 < 1.0,
                     qPrintable(QString("Unstable pole 1 at f0=%1 Hz, gain=%2 dB: |pole1| = %3 >= 1.0")
                                    .arg(f0).arg(g).arg(poleMag1)));
            QVERIFY2(poleMag2 < 1.0,
                     qPrintable(QString("Unstable pole 2 at f0=%1 Hz, gain=%2 dB: |pole2| = %3 >= 1.0")
                                    .arg(f0).arg(g).arg(poleMag2)));
        }
    }
}

void TestChallengerM1_2::testEqualizerTimeDomainSteadyStateResponse()
{
    // Feed a synthesized pure sine wave at band center frequency through processSample
    // and measure the steady-state RMS amplitude to verify time-domain amplification matches dB gain.
    double sampleRate = 48000.0;
    double f0 = 1000.0; // 1kHz band (band 5)
    double gainDb = +6.0; // +6dB is a factor of ~ 1.99526

    EqualizerDSP eq;
    eq.setBandGain(5, gainDb);

    int numSamples = 48000; // 1 second of audio
    std::vector<float> input(numSamples);
    std::vector<float> output(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(std::sin(2.0 * M_PI * f0 * i / sampleRate));
        output[i] = eq.processSample(input[i], 0, sampleRate);
    }

    // Measure RMS over the last 20,000 samples (steady state)
    double sumSqIn = 0.0;
    double sumSqOut = 0.0;
    int count = 20000;
    for (int i = numSamples - count; i < numSamples; ++i) {
        sumSqIn += input[i] * input[i];
        sumSqOut += output[i] * output[i];
    }
    double rmsIn = std::sqrt(sumSqIn / count);
    double rmsOut = std::sqrt(sumSqOut / count);

    double measuredGainDb = 20.0 * std::log10(rmsOut / rmsIn);

    QVERIFY2(std::abs(measuredGainDb - gainDb) < 0.1,
             qPrintable(QString("Time domain steady state gain mismatch: expected %1 dB, got %2 dB")
                            .arg(gainDb).arg(measuredGainDb)));
}

void TestChallengerM1_2::testEqualizerCascadedLongStreamStability()
{
    // Feed 500,000 samples of noise, Dirac impulses, and high amplitude signals through all 10 bands at max gain (+12dB)
    EqualizerDSP eq;
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        eq.setBandGain(i, (i % 2 == 0) ? 12.0 : -12.0);
    }

    constexpr int NUM_FRAMES = 500000;
    std::vector<float> buffer(NUM_FRAMES * 2); // Stereo

    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // Initial Dirac impulse
    buffer[0] = 1.0f;
    buffer[1] = 1.0f;

    // Followed by uniform noise
    for (int i = 2; i < NUM_FRAMES * 2; ++i) {
        buffer[i] = dist(rng);
    }

    eq.processInterleavedBuffer(buffer.data(), NUM_FRAMES, 2, 48000.0);

    for (int i = 0; i < NUM_FRAMES * 2; ++i) {
        float s = buffer[i];
        QVERIFY2(!std::isnan(s), qPrintable(QString("NaN encountered at sample index %1").arg(i)));
        QVERIFY2(!std::isinf(s), qPrintable(QString("Inf encountered at sample index %1").arg(i)));
    }
}

void TestChallengerM1_2::testEqualizerAsymptoticDCNyquistResponse()
{
    // For peaking equalizer filters, response at DC (0 Hz) and Nyquist (fs/2) must be exactly 0 dB (1.0).
    double sampleRate = 48000.0;
    EqualizerDSP eq;
    eq.setBandGain(5, 12.0); // 1 kHz band at +12 dB

    double dcGain = eq.calculateMagnitudeResponse(0.001, sampleRate);
    double nyquistGain = eq.calculateMagnitudeResponse(sampleRate / 2.0 - 1.0, sampleRate);

    QVERIFY2(std::abs(dcGain) < 0.01,
             qPrintable(QString("DC response expected ~0 dB, got %1 dB").arg(dcGain)));
    QVERIFY2(std::abs(nyquistGain) < 0.01,
             qPrintable(QString("Nyquist response expected ~0 dB, got %1 dB").arg(nyquistGain)));
}

void TestChallengerM1_2::testEqualizerVariousSampleRates()
{
    // Test biquad calculations across 44.1kHz, 48kHz, 96kHz, 192kHz
    std::vector<double> rates = {44100.0, 48000.0, 88200.0, 96000.0, 192000.0};

    for (double sr : rates) {
        EqualizerDSP eq;
        eq.setBandGain(3, 6.0); // 250 Hz
        // Trigger sample rate sync in DSP engine
        eq.processSample(0.0f, 0, sr);

        double resp = eq.calculateMagnitudeResponse(250.0, sr);
        QVERIFY2(std::abs(resp - 6.0) < 0.1,
                 qPrintable(QString("Sample rate %1 Hz failed: expected 6.0 dB at 250Hz, got %2 dB")
                                .arg(sr).arg(resp)));
    }
}


// =============================================================================
// 2. VUMeterDSP Tests
// =============================================================================

void TestChallengerM1_2::testVUMeterSineRmsAndPeakAccuracy()
{
    VUMeterDSP vu;
    constexpr int NUM_FRAMES = 4800; // 100ms at 48kHz
    std::vector<float> buffer(NUM_FRAMES * 2);

    // Full scale 1.0 amplitude sine wave on Left channel (0 dBFS peak, -3.0103 dBFS RMS)
    // 0.5 amplitude sine wave on Right channel (-6.0206 dBFS peak, -9.0309 dBFS RMS)
    for (int i = 0; i < NUM_FRAMES; ++i) {
        double phase = 2.0 * M_PI * 1000.0 * i / 48000.0;
        buffer[i * 2] = static_cast<float>(1.0 * std::sin(phase));
        buffer[i * 2 + 1] = static_cast<float>(0.5 * std::sin(phase));
    }

    vu.processBuffer(buffer.data(), NUM_FRAMES, 2);
    VULevels levels = vu.currentLevels();

    // Verify Left peak is ~ 0.0 dBFS
    QVERIFY2(std::abs(levels.leftPeakDb - 0.0) < 0.05,
             qPrintable(QString("Left peak: expected 0.0 dB, got %1 dB").arg(levels.leftPeakDb)));
    // Verify Left RMS is ~ -3.01 dBFS
    QVERIFY2(std::abs(levels.leftRmsDb - (-3.0103)) < 0.1,
             qPrintable(QString("Left RMS: expected -3.01 dB, got %1 dB").arg(levels.leftRmsDb)));

    // Verify Right peak is ~ -6.02 dBFS
    QVERIFY2(std::abs(levels.rightPeakDb - (-6.0206)) < 0.1,
             qPrintable(QString("Right peak: expected -6.02 dB, got %1 dB").arg(levels.rightPeakDb)));
    // Verify Right RMS is ~ -9.03 dBFS
    QVERIFY2(std::abs(levels.rightRmsDb - (-9.0309)) < 0.1,
             qPrintable(QString("Right RMS: expected -9.03 dB, got %1 dB").arg(levels.rightRmsDb)));
}

void TestChallengerM1_2::testVUMeterSilenceUnderflowHandling()
{
    VUMeterDSP vu;
    constexpr int NUM_FRAMES = 1024;
    std::vector<float> silence(NUM_FRAMES * 2, 0.0f);

    vu.processBuffer(silence.data(), NUM_FRAMES, 2);
    VULevels levels = vu.currentLevels();

    QCOMPARE(levels.leftPeakDb, VU_MIN_DB);
    QCOMPARE(levels.rightPeakDb, VU_MIN_DB);
    QCOMPARE(levels.leftRmsDb, VU_MIN_DB);
    QCOMPARE(levels.rightRmsDb, VU_MIN_DB);
    QCOMPARE(levels.leftPeakNorm, 0.0);
    QCOMPARE(levels.rightPeakNorm, 0.0);
    QVERIFY(!levels.leftClipped);
    QVERIFY(!levels.rightClipped);
}

void TestChallengerM1_2::testVUMeterDecayRateExactPhysics()
{
    VUMeterDSP vu;
    // Set level to 0.0 dBFS
    vu.simulateSignal(0.0, 0.0, 0.0);
    QCOMPARE(vu.currentLevels().leftPeakDb, 0.0);

    // VU_DECAY_RATE_DB_PER_SEC = 20.0 dB/s
    // Step 0.1s -> 2.0 dB drop -> -2.0 dB
    vu.updateTick(0.1);
    QVERIFY2(std::abs(vu.currentLevels().leftPeakDb - (-2.0)) < 0.01,
             qPrintable(QString("Decay after 0.1s: expected -2.0 dB, got %1 dB").arg(vu.currentLevels().leftPeakDb)));

    // Step 0.4s -> additional 8.0 dB drop -> -10.0 dB
    vu.updateTick(0.4);
    QVERIFY2(std::abs(vu.currentLevels().leftPeakDb - (-10.0)) < 0.01,
             qPrintable(QString("Decay after 0.5s total: expected -10.0 dB, got %1 dB").arg(vu.currentLevels().leftPeakDb)));

    // Step 2.5s -> additional 50 dB drop -> -60.0 dB (clamped at VU_MIN_DB)
    vu.updateTick(2.5);
    QCOMPARE(vu.currentLevels().leftPeakDb, VU_MIN_DB);
}

void TestChallengerM1_2::testVUMeterPeakHoldDurationAndDecay()
{
    VUMeterDSP vu;
    vu.simulateSignal(0.0, 0.0, 0.0);
    QCOMPARE(vu.currentLevels().leftPeakHoldDb, 0.0);

    // Hold time is 1.0s. During this time, peak hold must remain locked at 0.0 dBFS.
    vu.updateTick(0.3);
    QCOMPARE(vu.currentLevels().leftPeakHoldDb, 0.0);

    vu.updateTick(0.5); // Total 0.8s
    QCOMPARE(vu.currentLevels().leftPeakHoldDb, 0.0);

    // Now cross the 1.0s threshold with a 0.5s step (total 1.3s, 0.3s after hold expiration)
    // Peak hold decay rate = 30.0 dB/s -> 0.3s * 30 dB/s = 9.0 dB drop -> -9.0 dB
    vu.updateTick(0.5);
    QVERIFY2(std::abs(vu.currentLevels().leftPeakHoldDb - (-9.0)) < 0.01,
             qPrintable(QString("Peak hold after 1.3s: expected -9.0 dB, got %1 dB").arg(vu.currentLevels().leftPeakHoldDb)));
}

void TestChallengerM1_2::testVUMeterClippingFlagAndHoldTimer()
{
    VUMeterDSP vu;
    // Input signal above 0 dBFS (+2.0 dBFS)
    vu.simulateSignal(2.0, 2.0, 0.0);
    VULevels l = vu.currentLevels();
    QVERIFY(l.leftClipped);
    QVERIFY(l.rightClipped);

    // Clip hold time is 1.5s. It must stay clipped for 1.4s even if subsequent signals are low.
    vu.updateTick(0.5);
    QVERIFY(vu.currentLevels().leftClipped);
    vu.updateTick(0.9); // Total 1.4s
    QVERIFY(vu.currentLevels().leftClipped);

    // Cross 1.5s threshold (additional 0.2s -> total 1.6s) -> must reset clipping
    vu.updateTick(0.2);
    QVERIFY(!vu.currentLevels().leftClipped);
    QVERIFY(!vu.currentLevels().rightClipped);
}

void TestChallengerM1_2::testVUMeterInt16PCMConversionPrecision()
{
    VUMeterDSP vu;
    constexpr int NUM_FRAMES = 1024;
    std::vector<int16_t> pcm(NUM_FRAMES * 2);

    // Max 16-bit positive amplitude 32767 (~ 0 dBFS)
    for (int i = 0; i < NUM_FRAMES; ++i) {
        pcm[i * 2] = 32767;
        pcm[i * 2 + 1] = 16384; // Half scale (-6.02 dBFS)
    }

    vu.processInt16Buffer(pcm.data(), NUM_FRAMES, 2);
    VULevels l = vu.currentLevels();

    QVERIFY2(std::abs(l.leftPeakDb - 0.0) < 0.05,
             qPrintable(QString("Int16 Left Peak: expected ~0 dB, got %1 dB").arg(l.leftPeakDb)));
    QVERIFY2(std::abs(l.rightPeakDb - (-6.02)) < 0.1,
             qPrintable(QString("Int16 Right Peak: expected ~ -6.02 dB, got %1 dB").arg(l.rightPeakDb)));
}

void TestChallengerM1_2::testVUMeterHighNoiseAndBurstStress()
{
    VUMeterDSP vu;
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-2.0f, 2.0f); // Over-scale noise

    constexpr int CHUNK = 256;
    std::vector<float> noise(CHUNK * 2);

    for (int iter = 0; iter < 1000; ++iter) {
        for (int i = 0; i < CHUNK * 2; ++i) {
            noise[i] = dist(rng);
        }
        vu.processBuffer(noise.data(), CHUNK, 2);
        vu.updateTick(1.0 / 60.0);

        VULevels l = vu.currentLevels();
        QVERIFY(!std::isnan(l.leftPeakDb) && !std::isinf(l.leftPeakDb));
        QVERIFY(!std::isnan(l.rightPeakDb) && !std::isinf(l.rightPeakDb));
        QVERIFY(!std::isnan(l.leftPeakNorm) && !std::isinf(l.leftPeakNorm));
        QVERIFY(l.leftPeakNorm >= 0.0 && l.leftPeakNorm <= 1.0);
    }
}

// =============================================================================
// 3. SubtitleLoader Malformed Stress Tests
// =============================================================================

void TestChallengerM1_2::testSubtitleLoaderSrtMalformedTimestamps()
{
    // Test SRT with corrupt timestamps: non-digits, incomplete fields, invalid syntax
    QString malformedSrt =
        "1\n"
        "NOT_A_TIMESTAMP --> STILL_NOT\n"
        "Bad cue 1\n\n"
        "2\n"
        "00:00:01,000 --> 00:00:04,000\n"
        "Valid cue 2\n\n"
        "3\n"
        "99:99:99,999 -> 99:99:99,999\n" // Missing hyphen in arrow
        "Bad cue 3\n\n"
        "4\n"
        "00:00:05.500 --> 00:00:08.500\n" // Dot separator instead of comma
        "Valid cue 4\n";

    SubtitleLoader loader;
    bool ok = loader.parseSrt(malformedSrt);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 2);
    QCOMPARE(loader.cues()[0].plainText, QString("Valid cue 2"));
    QCOMPARE(loader.cues()[1].plainText, QString("Valid cue 4"));
}

void TestChallengerM1_2::testSubtitleLoaderSrtZeroLengthAndInvertedCues()
{
    // Zero-length and inverted (end <= start) cues should be safely discarded
    QString srt =
        "1\n"
        "00:00:05,000 --> 00:00:05,000\n" // Zero duration
        "Zero length cue\n\n"
        "2\n"
        "00:00:10,000 --> 00:00:04,000\n" // Inverted
        "Inverted time cue\n\n"
        "3\n"
        "00:00:12,000 --> 00:00:15,000\n"
        "Legitimate cue\n";

    SubtitleLoader loader;
    bool ok = loader.parseSrt(srt);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 1);
    QCOMPARE(loader.cues()[0].plainText, QString("Legitimate cue"));
}

void TestChallengerM1_2::testSubtitleLoaderSrtUnclosedTagsAndFormatting()
{
    QString srt =
        "1\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "<b><i>Unclosed HTML tags\n\n"
        "2\n"
        "00:00:04,000 --> 00:00:06,000\n"
        "<font color=\"#FF4400\" size=\"+2\">Brutalist Styling</font>\n\n"
        "3\n"
        "00:00:07,000 --> 00:00:09,000\n"
        "{\\b1\\c&H00FFFF&}ASS tags inside SRT{\\b0}\n";

    SubtitleLoader loader;
    bool ok = loader.parseSrt(srt);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 3);

    QCOMPARE(loader.cues()[0].plainText, QString("Unclosed HTML tags"));
    QCOMPARE(loader.cues()[1].plainText, QString("Brutalist Styling"));
    QCOMPARE(loader.cues()[2].plainText, QString("ASS tags inside SRT"));
}

void TestChallengerM1_2::testSubtitleLoaderVttMalformedAndSettings()
{
    // WebVTT with cue settings (position, line, align) and short timestamps
    QString vtt =
        "WEBVTT - Penguin Test Suite\n\n"
        "NOTE This is a comment block\n\n"
        "cue-header-id-01\n"
        "00:02.000 --> 00:05.000 position:10%,line:90% align:start\n"
        "<v Speaker1>Line with cue payload and settings</v>\n\n"
        "00:06.500 --> 00:09.000\n"
        "<b>Bold</b> and <i>Italic</i> WebVTT\n";

    SubtitleLoader loader;
    bool ok = loader.parseVtt(vtt);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 2);
    QCOMPARE(loader.cues()[0].startMs, 2000LL);
    QCOMPARE(loader.cues()[0].endMs, 5000LL);
    QCOMPARE(loader.cues()[0].plainText, QString("Line with cue payload and settings"));
    QCOMPARE(loader.cues()[1].plainText, QString("Bold and Italic WebVTT"));
}

void TestChallengerM1_2::testSubtitleLoaderAssMalformedEventsAndTags()
{
    QString ass =
        "[Script Info]\n"
        "Title: Malformed ASS Test\n\n"
        "[Events]\n"
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
        "Dialogue: 0,0:00:01.00,0:00:04.00,Default,,0,0,0,,{\\pos(960,540)\\an5\\fad(100,100)}Line with tags\\Nand hard break\\nsecond line\n"
        "Dialogue: 0,0:00:05.00,0:00:02.00,Default,,0,0,0,,Inverted timestamp (drop me)\n"
        "Dialogue: 0,0:00:06.00,0:00:08.50,Default,,0,0,0,,{\\unclosedASS bracket text\n";

    SubtitleLoader loader;
    bool ok = loader.parseAss(ass);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 2);

    // Check line break replacement and ASS tag stripping
    QVERIFY(loader.cues()[0].plainText.contains("\nand hard break"));
    QCOMPARE(loader.cues()[1].startMs, 6000LL);
}

void TestChallengerM1_2::testSubtitleLoaderAssCommasInDialogue()
{
    // Crucial ASS test: Dialogue text containing commas must NOT be truncated
    QString ass =
        "[Events]\n"
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
        "Dialogue: 0,0:00:01.00,0:00:05.00,Default,,0,0,0,,First clause, second clause, and third clause, with commas!\n";

    SubtitleLoader loader;
    bool ok = loader.parseAss(ass);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 1);
    QCOMPARE(loader.cues()[0].plainText, QString("First clause, second clause, and third clause, with commas!"));
}

void TestChallengerM1_2::testSubtitleLoaderExtremeLargeFileStress()
{
    // Synthesize 5,000 subtitle cues with standard HH:MM:SS,mmm formatting
    QString largeSrt;
    largeSrt.reserve(5000 * 90);
    for (int i = 1; i <= 5000; ++i) {
        int totalSec = i * 2;
        int hh = totalSec / 3600;
        int mm = (totalSec % 3600) / 60;
        int ss = totalSec % 60;

        int endSec = totalSec + 1;
        int endHh = endSec / 3600;
        int endMm = (endSec % 3600) / 60;
        int endSs = endSec % 60;

        largeSrt.append(QString("%1\n%2:%3:%4,000 --> %5:%6:%7,500\nSubtitle line %1 with <i>tags</i>\n\n")
                            .arg(i)
                            .arg(hh, 2, 10, QChar('0'))
                            .arg(mm, 2, 10, QChar('0'))
                            .arg(ss, 2, 10, QChar('0'))
                            .arg(endHh, 2, 10, QChar('0'))
                            .arg(endMm, 2, 10, QChar('0'))
                            .arg(endSs, 2, 10, QChar('0')));
    }

    SubtitleLoader loader;
    bool ok = loader.parseSrt(largeSrt);
    QVERIFY(ok);
    QCOMPARE(loader.cueCount(), 5000);
}


void TestChallengerM1_2::testSubtitleLoaderActiveLookupOverlaps()
{
    // Test overlapping subtitle cues lookup
    QString srt =
        "1\n00:00:01,000 --> 00:00:05,000\nTop subtitle\n\n"
        "2\n00:00:02,000 --> 00:00:06,000\nBottom subtitle\n\n"
        "3\n00:00:08,000 --> 00:00:10,000\nLater subtitle\n";

    SubtitleLoader loader;
    loader.parseSrt(srt);

    // At 1500ms: only cue 1
    QCOMPARE(loader.activeSubtitleText(1500), QString("Top subtitle"));

    // At 3000ms: both cue 1 and cue 2 overlap
    QList<SubtitleCue> active = loader.activeCues(3000);
    QCOMPARE(active.size(), 2);
    QCOMPARE(active[0].plainText, QString("Top subtitle"));
    QCOMPARE(active[1].plainText, QString("Bottom subtitle"));

    // At 5500ms: only cue 2
    QCOMPARE(loader.activeSubtitleText(5500), QString("Bottom subtitle"));

    // At 7000ms: none
    QVERIFY(loader.activeSubtitleText(7000).isEmpty());

    // At 8500ms: cue 3
    QCOMPARE(loader.activeSubtitleText(8500), QString("Later subtitle"));
}

QTEST_MAIN(TestChallengerM1_2)
#include "test_challenger_m1_2.moc"
