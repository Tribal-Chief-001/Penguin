#ifndef EQUALIZERRACKWIDGET_H
#define EQUALIZERRACKWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QVector>
#include <array>

#include "EqualizerDSP.h"

namespace Penguin {
namespace UI {

class EqualizerRackWidget : public QWidget {
    Q_OBJECT

public:
    explicit EqualizerRackWidget(QWidget *parent = nullptr);
    virtual ~EqualizerRackWidget() = default;

    double bandGain(int bandIndex) const;
    QVector<double> allGains() const;
    QString currentPreset() const;

public slots:
    void setBandGain(int bandIndex, double gainDb);
    void setAllGains(const QVector<double> &gains);
    void setPreset(const QString &presetName);
    void resetFlat();

signals:
    void bandGainChanged(int bandIndex, double gainDb);
    void allGainsChanged(const QVector<double> &gains);
    void presetChanged(const QString &presetName);

private slots:
    void onSliderValueChanged(int value);
    void onPresetComboActivated(const QString &presetName);

private:
    void setupUI();
    void updateSliderGainLabel(int bandIndex, double gainDb);

    Core::EqualizerDSP m_dsp;
    QComboBox *m_presetCombo = nullptr;
    QPushButton *m_resetFlatBtn = nullptr;

    std::array<QSlider*, Core::EQ_NUM_BANDS> m_sliders{};
    std::array<QLabel*, Core::EQ_NUM_BANDS> m_gainLabels{};
    std::array<QLabel*, Core::EQ_NUM_BANDS> m_freqLabels{};

    bool m_blockSliderSignals = false;
};

} // namespace UI
} // namespace Penguin

#endif // EQUALIZERRACKWIDGET_H
