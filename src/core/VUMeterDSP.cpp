#include "VUMeterDSP.h"
#include <cmath>
#include <algorithm>

namespace Penguin {
namespace Core {

VUMeterDSP::VUMeterDSP()
{
    reset();
}

void VUMeterDSP::reset()
{
    m_levels.leftPeakDb = VU_MIN_DB;
    m_levels.rightPeakDb = VU_MIN_DB;
    m_levels.leftRmsDb = VU_MIN_DB;
    m_levels.rightRmsDb = VU_MIN_DB;
    m_levels.leftPeakHoldDb = VU_MIN_DB;
    m_levels.rightPeakHoldDb = VU_MIN_DB;
    m_levels.leftClipped = false;
    m_levels.rightClipped = false;

    m_leftHoldTimer = 0.0;
    m_rightHoldTimer = 0.0;
    m_leftClipTimer = 0.0;
    m_rightClipTimer = 0.0;

    updateNormalizedValues();
}

double VUMeterDSP::linearToDb(double linear)
{
    if (linear <= 1e-6) return VU_MIN_DB;
    double db = 20.0 * std::log10(linear);
    return std::clamp(db, VU_MIN_DB, VU_MAX_DB);
}

double VUMeterDSP::dbToLinear(double db)
{
    return std::pow(10.0, db / 20.0);
}

double VUMeterDSP::dbToNormalized(double db, double minDb, double maxDb)
{
    if (maxDb <= minDb) return 0.0;
    double clamped = std::clamp(db, minDb, maxDb);
    return (clamped - minDb) / (maxDb - minDb);
}

void VUMeterDSP::updateNormalizedValues()
{
    m_levels.leftPeakNorm = dbToNormalized(m_levels.leftPeakDb);
    m_levels.rightPeakNorm = dbToNormalized(m_levels.rightPeakDb);
    m_levels.leftRmsNorm = dbToNormalized(m_levels.leftRmsDb);
    m_levels.rightRmsNorm = dbToNormalized(m_levels.rightRmsDb);
    m_levels.leftPeakHoldNorm = dbToNormalized(m_levels.leftPeakHoldDb);
    m_levels.rightPeakHoldNorm = dbToNormalized(m_levels.rightPeakHoldDb);
}

void VUMeterDSP::processChannels(const float *leftSamples, const float *rightSamples, int numSamples)
{
    if (numSamples <= 0) return;

    double maxLeft = 0.0;
    double maxRight = 0.0;
    double sumSqLeft = 0.0;
    double sumSqRight = 0.0;

    for (int i = 0; i < numSamples; ++i) {
        double l = leftSamples ? std::abs(static_cast<double>(leftSamples[i])) : 0.0;
        double r = rightSamples ? std::abs(static_cast<double>(rightSamples[i])) : 0.0;

        if (l > maxLeft) maxLeft = l;
        if (r > maxRight) maxRight = r;

        sumSqLeft += l * l;
        sumSqRight += r * r;
    }

    double rmsLeft = std::sqrt(sumSqLeft / numSamples);
    double rmsRight = std::sqrt(sumSqRight / numSamples);

    double rawLeftPeakDb = linearToDb(maxLeft);
    double rawRightPeakDb = linearToDb(maxRight);
    double rawLeftRmsDb = linearToDb(rmsLeft);
    double rawRightRmsDb = linearToDb(rmsRight);

    applyBallistics(rawLeftPeakDb, rawRightPeakDb, rawLeftRmsDb, rawRightRmsDb, 0.0);
}

void VUMeterDSP::processBuffer(const float *interleavedSamples, int numFrames, int channels)
{
    if (!interleavedSamples || numFrames <= 0 || channels <= 0) return;

    double maxLeft = 0.0;
    double maxRight = 0.0;
    double sumSqLeft = 0.0;
    double sumSqRight = 0.0;

    for (int i = 0; i < numFrames; ++i) {
        double l = std::abs(static_cast<double>(interleavedSamples[i * channels]));
        double r = (channels > 1) ? std::abs(static_cast<double>(interleavedSamples[i * channels + 1])) : l;

        if (l > maxLeft) maxLeft = l;
        if (r > maxRight) maxRight = r;

        sumSqLeft += l * l;
        sumSqRight += r * r;
    }

    double rmsLeft = std::sqrt(sumSqLeft / numFrames);
    double rmsRight = std::sqrt(sumSqRight / numFrames);

    double rawLeftPeakDb = linearToDb(maxLeft);
    double rawRightPeakDb = linearToDb(maxRight);
    double rawLeftRmsDb = linearToDb(rmsLeft);
    double rawRightRmsDb = linearToDb(rmsRight);

    applyBallistics(rawLeftPeakDb, rawRightPeakDb, rawLeftRmsDb, rawRightRmsDb, 0.0);
}

void VUMeterDSP::processInt16Buffer(const int16_t *interleavedSamples, int numFrames, int channels)
{
    if (!interleavedSamples || numFrames <= 0 || channels <= 0) return;

    double maxLeft = 0.0;
    double maxRight = 0.0;
    double sumSqLeft = 0.0;
    double sumSqRight = 0.0;

    for (int i = 0; i < numFrames; ++i) {
        double l = std::abs(static_cast<double>(interleavedSamples[i * channels])) / 32768.0;
        double r = (channels > 1) ? std::abs(static_cast<double>(interleavedSamples[i * channels + 1])) / 32768.0 : l;

        if (l > maxLeft) maxLeft = l;
        if (r > maxRight) maxRight = r;

        sumSqLeft += l * l;
        sumSqRight += r * r;
    }

    double rmsLeft = std::sqrt(sumSqLeft / numFrames);
    double rmsRight = std::sqrt(sumSqRight / numFrames);

    double rawLeftPeakDb = linearToDb(maxLeft);
    double rawRightPeakDb = linearToDb(maxRight);
    double rawLeftRmsDb = linearToDb(rmsLeft);
    double rawRightRmsDb = linearToDb(rmsRight);

    applyBallistics(rawLeftPeakDb, rawRightPeakDb, rawLeftRmsDb, rawRightRmsDb, 0.0);
}

void VUMeterDSP::applyBallistics(double rawLeftPeakDb, double rawRightPeakDb,
                                double rawLeftRmsDb, double rawRightRmsDb,
                                double deltaSeconds)
{
    // Attack is instantaneous
    if (rawLeftPeakDb >= m_levels.leftPeakDb) {
        m_levels.leftPeakDb = rawLeftPeakDb;
    } else if (deltaSeconds > 0.0) {
        m_levels.leftPeakDb = std::max(rawLeftPeakDb, m_levels.leftPeakDb - VU_DECAY_RATE_DB_PER_SEC * deltaSeconds);
    }

    if (rawRightPeakDb >= m_levels.rightPeakDb) {
        m_levels.rightPeakDb = rawRightPeakDb;
    } else if (deltaSeconds > 0.0) {
        m_levels.rightPeakDb = std::max(rawRightPeakDb, m_levels.rightPeakDb - VU_DECAY_RATE_DB_PER_SEC * deltaSeconds);
    }

    // RMS attack / decay
    if (rawLeftRmsDb >= m_levels.leftRmsDb) {
        m_levels.leftRmsDb = rawLeftRmsDb;
    } else if (deltaSeconds > 0.0) {
        m_levels.leftRmsDb = std::max(rawLeftRmsDb, m_levels.leftRmsDb - (VU_DECAY_RATE_DB_PER_SEC * 1.5) * deltaSeconds);
    }

    if (rawRightRmsDb >= m_levels.rightRmsDb) {
        m_levels.rightRmsDb = rawRightRmsDb;
    } else if (deltaSeconds > 0.0) {
        m_levels.rightRmsDb = std::max(rawRightRmsDb, m_levels.rightRmsDb - (VU_DECAY_RATE_DB_PER_SEC * 1.5) * deltaSeconds);
    }

    // Peak-Hold Ballistics
    if (m_levels.leftPeakDb >= m_levels.leftPeakHoldDb) {
        m_levels.leftPeakHoldDb = m_levels.leftPeakDb;
        m_leftHoldTimer = VU_PEAK_HOLD_TIME_SEC;
    } else if (deltaSeconds > 0.0) {
        if (m_leftHoldTimer > 0.0) {
            if (deltaSeconds >= m_leftHoldTimer) {
                double remainingDt = deltaSeconds - m_leftHoldTimer;
                m_leftHoldTimer = 0.0;
                m_levels.leftPeakHoldDb = std::max(VU_MIN_DB, m_levels.leftPeakHoldDb - VU_PEAK_HOLD_DECAY_RATE_DB_PER_SEC * remainingDt);
            } else {
                m_leftHoldTimer -= deltaSeconds;
            }
        } else {
            m_levels.leftPeakHoldDb = std::max(VU_MIN_DB, m_levels.leftPeakHoldDb - VU_PEAK_HOLD_DECAY_RATE_DB_PER_SEC * deltaSeconds);
        }
    }

    if (m_levels.rightPeakDb >= m_levels.rightPeakHoldDb) {
        m_levels.rightPeakHoldDb = m_levels.rightPeakDb;
        m_rightHoldTimer = VU_PEAK_HOLD_TIME_SEC;
    } else if (deltaSeconds > 0.0) {
        if (m_rightHoldTimer > 0.0) {
            if (deltaSeconds >= m_rightHoldTimer) {
                double remainingDt = deltaSeconds - m_rightHoldTimer;
                m_rightHoldTimer = 0.0;
                m_levels.rightPeakHoldDb = std::max(VU_MIN_DB, m_levels.rightPeakHoldDb - VU_PEAK_HOLD_DECAY_RATE_DB_PER_SEC * remainingDt);
            } else {
                m_rightHoldTimer -= deltaSeconds;
            }
        } else {
            m_levels.rightPeakHoldDb = std::max(VU_MIN_DB, m_levels.rightPeakHoldDb - VU_PEAK_HOLD_DECAY_RATE_DB_PER_SEC * deltaSeconds);
        }
    }

    // Clipping detection
    if (m_levels.leftPeakDb >= VU_CLIP_THRESHOLD_DB) {
        m_levels.leftClipped = true;
        m_leftClipTimer = VU_CLIP_HOLD_TIME_SEC;
    } else if (deltaSeconds > 0.0) {
        if (m_leftClipTimer > 0.0) {
            if (deltaSeconds >= m_leftClipTimer) {
                m_leftClipTimer = 0.0;
                m_levels.leftClipped = (m_levels.leftPeakDb >= VU_CLIP_THRESHOLD_DB);
            } else {
                m_leftClipTimer -= deltaSeconds;
            }
        } else {
            m_levels.leftClipped = (m_levels.leftPeakDb >= VU_CLIP_THRESHOLD_DB);
        }
    }

    if (m_levels.rightPeakDb >= VU_CLIP_THRESHOLD_DB) {
        m_levels.rightClipped = true;
        m_rightClipTimer = VU_CLIP_HOLD_TIME_SEC;
    } else if (deltaSeconds > 0.0) {
        if (m_rightClipTimer > 0.0) {
            if (deltaSeconds >= m_rightClipTimer) {
                m_rightClipTimer = 0.0;
                m_levels.rightClipped = (m_levels.rightPeakDb >= VU_CLIP_THRESHOLD_DB);
            } else {
                m_rightClipTimer -= deltaSeconds;
            }
        } else {
            m_levels.rightClipped = (m_levels.rightPeakDb >= VU_CLIP_THRESHOLD_DB);
        }
    }

    updateNormalizedValues();
}

void VUMeterDSP::updateTick(double deltaSeconds)
{
    if (deltaSeconds <= 0.0) return;
    applyBallistics(VU_MIN_DB, VU_MIN_DB, VU_MIN_DB, VU_MIN_DB, deltaSeconds);
}

void VUMeterDSP::simulateSignal(double leftTargetDb, double rightTargetDb, double deltaSeconds)
{
    double left = std::clamp(leftTargetDb, VU_MIN_DB, VU_MAX_DB);
    double right = std::clamp(rightTargetDb, VU_MIN_DB, VU_MAX_DB);
    double leftRms = std::max(VU_MIN_DB, left - 3.0);
    double rightRms = std::max(VU_MIN_DB, right - 3.0);

    applyBallistics(left, right, leftRms, rightRms, deltaSeconds);
}

} // namespace Core
} // namespace Penguin
