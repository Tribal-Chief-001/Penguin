#ifndef VUMETERDSP_H
#define VUMETERDSP_H

#include <QtGlobal>
#include <cstdint>

namespace Penguin {
namespace Core {

constexpr double VU_MIN_DB = -60.0;
constexpr double VU_MAX_DB = 3.0;
constexpr double VU_CLIP_THRESHOLD_DB = 0.0;
constexpr double VU_DECAY_RATE_DB_PER_SEC = 20.0;
constexpr double VU_PEAK_HOLD_DECAY_RATE_DB_PER_SEC = 30.0;
constexpr double VU_PEAK_HOLD_TIME_SEC = 1.0;
constexpr double VU_CLIP_HOLD_TIME_SEC = 1.5;

struct VULevels {
    double leftPeakDb = VU_MIN_DB;
    double rightPeakDb = VU_MIN_DB;
    double leftRmsDb = VU_MIN_DB;
    double rightRmsDb = VU_MIN_DB;
    double leftPeakHoldDb = VU_MIN_DB;
    double rightPeakHoldDb = VU_MIN_DB;
    bool leftClipped = false;
    bool rightClipped = false;

    // Normalized [0.0, 1.0] representations (mapped from -60dB to 0dB)
    double leftPeakNorm = 0.0;
    double rightPeakNorm = 0.0;
    double leftRmsNorm = 0.0;
    double rightRmsNorm = 0.0;
    double leftPeakHoldNorm = 0.0;
    double rightPeakHoldNorm = 0.0;
};

class VUMeterDSP {
public:
    VUMeterDSP();

    // Process raw float stereo audio buffer (interleaved L, R, L, R...)
    void processBuffer(const float *interleavedSamples, int numFrames, int channels = 2);

    // Process separate channel float buffers
    void processChannels(const float *leftSamples, const float *rightSamples, int numSamples);

    // Process int16 PCM buffer
    void processInt16Buffer(const int16_t *interleavedSamples, int numFrames, int channels = 2);

    // Run physics decay tick (e.g. at 60Hz: deltaSeconds = 1.0 / 60.0)
    void updateTick(double deltaSeconds);

    // Provide synthetic/simulated target levels (for testing or idle animation)
    void simulateSignal(double leftTargetDb, double rightTargetDb, double deltaSeconds = 1.0 / 60.0);

    // Reset all levels to minimum
    void reset();

    // Current ballistic levels
    VULevels currentLevels() const { return m_levels; }

    // Utility conversion functions
    static double linearToDb(double linear);
    static double dbToLinear(double db);
    static double dbToNormalized(double db, double minDb = VU_MIN_DB, double maxDb = 0.0);

private:
    void applyBallistics(double rawLeftPeakDb, double rawRightPeakDb,
                         double rawLeftRmsDb, double rawRightRmsDb,
                         double deltaSeconds);
    void updateNormalizedValues();

    VULevels m_levels;
    double m_leftHoldTimer = 0.0;
    double m_rightHoldTimer = 0.0;
    double m_leftClipTimer = 0.0;
    double m_rightClipTimer = 0.0;
};

} // namespace Core
} // namespace Penguin

#endif // VUMETERDSP_H
