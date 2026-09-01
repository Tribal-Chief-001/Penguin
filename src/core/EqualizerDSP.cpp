#include "EqualizerDSP.h"
#include <cmath>
#include <algorithm>
#include <QJsonArray>

namespace Penguin {
namespace Core {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

EqualizerDSP::EqualizerDSP()
    : m_currentPreset("Flat")
{
    m_gains.fill(0.0);
    resetDSPStates();
    updateBiquadCoeffs(48000.0);
}

void EqualizerDSP::resetDSPStates()
{
    for (int ch = 0; ch < MAX_CHANNELS; ++ch) {
        for (int band = 0; band < EQ_NUM_BANDS; ++band) {
            m_states[ch][band].reset();
        }
    }
}

double EqualizerDSP::bandFrequency(int bandIndex)
{
    if (bandIndex >= 0 && bandIndex < EQ_NUM_BANDS) {
        return EQ_BAND_FREQUENCIES[bandIndex];
    }
    return 1000.0;
}

QString EqualizerDSP::bandLabel(int bandIndex)
{
    if (bandIndex < 0 || bandIndex >= EQ_NUM_BANDS) return "";
    double freq = EQ_BAND_FREQUENCIES[bandIndex];
    if (freq >= 1000.0) {
        return QString("%1k").arg(static_cast<int>(freq / 1000.0));
    } else if (freq < 100.0) {
        return QString::number(static_cast<int>(std::round(freq)));
    } else {
        return QString::number(static_cast<int>(freq));
    }
}

void EqualizerDSP::setBandGain(int bandIndex, double gainDb)
{
    if (bandIndex >= 0 && bandIndex < EQ_NUM_BANDS) {
        double clamped = std::clamp(gainDb, EQ_MIN_GAIN_DB, EQ_MAX_GAIN_DB);
        if (std::abs(m_gains[bandIndex] - clamped) > 0.001) {
            m_gains[bandIndex] = clamped;
            m_currentPreset = "Custom";
            updateBiquadCoeffs(m_lastSampleRate);
        }
    }
}

double EqualizerDSP::bandGain(int bandIndex) const
{
    if (bandIndex >= 0 && bandIndex < EQ_NUM_BANDS) {
        return m_gains[bandIndex];
    }
    return 0.0;
}

void EqualizerDSP::setAllGains(const QVector<double> &gains)
{
    int count = std::min(static_cast<int>(gains.size()), EQ_NUM_BANDS);
    for (int i = 0; i < count; ++i) {
        m_gains[i] = std::clamp(gains[i], EQ_MIN_GAIN_DB, EQ_MAX_GAIN_DB);
    }
    for (int i = count; i < EQ_NUM_BANDS; ++i) {
        m_gains[i] = 0.0;
    }
    m_currentPreset = "Custom";
    updateBiquadCoeffs(m_lastSampleRate);
}

QVector<double> EqualizerDSP::allGains() const
{
    QVector<double> list;
    list.reserve(EQ_NUM_BANDS);
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        list.append(m_gains[i]);
    }
    return list;
}

void EqualizerDSP::resetFlat()
{
    m_gains.fill(0.0);
    m_currentPreset = "Flat";
    updateBiquadCoeffs(m_lastSampleRate);
}

bool EqualizerDSP::isFlat() const
{
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        if (std::abs(m_gains[i]) > 0.01) return false;
    }
    return true;
}

QStringList EqualizerDSP::availablePresets()
{
    return QStringList{
        "Flat",
        "Rock",
        "Pop",
        "Jazz",
        "Electronic",
        "Vocal Boost",
        "Bass Boost",
        "Treble Boost",
        "Classical",
        "Acoustic"
    };
}

QVector<double> EqualizerDSP::presetGains(const QString &presetName)
{
    QString name = presetName.trimmed().toLower();
    if (name == "flat") {
        return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    } else if (name == "rock") {
        return {4.5, 3.5, 2.0, 0.0, -1.5, -1.0, 1.0, 2.5, 4.0, 4.5};
    } else if (name == "pop") {
        return {-1.5, 1.0, 3.0, 4.0, 3.0, 0.0, -1.0, -1.5, 1.0, 2.0};
    } else if (name == "jazz") {
        return {3.0, 2.0, 1.0, 1.5, -1.5, -1.5, 0.0, 1.5, 2.5, 3.5};
    } else if (name == "electronic") {
        return {4.0, 3.5, 1.0, 0.0, -1.5, 2.0, 1.0, 2.5, 4.0, 4.0};
    } else if (name == "vocal boost" || name == "vocal") {
        return {1.0, -1.0, -2.0, 2.0, 4.5, 4.5, 3.0, 1.0, 0.0, -1.0};
    } else if (name == "bass boost" || name == "bass") {
        return {6.0, 5.0, 4.0, 2.5, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    } else if (name == "treble boost" || name == "treble") {
        return {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.5, 4.5, 6.0, 7.0};
    } else if (name == "classical") {
        return {4.0, 3.0, 2.0, 1.5, -1.0, -1.0, 0.0, 2.0, 3.0, 3.5};
    } else if (name == "acoustic") {
        return {3.5, 2.5, 1.5, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 2.0};
    }
    return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

void EqualizerDSP::setPreset(const QString &presetName)
{
    QVector<double> gains = presetGains(presetName);
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        m_gains[i] = gains[i];
    }
    m_currentPreset = presetName;
    updateBiquadCoeffs(m_lastSampleRate);
}

BiquadCoeffs EqualizerDSP::computePeakingCoeffs(double centerFreqHz, double gainDb, double sampleRate, double q)
{
    BiquadCoeffs c;
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    if (q <= 0.0) q = 1.414;

    // Robert Bristow-Johnson Audio EQ Cookbook Peaking Filter
    double A = std::pow(10.0, gainDb / 40.0);
    double w0 = 2.0 * M_PI * centerFreqHz / sampleRate;
    double alpha = std::sin(w0) / (2.0 * q);
    double cosW0 = std::cos(w0);

    double b0 = 1.0 + alpha * A;
    double b1 = -2.0 * cosW0;
    double b2 = 1.0 - alpha * A;
    double a0 = 1.0 + alpha / A;
    double a1 = -2.0 * cosW0;
    double a2 = 1.0 - alpha / A;

    // Normalize by a0
    c.b0 = b0 / a0;
    c.b1 = b1 / a0;
    c.b2 = b2 / a0;
    c.a1 = a1 / a0;
    c.a2 = a2 / a0;

    return c;
}

void EqualizerDSP::updateBiquadCoeffs(double sampleRate)
{
    m_lastSampleRate = sampleRate;
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        m_coeffs[i] = computePeakingCoeffs(EQ_BAND_FREQUENCIES[i], m_gains[i], sampleRate, 1.414);
    }
}

double EqualizerDSP::calculateMagnitudeResponse(double freqHz, double sampleRate) const
{
    if (sampleRate <= 0.0) sampleRate = 48000.0;
    double w = 2.0 * M_PI * freqHz / sampleRate;
    double cosW = std::cos(w);
    double cos2W = std::cos(2.0 * w);
    double sinW = std::sin(w);
    double sin2W = std::sin(2.0 * w);

    double totalDb = 0.0;

    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        const auto &c = m_coeffs[i];
        // Numerator = b0 + b1 e^-jw + b2 e^-2jw
        double numRe = c.b0 + c.b1 * cosW + c.b2 * cos2W;
        double numIm = -c.b1 * sinW - c.b2 * sin2W;
        // Denominator = 1 + a1 e^-jw + a2 e^-2jw
        double denRe = 1.0 + c.a1 * cosW + c.a2 * cos2W;
        double denIm = -c.a1 * sinW - c.a2 * sin2W;

        double numMagSq = numRe * numRe + numIm * numIm;
        double denMagSq = denRe * denRe + denIm * denIm;

        if (denMagSq > 1e-12) {
            double hMagSq = numMagSq / denMagSq;
            totalDb += 10.0 * std::log10(std::max(1e-12, hMagSq));
        }
    }

    return totalDb;
}

float EqualizerDSP::processSample(float sample, int channel, double sampleRate)
{
    if (channel < 0 || channel >= MAX_CHANNELS) return sample;
    if (sampleRate != m_lastSampleRate) {
        updateBiquadCoeffs(sampleRate);
    }

    double y = sample;
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        if (std::abs(m_gains[i]) < 0.001) continue; // Bypass flat band for performance

        const auto &c = m_coeffs[i];
        auto &s = m_states[channel][i];

        double out = c.b0 * y + c.b1 * s.x1 + c.b2 * s.x2 - c.a1 * s.y1 - c.a2 * s.y2;
        s.x2 = s.x1;
        s.x1 = y;
        s.y2 = s.y1;
        s.y1 = out;

        y = out;
    }

    return static_cast<float>(y);
}

void EqualizerDSP::processInterleavedBuffer(float *buffer, int numFrames, int numChannels, double sampleRate)
{
    if (!buffer || numFrames <= 0 || numChannels <= 0) return;
    int channels = std::min(numChannels, MAX_CHANNELS);

    for (int frame = 0; frame < numFrames; ++frame) {
        for (int ch = 0; ch < channels; ++ch) {
            int idx = frame * numChannels + ch;
            buffer[idx] = processSample(buffer[idx], ch, sampleRate);
        }
    }
}

QString EqualizerDSP::toMpvFilterString() const
{
    if (isFlat()) return "";

    QStringList bandFilters;
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        bandFilters.append(QString("equalizer=f=%1:width_type=o:w=1:g=%2")
                               .arg(EQ_BAND_FREQUENCIES[i], 0, 'f', 2)
                               .arg(m_gains[i], 0, 'f', 1));
    }
    return QString("lavfi=[%1]").arg(bandFilters.join(','));
}

QString EqualizerDSP::toGStreamerFilterString() const
{
    QStringList params;
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        params.append(QString("band%1=%2").arg(i).arg(m_gains[i], 0, 'f', 1));
    }
    return QString("equalizer-10bands %1").arg(params.join(' '));
}

QJsonObject EqualizerDSP::toJson() const
{
    QJsonObject obj;
    obj["preset"] = m_currentPreset;
    QJsonArray arr;
    for (int i = 0; i < EQ_NUM_BANDS; ++i) {
        arr.append(m_gains[i]);
    }
    obj["gains"] = arr;
    return obj;
}

bool EqualizerDSP::fromJson(const QJsonObject &json)
{
    if (json.contains("preset")) {
        m_currentPreset = json["preset"].toString();
    }
    if (json.contains("gains") && json["gains"].isArray()) {
        QJsonArray arr = json["gains"].toArray();
        int count = std::min(static_cast<int>(arr.size()), EQ_NUM_BANDS);
        for (int i = 0; i < count; ++i) {
            m_gains[i] = std::clamp(arr[i].toDouble(), EQ_MIN_GAIN_DB, EQ_MAX_GAIN_DB);
        }
        updateBiquadCoeffs(m_lastSampleRate);
        return true;
    }
    return false;
}

} // namespace Core
} // namespace Penguin
