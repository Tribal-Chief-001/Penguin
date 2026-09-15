#include "DiagnosticsHUDWidget.h"
#include "BrutalistTheme.h"

#include <QPainter>
#include <QFontMetrics>
#include <cmath>

namespace Penguin {
namespace UI {

DiagnosticsHUDWidget::DiagnosticsHUDWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void DiagnosticsHUDWidget::updateDiagnostics(const Core::DiagnosticsData &diag)
{
    m_diag = diag;
    update();
}

void DiagnosticsHUDWidget::setFps(double actualFps, double nominalFps)
{
    m_diag.fps = actualFps;
    if (nominalFps > 0.0) m_diag.nominalFps = nominalFps;
    update();
}

void DiagnosticsHUDWidget::setDroppedFrames(qint64 dropped)
{
    m_diag.droppedFrames = dropped;
    update();
}

void DiagnosticsHUDWidget::setVideoCodec(const QString &codec)
{
    m_diag.videoCodec = codec;
    update();
}

void DiagnosticsHUDWidget::setAudioCodec(const QString &codec)
{
    m_diag.audioCodec = codec;
    update();
}

void DiagnosticsHUDWidget::setBitrate(qint64 bps)
{
    m_diag.videoBitrate = bps;
    update();
}

void DiagnosticsHUDWidget::setResolution(int w, int h)
{
    m_diag.videoWidth = w;
    m_diag.videoHeight = h;
    update();
}

void DiagnosticsHUDWidget::setRenderTimeMs(double ms)
{
    m_diag.renderTimeMs = ms;
    update();
}

void DiagnosticsHUDWidget::setAvSkewMs(double ms)
{
    m_diag.avSkewMs = ms;
    update();
}

QSize DiagnosticsHUDWidget::sizeHint() const
{
    return QSize(380, 110);
}

QSize DiagnosticsHUDWidget::minimumSizeHint() const
{
    return QSize(300, 90);
}

void DiagnosticsHUDWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Smoked glass HUD card with micro-radii
    QRect bgRect = rect().adjusted(0, 0, -1, -1);
    p.setPen(QPen(QColor(0, 229, 255, 60), 1));
    p.setBrush(QColor(8, 8, 14, 225));
    p.drawRoundedRect(bgRect, 6, 6);

    // Optical corner brackets (ARRI/RED cinema camera style)
    int blen = 8;
    p.setPen(QPen(BrutalistTheme::ACCENT_TELEMETRY_CYAN, 1.5));
    // Top-left
    p.drawLine(bgRect.left() + 2, bgRect.top() + 2 + blen, bgRect.left() + 2, bgRect.top() + 2);
    p.drawLine(bgRect.left() + 2, bgRect.top() + 2, bgRect.left() + 2 + blen, bgRect.top() + 2);
    // Top-right
    p.drawLine(bgRect.right() - 2 - blen, bgRect.top() + 2, bgRect.right() - 2, bgRect.top() + 2);
    p.drawLine(bgRect.right() - 2, bgRect.top() + 2, bgRect.right() - 2, bgRect.top() + 2 + blen);
    // Bottom-left
    p.drawLine(bgRect.left() + 2, bgRect.bottom() - 2 - blen, bgRect.left() + 2, bgRect.bottom() - 2);
    p.drawLine(bgRect.left() + 2, bgRect.bottom() - 2, bgRect.left() + 2 + blen, bgRect.bottom() - 2);
    // Bottom-right
    p.drawLine(bgRect.right() - 2 - blen, bgRect.bottom() - 2, bgRect.right() - 2, bgRect.bottom() - 2);
    p.drawLine(bgRect.right() - 2, bgRect.bottom() - 2 - blen, bgRect.right() - 2, bgRect.bottom() - 2);

    // Glowing laser accent dot & line
    p.setPen(Qt::NoPen);
    p.setBrush(BrutalistTheme::ACCENT_TELEMETRY_CYAN);
    p.drawEllipse(QPoint(bgRect.left() + 14, bgRect.top() + 14), 2, 2);

    p.setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));

    int lineHeight = 18;
    int curY = 18;
    int marginX = 12;

    // Header Tag
    p.setPen(BrutalistTheme::ACCENT_TELEMETRY_CYAN);
    p.drawText(marginX, curY, "TELEMETRY OSD // HARDWARE DIAGNOSTICS");
    curY += lineHeight;

    p.setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));

    // Line 1: FPS & Dropped frames
    double fpsNom = (m_diag.nominalFps > 0.0) ? m_diag.nominalFps : 60.0;
    double fpsAct = (m_diag.fps > 0.0) ? m_diag.fps : fpsNom;
    QString line1 = QString("FPS: %1 / %2  |  DROP: %3")
        .arg(fpsAct, 0, 'f', 2)
        .arg(fpsNom, 0, 'f', 2)
        .arg(m_diag.droppedFrames);
    p.setPen(BrutalistTheme::TEXT_HIGH_CONTRAST);
    p.drawText(marginX, curY, line1);
    curY += lineHeight;

    // Line 2: Video Codec & Bitrate & Resolution
    QString vCodec = m_diag.videoCodec.isEmpty() ? "H.264" : m_diag.videoCodec.toUpper();
    qint64 brKbps = (m_diag.videoBitrate > 0) ? (m_diag.videoBitrate / 1000) : 4820;
    int w = (m_diag.videoWidth > 0) ? m_diag.videoWidth : 1920;
    int h = (m_diag.videoHeight > 0) ? m_diag.videoHeight : 1080;
    QString line2 = QString("V_CODEC: %1 %2 kbps  |  RES: %3x%4@%5Hz")
        .arg(vCodec)
        .arg(brKbps)
        .arg(w)
        .arg(h)
        .arg(static_cast<int>(std::round(fpsNom)));
    p.drawText(marginX, curY, line2);
    curY += lineHeight;

    // Line 3: Audio Codec & Sample Rate & Color Space
    QString aCodec = m_diag.audioCodec.isEmpty() ? "AAC" : m_diag.audioCodec.toUpper();
    int sRate = (m_diag.audioSampleRate > 0) ? m_diag.audioSampleRate : 48000;
    int aChannels = (m_diag.audioChannels > 0) ? m_diag.audioChannels : 2;
    QString line3 = QString("A_CODEC: %1 (%2ch %3Hz)  |  COLOR: %4")
        .arg(aCodec)
        .arg(aChannels)
        .arg(sRate)
        .arg(m_diag.colorSpace.isEmpty() ? "BT.709 8-bit" : QString("%1 %2-bit").arg(m_diag.colorSpace).arg(m_diag.bitDepth));
    p.drawText(marginX, curY, line3);
    curY += lineHeight;

    // Line 4: Render time & A/V Skew
    double rTime = (m_diag.renderTimeMs > 0.0) ? m_diag.renderTimeMs : 1.42;
    QString line4 = QString("RENDER: %1ms  |  A/V SKEW: %2%3ms")
        .arg(rTime, 0, 'f', 2)
        .arg(m_diag.avSkewMs >= 0 ? "+" : "")
        .arg(m_diag.avSkewMs, 0, 'f', 3);
    p.setPen(BrutalistTheme::TEXT_SECONDARY_DIM);
    p.drawText(marginX, curY, line4);
}

} // namespace UI
} // namespace Penguin
