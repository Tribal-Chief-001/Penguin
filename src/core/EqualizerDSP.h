#ifndef EQUALIZERDSP_H
#define EQUALIZERDSP_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QJsonObject>
#include <array>

namespace Penguin {
namespace Core {

// Number of ISO standard EQ bands
constexpr int EQ_NUM_BANDS = 10;
constexpr double EQ_MIN_GAIN_DB = -12.0;
constexpr double EQ_MAX_GAIN_DB = 12.0;

// ISO standard center frequencies (Hz)
constexpr std::array<double, EQ_NUM_BANDS> EQ_BAND_FREQUENCIES = {
    31.25, 62.5, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 16000.0
};

// Peaking Biquad IIR Filter Coefficients (Direct Form I / II)
struct BiquadCoeffs {
    double b0 = 1.0;
    double b1 = 0.0;
    double b2 = 0.0;
    double a1 = 0.0;
    double a2 = 0.0;
};

// Filter state for one channel and one band
struct BiquadState {
    double x1 = 0.0; // x[n-1]
    double x2 = 0.0; // x[n-2]
    double y1 = 0.0; // y[n-1]
    double y2 = 0.0; // y[n-2]

    void reset() {
        x1 = x2 = y1 = y2 = 0.0;
    }
};

class EqualizerDSP {
public:
    EqualizerDSP();

    // Gain adjustment per band (-12dB to +12dB)
    void setBandGain(int bandIndex, double gainDb);
    double bandGain(int bandIndex) const;

    // Set all 10 band gains
    void setAllGains(const QVector<double> &gains);
    QVector<double> allGains() const;

    // Center frequency for a given band index
    static double bandFrequency(int bandIndex);
    static QString bandLabel(int bandIndex);

    // Preset management
    void setPreset(const QString &presetName);
    QString currentPreset() const { return m_currentPreset; }
    static QStringList availablePresets();
    static QVector<double> presetGains(const QString &presetName);

    // Reset all bands to 0 dB
    void resetFlat();
    bool isFlat() const;

    // Compute Biquad Peaking filter coefficients (RBJ Audio EQ Cookbook)
    static BiquadCoeffs computePeakingCoeffs(double centerFreqHz, double gainDb, double sampleRate = 48000.0, double q = 1.414);

    // Calculate total magnitude response in dB at a given frequency across all 10 bands
    double calculateMagnitudeResponse(double freqHz, double sampleRate = 48000.0) const;

    // Process a single sample through all 10 bands for a specific channel
    float processSample(float sample, int channel = 0, double sampleRate = 48000.0);

    // Process a multi-channel interleaved float buffer in-place
    void processInterleavedBuffer(float *buffer, int numFrames, int numChannels, double sampleRate = 48000.0);

    // Reset internal DSP states
    void resetDSPStates();

    // String serializations
    QString toMpvFilterString() const;
    QString toGStreamerFilterString() const;

    // JSON serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

private:
    void updateBiquadCoeffs(double sampleRate = 48000.0);

    std::array<double, EQ_NUM_BANDS> m_gains;
    std::array<BiquadCoeffs, EQ_NUM_BANDS> m_coeffs;
    // States for up to 8 channels
    static constexpr int MAX_CHANNELS = 8;
    std::array<std::array<BiquadState, EQ_NUM_BANDS>, MAX_CHANNELS> m_states;
    double m_lastSampleRate = 48000.0;
    QString m_currentPreset;
};

} // namespace Core
} // namespace Penguin

#endif // EQUALIZERDSP_H
