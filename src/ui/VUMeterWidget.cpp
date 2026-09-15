#include "VUMeterWidget.h"
#include "BrutalistTheme.h"

#include <QPainter>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

namespace Penguin {
namespace UI {

VUMeterWidget::VUMeterWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

void VUMeterWidget::setLevels(double leftPeakDb, double rightPeakDb, double leftRmsDb, double rightRmsDb)
{
    m_levels.leftPeakDb = std::max(Core::VU_MIN_DB, std::min(leftPeakDb, Core::VU_MAX_DB));
    m_levels.rightPeakDb = std::max(Core::VU_MIN_DB, std::min(rightPeakDb, Core::VU_MAX_DB));
    m_levels.leftRmsDb = std::max(Core::VU_MIN_DB, std::min(leftRmsDb, Core::VU_MAX_DB));
    m_levels.rightRmsDb = std::max(Core::VU_MIN_DB, std::min(rightRmsDb, Core::VU_MAX_DB));

    if (m_levels.leftPeakDb > m_levels.leftPeakHoldDb) {
        m_levels.leftPeakHoldDb = m_levels.leftPeakDb;
    }
    if (m_levels.rightPeakDb > m_levels.rightPeakHoldDb) {
        m_levels.rightPeakHoldDb = m_levels.rightPeakDb;
    }
    m_levels.leftClipped = (m_levels.leftPeakDb >= Core::VU_CLIP_THRESHOLD_DB);
    m_levels.rightClipped = (m_levels.rightPeakDb >= Core::VU_CLIP_THRESHOLD_DB);

    update();
}

void VUMeterWidget::setVULevels(const Core::VULevels &levels)
{
    m_levels = levels;
    update();
}

void VUMeterWidget::setLeftPeakDb(double db)
{
    setLevels(db, m_levels.rightPeakDb, m_levels.leftRmsDb, m_levels.rightRmsDb);
}

void VUMeterWidget::setRightPeakDb(double db)
{
    setLevels(m_levels.leftPeakDb, db, m_levels.leftRmsDb, m_levels.rightRmsDb);
}

void VUMeterWidget::reset()
{
    m_levels = Core::VULevels();
    update();
}

QSize VUMeterWidget::sizeHint() const
{
    return QSize(360, 72);
}

QSize VUMeterWidget::minimumSizeHint() const
{
    return QSize(240, 60);
}

double VUMeterWidget::dbToX(double db, int width) const
{
    // Map [-60dB, +3dB] to [0, width]
    double clamped = std::max(Core::VU_MIN_DB, std::min(db, Core::VU_MAX_DB));
    double norm = (clamped - Core::VU_MIN_DB) / (Core::VU_MAX_DB - Core::VU_MIN_DB);
    return norm * width;
}

void VUMeterWidget::drawChannelBar(QPainter &p, const QRect &barRect, double peakDb, double peakHoldDb, bool clipped, const QString &label)
{
    int totalWidth = barRect.width();
    int totalHeight = barRect.height();

    // 1. Channel label (CH_L / CH_R)
    p.setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    p.setPen(BrutalistTheme::TEXT_SECONDARY_DIM);
    QRect labelRect(barRect.left() - 44, barRect.top(), 40, totalHeight);
    p.drawText(labelRect, Qt::AlignVCenter | Qt::AlignRight, label);

    // 2. Recessed background channel groove
    p.setPen(QPen(QColor("#1C1C26"), 1));
    p.setBrush(QColor("#0D0D12"));
    p.drawRoundedRect(barRect, 3, 3);

    // 3. Segmented LEDs with organic phosphor bloom
    int numSegments = 30;
    int segMargin = 1;
    double segWidth = static_cast<double>(totalWidth - (numSegments - 1) * segMargin - 4) / numSegments;

    for (int i = 0; i < numSegments; ++i) {
        double segDb = Core::VU_MIN_DB + (static_cast<double>(i + 1) / numSegments) * (Core::VU_MAX_DB - Core::VU_MIN_DB);
        double segLeft = barRect.left() + 2 + i * (segWidth + segMargin);
        QRectF segRect(segLeft, barRect.top() + 2, segWidth, totalHeight - 4);

        bool isActive = (peakDb >= segDb);

        QColor onColor;
        QColor offColor = QColor(20, 20, 28, 160);

        if (segDb > 0.0) {
            onColor = QColor("#FF1E1E"); // > 0dB Crimson
        } else if (segDb >= -3.0) {
            onColor = QColor("#FF6600"); // -3dB to 0dB Amber
        } else if (segDb >= -12.0) {
            onColor = QColor("#CCFF00"); // -12dB to -3dB Signal Lime
        } else {
            onColor = QColor("#88DD00"); // -60dB to -12dB Deep Lime
        }

        if (isActive) {
            p.setPen(Qt::NoPen);
            p.setBrush(onColor);
            p.drawRoundedRect(segRect, 1.5, 1.5);

            // Subtle glow on highest active segments
            if (i >= numSegments - 4) {
                p.setBrush(QColor(onColor.red(), onColor.green(), onColor.blue(), 50));
                p.drawRoundedRect(segRect.adjusted(-1, -1, 1, 1), 2, 2);
            }
        } else {
            p.setPen(QPen(QColor(30, 30, 40, 100), 0.5));
            p.setBrush(offColor);
            p.drawRoundedRect(segRect, 1.5, 1.5);
        }
    }

    // 4. Peak Hold needle with laser glow
    if (peakHoldDb > Core::VU_MIN_DB) {
        int holdX = barRect.left() + static_cast<int>(dbToX(peakHoldDb, totalWidth));
        holdX = std::min(holdX, barRect.right() - 2);

        // Glow aura
        p.setPen(QPen(QColor(255, 255, 255, 80), 3));
        p.drawLine(holdX, barRect.top() + 1, holdX, barRect.bottom() - 1);

        // Sharp core needle
        p.setPen(QPen(QColor("#FFFFFF"), 1.5));
        p.drawLine(holdX, barRect.top() + 1, holdX, barRect.bottom() - 1);
    }

    // 5. dB Readout text
    QString dbStr = (peakDb <= Core::VU_MIN_DB) ? "-inf" : QString("%1%2 dB").arg(peakDb >= 0 ? "+" : "").arg(peakDb, 0, 'f', 1);
    QRect textRect(barRect.right() + 8, barRect.top(), 56, totalHeight);
    p.setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    p.setPen(clipped ? BrutalistTheme::ACCENT_CLIP_RED : (peakDb >= -3.0 ? BrutalistTheme::ACCENT_SAFETY_ORANGE : BrutalistTheme::TEXT_HIGH_CONTRAST));
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, dbStr);

    // 6. CLIP badge pill
    if (clipped) {
        QRect clipRect(barRect.right() + 68, barRect.top() + 1, 36, totalHeight - 2);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#FF1E1E"));
        p.drawRoundedRect(clipRect, 3, 3);

        p.setPen(QColor("#FFFFFF"));
        p.setFont(BrutalistTheme::monospaceFont(7, QFont::Bold));
        p.drawText(clipRect, Qt::AlignCenter, "CLIP");
    }
}

void VUMeterWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Chassis background with micro-radii
    QRect bgRect = rect().adjusted(0, 0, -1, -1);
    p.setPen(QPen(BrutalistTheme::GRID_STRUCTURAL_BORDER, 1));
    p.setBrush(QColor("#08080C"));
    p.drawRoundedRect(bgRect, 4, 4);

    // Dimensions
    int leftOffset = 52;
    int rightOffset = 112;
    int barWidth = width() - leftOffset - rightOffset;
    if (barWidth < 20) barWidth = 20;

    int barHeight = 14;
    int chL_Y = 22;
    int chR_Y = 44;

    // dB Scale Markings across the top
    p.setFont(BrutalistTheme::monospaceFont(7, QFont::Normal));
    p.setPen(BrutalistTheme::TEXT_MUTED);

    double markDbs[] = {-60.0, -40.0, -20.0, -12.0, -6.0, -3.0, 0.0, 3.0};
    for (double db : markDbs) {
        int x = leftOffset + static_cast<int>(dbToX(db, barWidth));
        QString markStr = (db == 0.0) ? "0" : QString::number(static_cast<int>(db));
        if (db > 0) markStr = "+" + markStr;
        QRect markRect(x - 14, 4, 28, 12);

        if (db == 0.0) {
            p.setPen(BrutalistTheme::ACCENT_TELEMETRY_CYAN);
        } else if (db > 0.0) {
            p.setPen(BrutalistTheme::ACCENT_CLIP_RED);
        } else {
            p.setPen(BrutalistTheme::TEXT_MUTED);
        }
        p.drawText(markRect, Qt::AlignCenter, markStr);

        p.setPen(QPen(QColor("#242430"), 1));
        p.drawLine(x, 17, x, 20);
    }

    // Draw CH_L and CH_R
    drawChannelBar(p, QRect(leftOffset, chL_Y, barWidth, barHeight), m_levels.leftPeakDb, m_levels.leftPeakHoldDb, m_levels.leftClipped, "CH_L");
    drawChannelBar(p, QRect(leftOffset, chR_Y, barWidth, barHeight), m_levels.rightPeakDb, m_levels.rightPeakHoldDb, m_levels.rightClipped, "CH_R");
}

} // namespace UI
} // namespace Penguin
