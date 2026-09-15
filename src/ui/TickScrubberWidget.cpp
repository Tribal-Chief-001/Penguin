#include "TickScrubberWidget.h"
#include "BrutalistTheme.h"
#include "TimecodeFormatter.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>

namespace Penguin {
namespace UI {

TickScrubberWidget::TickScrubberWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

void TickScrubberWidget::setPositionMs(qint64 posMs)
{
    if (m_isDragging) return; // Don't overwrite during active user drag
    posMs = std::max(0LL, std::min(posMs, m_durationMs > 0 ? m_durationMs : posMs));
    if (m_positionMs != posMs) {
        m_positionMs = posMs;
        emit positionChanged(m_positionMs);
        update();
    }
}

void TickScrubberWidget::setDurationMs(qint64 durationMs)
{
    durationMs = std::max(0LL, durationMs);
    if (m_durationMs != durationMs) {
        m_durationMs = durationMs;
        emit durationChanged(m_durationMs);
        update();
    }
}

void TickScrubberWidget::setFps(double fps)
{
    if (fps > 0.0 && !qFuzzyCompare(m_fps, fps)) {
        m_fps = fps;
        update();
    }
}

void TickScrubberWidget::setDropFrame(bool dropFrame)
{
    if (m_dropFrame != dropFrame) {
        m_dropFrame = dropFrame;
        update();
    }
}

void TickScrubberWidget::setChapters(const QList<ChapterMarker> &chapters)
{
    m_chapters = chapters;
    update();
}

void TickScrubberWidget::addChapter(qint64 timestampMs, const QString &title)
{
    m_chapters.append(ChapterMarker{timestampMs, title});
    update();
}

void TickScrubberWidget::clearChapters()
{
    m_chapters.clear();
    update();
}

QSize TickScrubberWidget::sizeHint() const
{
    return QSize(600, 38);
}

QSize TickScrubberWidget::minimumSizeHint() const
{
    return QSize(300, 32);
}

QRect TickScrubberWidget::trackRect() const
{
    int leftMargin = 110;  // Space for left SMPTE elapsed display
    int rightMargin = 110; // Space for right SMPTE remaining display
    int w = width() - leftMargin - rightMargin;
    if (w < 10) w = 10;
    return QRect(leftMargin, 0, w, height());
}

qint64 TickScrubberWidget::applyMagneticSnap(int x, qint64 rawMs) const
{
    m_snappedChapterIndex = -1;
    m_isMagneticSnapped = false;
    for (int i = 0; i < m_chapters.size(); ++i) {
        int cx = positionMsToX(m_chapters[i].timestampMs);
        if (std::abs(x - cx) <= 12) {
            m_snappedChapterIndex = i;
            m_isMagneticSnapped = true;
            return m_chapters[i].timestampMs;
        }
    }
    return rawMs;
}

qint64 TickScrubberWidget::xToPositionMs(int x) const
{
    QRect tr = trackRect();
    if (tr.width() <= 0 || m_durationMs <= 0) return 0;

    int clampedX = std::max(tr.left(), std::min(x, tr.right()));
    double fraction = static_cast<double>(clampedX - tr.left()) / tr.width();
    return static_cast<qint64>(std::round(fraction * m_durationMs));
}

int TickScrubberWidget::positionMsToX(qint64 ms) const
{
    QRect tr = trackRect();
    if (m_durationMs <= 0 || tr.width() <= 0) return tr.left();

    double fraction = static_cast<double>(ms) / m_durationMs;
    fraction = std::max(0.0, std::min(fraction, 1.0));
    return tr.left() + static_cast<int>(std::round(fraction * tr.width()));
}

void TickScrubberWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 1. Background
    p.fillRect(rect(), BrutalistTheme::BG_DEEP_OBSIDIAN);

    // 2. Top and Bottom structural lines
    p.setPen(QPen(BrutalistTheme::GRID_STRUCTURAL_BORDER, 1));
    p.drawLine(0, 0, width(), 0);
    p.drawLine(0, height() - 1, width(), height() - 1);

    // 3. Timecode Text Displays (JetBrains Mono)
    QFont monoFont = BrutalistTheme::monospaceFont(9, QFont::Bold);
    p.setFont(monoFont);

    QString elapsedStr = Core::TimecodeFormatter::formatTimecode(m_positionMs, m_fps, m_dropFrame);
    QString remainingStr = Core::TimecodeFormatter::formatRemaining(m_positionMs, m_durationMs, m_fps, m_dropFrame);

    int centerY = height() / 2;

    // Left elapsed time with pulsing live dot
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#CCFF00"));
    p.drawEllipse(QPoint(12, centerY), 3, 3);

    p.setPen(BrutalistTheme::TEXT_HIGH_CONTRAST);
    QRect leftTextRect(20, 0, 84, height());
    p.drawText(leftTextRect, Qt::AlignVCenter | Qt::AlignLeft, elapsedStr);

    // Right remaining time
    p.setPen(BrutalistTheme::TEXT_SECONDARY_DIM);
    QRect rightTextRect(width() - 104, 0, 96, height());
    p.drawText(rightTextRect, Qt::AlignVCenter | Qt::AlignRight, remainingStr);

    // 4. Track Region
    QRect tr = trackRect();

    // Recessed track channel groove
    QRect trackChannel(tr.left(), centerY - 3, tr.width(), 6);
    p.setPen(QPen(QColor("#1C1C26"), 1));
    p.setBrush(QColor("#0B0B10"));
    p.drawRoundedRect(trackChannel, 3, 3);

    // Played progress bar highlight with glowing gradient
    int playheadX = positionMsToX(m_positionMs);
    if (playheadX > tr.left()) {
        int playedW = playheadX - tr.left();
        QRect playedBar(tr.left(), centerY - 3, playedW, 6);

        QLinearGradient grad(tr.left(), 0, playheadX, 0);
        grad.setColorAt(0.0, QColor("#FF3300"));
        grad.setColorAt(1.0, QColor("#FF7700"));
        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        p.drawRoundedRect(playedBar, 3, 3);

        // Ambient subtle glow bloom
        QLinearGradient glowGrad(tr.left(), 0, playheadX, 0);
        glowGrad.setColorAt(0.0, QColor(255, 51, 0, 30));
        glowGrad.setColorAt(1.0, QColor(255, 119, 0, 80));
        p.setBrush(glowGrad);
        p.drawRoundedRect(playedBar.adjusted(0, -1, 1, 1), 3, 3);
    }

    // 5. Graduation Ticks (Mechanical Ruler)
    int tickSpacing = 16;
    int numTicks = tr.width() / tickSpacing;
    for (int i = 0; i <= numTicks; ++i) {
        int tx = tr.left() + i * tickSpacing;
        if (tx > tr.right()) break;

        bool isMajor = (i % 5 == 0);
        int tickHeight = isMajor ? 12 : 5;
        QColor tickColor = isMajor ? BrutalistTheme::TEXT_SECONDARY_DIM : QColor("#2A2A38");

        p.setPen(QPen(tickColor, 1));
        p.drawLine(tx, centerY - tickHeight / 2, tx, centerY + tickHeight / 2);
    }

    // 6. Chapter Markers (Solid Cyan Diamonds with Magnetic Aura)
    if (m_durationMs > 0) {
        for (int i = 0; i < m_chapters.size(); ++i) {
            const auto &chap = m_chapters[i];
            int cx = positionMsToX(chap.timestampMs);
            bool isSnapped = (i == m_snappedChapterIndex);

            if (isSnapped) {
                // Magnetic glowing aura ring
                p.setPen(QPen(QColor(0, 229, 255, 160), 1));
                p.setBrush(QColor(0, 229, 255, 45));
                p.drawEllipse(QPoint(cx, centerY), 8, 8);
            }

            QPainterPath diamond;
            diamond.moveTo(cx, centerY - 6);
            diamond.lineTo(cx + 4, centerY);
            diamond.lineTo(cx, centerY + 6);
            diamond.lineTo(cx - 4, centerY);
            diamond.closeSubpath();

            p.setPen(QPen(isSnapped ? QColor("#FFFFFF") : QColor("#0088AA"), 1));
            p.setBrush(isSnapped ? QColor("#00FFFF") : BrutalistTheme::ACCENT_TELEMETRY_CYAN);
            p.drawPath(diamond);
        }
    }

    // 7. Hover Indicator & Floating SMPTE Badge
    if (m_isHovered && !m_isDragging && m_durationMs > 0 && m_hoverX >= tr.left() && m_hoverX <= tr.right()) {
        int effectiveX = m_isMagneticSnapped && m_snappedChapterIndex >= 0
            ? positionMsToX(m_chapters[m_snappedChapterIndex].timestampMs)
            : m_hoverX;

        p.setPen(QPen(QColor("#00E5FF"), 1, Qt::DashLine));
        p.drawLine(effectiveX, 4, effectiveX, height() - 4);

        // Floating hover badge
        qint64 hoverMs = xToPositionMs(effectiveX);
        QString badgeText = Core::TimecodeFormatter::formatTimecode(hoverMs, m_fps, m_dropFrame);
        if (m_isMagneticSnapped && m_snappedChapterIndex >= 0 && m_snappedChapterIndex < m_chapters.size()) {
            badgeText += " • " + m_chapters[m_snappedChapterIndex].title;
        }

        QFont badgeFont = BrutalistTheme::monospaceFont(7, QFont::Bold);
        QFontMetrics fm(badgeFont);
        int badgeW = fm.horizontalAdvance(badgeText) + 12;
        int badgeH = 16;
        int badgeX = std::max(tr.left(), std::min(effectiveX - badgeW / 2, tr.right() - badgeW));
        int badgeY = 2;

        QRect badgeRect(badgeX, badgeY, badgeW, badgeH);
        p.setPen(QPen(QColor(0, 229, 255, 140), 1));
        p.setBrush(QColor(11, 11, 16, 230));
        p.drawRoundedRect(badgeRect, 3, 3);

        p.setFont(badgeFont);
        p.setPen(QColor("#00E5FF"));
        p.drawText(badgeRect, Qt::AlignCenter, badgeText);
    }

    // 8. Playhead Needle (Safety Orange with Laser Glow and Pip)
    p.setPen(QPen(QColor(255, 68, 0, 75), 4));
    p.drawLine(playheadX, 2, playheadX, height() - 2);

    p.setPen(QPen(BrutalistTheme::ACCENT_SAFETY_ORANGE, 2));
    p.drawLine(playheadX, 2, playheadX, height() - 2);

    // Top Flag Triangle
    QPainterPath flag;
    flag.moveTo(playheadX - 4, 1);
    flag.lineTo(playheadX + 4, 1);
    flag.lineTo(playheadX, 7);
    flag.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(BrutalistTheme::ACCENT_SAFETY_ORANGE);
    p.drawPath(flag);

    // Center micro-pip
    p.setBrush(QColor("#FFFFFF"));
    p.drawEllipse(QPoint(playheadX, centerY), 2, 2);
}

void TickScrubberWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QRect tr = trackRect();
        if (!tr.contains(event->pos())) {
            event->ignore();
            return;
        }
        m_isDragging = true;
        emit scrubbingStarted();
        qint64 rawMs = xToPositionMs(event->pos().x());
        qint64 targetMs = applyMagneticSnap(event->pos().x(), rawMs);
        m_positionMs = targetMs;
        emit positionChanged(m_positionMs);
        emit seekRequested(targetMs);
        update();
    }
}

void TickScrubberWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_isHovered = true;
    m_hoverX = event->pos().x();

    QRect tr = trackRect();
    if (m_isDragging) {
        qint64 rawMs = xToPositionMs(event->pos().x());
        qint64 targetMs = applyMagneticSnap(event->pos().x(), rawMs);
        m_positionMs = targetMs;
        emit positionChanged(m_positionMs);
        emit seekRequested(targetMs);
        update();
    } else if (tr.contains(event->pos()) && m_durationMs > 0) {
        qint64 rawMs = xToPositionMs(event->pos().x());
        qint64 hoverMs = applyMagneticSnap(event->pos().x(), rawMs);
        emit hoverPositionChanged(hoverMs);

        // Check if hovering near chapter marker
        QString tooltipText = Core::TimecodeFormatter::formatTimecode(hoverMs, m_fps, m_dropFrame);
        for (const auto &chap : m_chapters) {
            int cx = positionMsToX(chap.timestampMs);
            if (std::abs(event->pos().x() - cx) <= 10) {
                tooltipText = QString("[%1] %2 (%3)")
                    .arg(Core::TimecodeFormatter::formatTimecode(chap.timestampMs, m_fps, m_dropFrame))
                    .arg(chap.title)
                    .arg(tooltipText);
                break;
            }
        }
        setToolTip(tooltipText);
        update();
    } else {
        setToolTip(QString());
        update();
    }
}

void TickScrubberWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        qint64 targetMs = xToPositionMs(event->pos().x());
        m_positionMs = targetMs;
        emit positionChanged(m_positionMs);
        emit seekRequested(targetMs);
        emit scrubbingEnded();
        update();
    }
}

void TickScrubberWidget::leaveEvent(QEvent * /*event*/)
{
    m_isHovered = false;
    update();
}

} // namespace UI
} // namespace Penguin
