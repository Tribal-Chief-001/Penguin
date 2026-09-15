#include "EqualizerRackWidget.h"
#include "BrutalistTheme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

namespace Penguin {
namespace UI {

EqualizerRackWidget::EqualizerRackWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void EqualizerRackWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    // 1. Top Header: Title, Preset Combobox, Flat Reset Button
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(8);

    auto *titleLabel = new QLabel("10-BAND ISO GRAPHIC EQUALIZER RACK", this);
    titleLabel->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    titleLabel->setStyleSheet("color: #FFFFFF; font-weight: bold; letter-spacing: 0.5px;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch(1);

    auto *presetLabel = new QLabel("PRESET:", this);
    presetLabel->setFont(BrutalistTheme::monospaceFont(7, QFont::Bold));
    presetLabel->setStyleSheet("color: #777788;");
    headerLayout->addWidget(presetLabel);

    m_presetCombo = new QComboBox(this);
    m_presetCombo->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_presetCombo->setStyleSheet("background-color: #12121A; border: 1px solid rgba(255, 255, 255, 0.12); border-radius: 4px; color: #FFFFFF; padding: 3px 8px;");
    m_presetCombo->addItems(Core::EqualizerDSP::availablePresets());
    connect(m_presetCombo, &QComboBox::currentTextChanged, this, &EqualizerRackWidget::onPresetComboActivated);
    headerLayout->addWidget(m_presetCombo);

    m_resetFlatBtn = new QPushButton("FLAT RESET", this);
    m_resetFlatBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_resetFlatBtn->setStyleSheet(BrutalistTheme::pillButtonStyleSheet());
    connect(m_resetFlatBtn, &QPushButton::clicked, this, &EqualizerRackWidget::resetFlat);
    headerLayout->addWidget(m_resetFlatBtn);

    mainLayout->addLayout(headerLayout);

    // 2. Sliders Grid
    auto *rackContainer = new QWidget(this);
    rackContainer->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #101016, stop:1 #08080C); border: 1px solid rgba(255, 255, 255, 0.08); border-radius: 6px;");
    auto *rackLayout = new QHBoxLayout(rackContainer);
    rackLayout->setContentsMargins(8, 8, 8, 8);
    rackLayout->setSpacing(4);

    QString faderStyle = R"(
        QSlider::groove:vertical {
            width: 4px;
            background: #14141E;
            border: 1px solid #22222E;
            border-radius: 2px;
        }
        QSlider::handle:vertical {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D0D0D8, stop:0.5 #FFFFFF, stop:1 #90909A);
            border: 1px solid #333342;
            height: 14px;
            margin-left: -5px;
            margin-right: -5px;
            border-radius: 2px;
        }
        QSlider::handle:vertical:hover {
            background: #CCFF00;
            border: 1px solid #CCFF00;
        }
    )";

    for (int i = 0; i < Core::EQ_NUM_BANDS; ++i) {
        auto *colLayout = new QVBoxLayout();
        colLayout->setSpacing(2);
        colLayout->setAlignment(Qt::AlignCenter);

        // Gain Readout Label (e.g. +0.0dB)
        auto *gainLabel = new QLabel("+0.0", this);
        gainLabel->setFont(BrutalistTheme::monospaceFont(7, QFont::Bold));
        gainLabel->setStyleSheet("color: #777788; border: none; background: transparent;");
        gainLabel->setAlignment(Qt::AlignCenter);
        m_gainLabels[i] = gainLabel;
        colLayout->addWidget(gainLabel);

        // Vertical Slider: -120 to +120
        auto *slider = new QSlider(Qt::Vertical, this);
        slider->setRange(-120, 120);
        slider->setValue(0);
        slider->setSingleStep(5);
        slider->setPageStep(20);
        slider->setTickPosition(QSlider::TicksBothSides);
        slider->setTickInterval(60);
        slider->setMinimumHeight(90);
        slider->setProperty("bandIndex", i);
        slider->setStyleSheet(faderStyle);
        connect(slider, &QSlider::valueChanged, this, &EqualizerRackWidget::onSliderValueChanged);
        m_sliders[i] = slider;
        colLayout->addWidget(slider, 1, Qt::AlignHCenter);

        // Frequency Label (e.g. 32Hz, 1k)
        auto *freqLabel = new QLabel(Core::EqualizerDSP::bandLabel(i), this);
        freqLabel->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
        freqLabel->setStyleSheet("color: #00E5FF; border: none; background: transparent;");
        freqLabel->setAlignment(Qt::AlignCenter);
        m_freqLabels[i] = freqLabel;
        colLayout->addWidget(freqLabel);

        rackLayout->addLayout(colLayout);
    }

    mainLayout->addWidget(rackContainer);
}

void EqualizerRackWidget::updateSliderGainLabel(int bandIndex, double gainDb)
{
    if (bandIndex >= 0 && bandIndex < Core::EQ_NUM_BANDS) {
        QString txt = QString("%1%2").arg(gainDb > 0 ? "+" : "").arg(gainDb, 0, 'f', 1);
        m_gainLabels[bandIndex]->setText(txt);
        if (std::abs(gainDb) < 0.05) {
            m_gainLabels[bandIndex]->setStyleSheet("color: #777788; border: none; background: transparent;");
        } else if (gainDb > 0) {
            m_gainLabels[bandIndex]->setStyleSheet("color: #CCFF00; border: none; background: transparent;");
        } else {
            m_gainLabels[bandIndex]->setStyleSheet("color: #FF4400; border: none; background: transparent;");
        }
    }
}

void EqualizerRackWidget::onSliderValueChanged(int value)
{
    if (m_blockSliderSignals) return;

    auto *slider = qobject_cast<QSlider*>(sender());
    if (!slider) return;

    int bandIndex = slider->property("bandIndex").toInt();
    double gainDb = value / 10.0;
    m_dsp.setBandGain(bandIndex, gainDb);
    updateSliderGainLabel(bandIndex, gainDb);

    emit bandGainChanged(bandIndex, gainDb);
    emit allGainsChanged(m_dsp.allGains());
}

void EqualizerRackWidget::onPresetComboActivated(const QString &presetName)
{
    setPreset(presetName);
}

double EqualizerRackWidget::bandGain(int bandIndex) const
{
    return m_dsp.bandGain(bandIndex);
}

QVector<double> EqualizerRackWidget::allGains() const
{
    return m_dsp.allGains();
}

QString EqualizerRackWidget::currentPreset() const
{
    return m_dsp.currentPreset();
}

void EqualizerRackWidget::setBandGain(int bandIndex, double gainDb)
{
    if (bandIndex < 0 || bandIndex >= Core::EQ_NUM_BANDS) return;

    m_dsp.setBandGain(bandIndex, gainDb);

    m_blockSliderSignals = true;
    m_sliders[bandIndex]->setValue(static_cast<int>(std::round(gainDb * 10.0)));
    updateSliderGainLabel(bandIndex, gainDb);
    m_blockSliderSignals = false;

    emit bandGainChanged(bandIndex, gainDb);
}

void EqualizerRackWidget::setAllGains(const QVector<double> &gains)
{
    m_dsp.setAllGains(gains);

    m_blockSliderSignals = true;
    for (int i = 0; i < Core::EQ_NUM_BANDS && i < gains.size(); ++i) {
        m_sliders[i]->setValue(static_cast<int>(std::round(gains[i] * 10.0)));
        updateSliderGainLabel(i, gains[i]);
    }
    m_blockSliderSignals = false;

    emit allGainsChanged(m_dsp.allGains());
}

void EqualizerRackWidget::setPreset(const QString &presetName)
{
    m_dsp.setPreset(presetName);
    QVector<double> gains = m_dsp.allGains();

    m_blockSliderSignals = true;
    for (int i = 0; i < Core::EQ_NUM_BANDS && i < gains.size(); ++i) {
        m_sliders[i]->setValue(static_cast<int>(std::round(gains[i] * 10.0)));
        updateSliderGainLabel(i, gains[i]);
    }
    if (m_presetCombo->currentText() != presetName) {
        m_presetCombo->setCurrentText(presetName);
    }
    m_blockSliderSignals = false;

    emit presetChanged(presetName);
    emit allGainsChanged(gains);
}

void EqualizerRackWidget::resetFlat()
{
    setPreset("Flat");
}

} // namespace UI
} // namespace Penguin
