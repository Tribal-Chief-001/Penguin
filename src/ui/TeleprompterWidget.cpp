#include "TeleprompterWidget.h"
#include "BrutalistTheme.h"
#include "TimecodeFormatter.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEasingCurve>
#include <cmath>
#include <algorithm>

namespace Penguin {
namespace UI {

TeleprompterWidget::TeleprompterWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    m_scrollAnim = new QPropertyAnimation(this, "scrollOffset", this);
    m_scrollAnim->setDuration(250);
    m_scrollAnim->setEasingCurve(QEasingCurve::OutCubic);
}

void TeleprompterWidget::setScrollOffset(double offset)
{
    m_scrollOffset = offset;
    update();
}

bool TeleprompterWidget::loadLrcContent(const QString &lrcContent)
{
    bool ok = m_parser.parse(lrcContent);
    m_cues = m_parser.cues();
    m_activeCueIndex = -1;
    m_scrollOffset = 0.0;
    m_targetScrollOffset = 0.0;
    update();
    return ok;
}

bool TeleprompterWidget::loadLrcFile(const QString &filePath)
{
    bool ok = m_parser.loadFromFile(filePath);
    m_cues = m_parser.cues();
    m_activeCueIndex = -1;
    m_scrollOffset = 0.0;
    m_targetScrollOffset = 0.0;
    update();
    return ok;
}

void TeleprompterWidget::setLrcParser(const Core::LrcParser &parser)
{
    m_parser = parser;
    m_cues = m_parser.cues();
    m_activeCueIndex = -1;
    m_scrollOffset = 0.0;
    m_targetScrollOffset = 0.0;
    update();
}

void TeleprompterWidget::clearLyrics()
{
    m_parser.clear();
    m_cues.clear();
    m_activeCueIndex = -1;
    m_scrollOffset = 0.0;
    m_targetScrollOffset = 0.0;
    update();
}

void TeleprompterWidget::setPositionMs(qint64 positionMs)
{
    m_lastPositionMs = positionMs;
    if (m_cues.isEmpty()) return;

    int newIdx = m_parser.findActiveCueIndex(positionMs);
    if (newIdx != m_activeCueIndex) {
        m_activeCueIndex = newIdx;
        QString text = (m_activeCueIndex >= 0 && m_activeCueIndex < m_cues.size()) ? m_cues[m_activeCueIndex].text : QString();
        emit activeCueChanged(m_activeCueIndex, text);
        updateScrollTarget(true);
    }
}

void TeleprompterWidget::setActiveCueIndex(int cueIndex)
{
    if (cueIndex >= -1 && cueIndex < m_cues.size() && cueIndex != m_activeCueIndex) {
        m_activeCueIndex = cueIndex;
        QString text = (m_activeCueIndex >= 0 && m_activeCueIndex < m_cues.size()) ? m_cues[m_activeCueIndex].text : QString();
        emit activeCueChanged(m_activeCueIndex, text);
        updateScrollTarget(true);
    }
}

void TeleprompterWidget::updateScrollTarget(bool animate)
{
    if (m_activeCueIndex < 0 || m_cues.isEmpty()) {
        m_targetScrollOffset = 0.0;
    } else {
        int centerY = height() / 2;
        m_targetScrollOffset = m_activeCueIndex * lineSpacing() - centerY + lineSpacing() / 2;
    }

    if (animate) {
        m_scrollAnim->stop();
        m_scrollAnim->setStartValue(m_scrollOffset);
        m_scrollAnim->setEndValue(m_targetScrollOffset);
        m_scrollAnim->start();
    } else {
        m_scrollOffset = m_targetScrollOffset;
        update();
    }
}

int TeleprompterWidget::cueIndexAtY(int y) const
{
    if (m_cues.isEmpty()) return -1;
    int adjustedY = y + static_cast<int>(m_scrollOffset);
    int idx = adjustedY / lineSpacing();
    if (idx >= 0 && idx < m_cues.size()) {
        return idx;
    }
    return -1;
}

QSize TeleprompterWidget::sizeHint() const
{
    return QSize(400, 300);
}

QSize TeleprompterWidget::minimumSizeHint() const
{
    return QSize(250, 150);
}

void TeleprompterWidget::resizeEvent(QResizeEvent * /*event*/)
{
    updateScrollTarget(false);
}

void TeleprompterWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 1. Background & Border
    p.fillRect(rect(), BrutalistTheme::BG_DEEP_OBSIDIAN);
    p.setPen(QPen(BrutalistTheme::GRID_STRUCTURAL_BORDER, 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    // 2. Empty State
    if (m_cues.isEmpty()) {
        p.setFont(BrutalistTheme::monospaceFont(9, QFont::Normal));
        p.setPen(BrutalistTheme::TEXT_MUTED);
        p.drawText(rect(), Qt::AlignCenter, "SYNCHRONIZED .LRC TELEPROMPTER\n[ NO LYRICS LOADED ]");
        return;
    }

    // 3. Render Lyric Lines
    int h = height();
    int w = width();
    int centerY = h / 2;

    // Draw subtle focus bracket / center guide line
    p.setPen(QPen(QColor("#1E1E24"), 1, Qt::DotLine));
    p.drawLine(10, centerY - lineSpacing() / 2, w - 10, centerY - lineSpacing() / 2);
    p.drawLine(10, centerY + lineSpacing() / 2, w - 10, centerY + lineSpacing() / 2);

    for (int i = 0; i < m_cues.size(); ++i) {
        int lineTop = i * lineSpacing() - static_cast<int>(m_scrollOffset);
        int lineBottom = lineTop + lineSpacing();

        // Skip lines outside visible viewport
        if (lineBottom < -20 || lineTop > h + 20) continue;

        QRect lineRect(16, lineTop, w - 32, lineSpacing());

        bool isActive = (i == m_activeCueIndex);
        bool isPast = (i < m_activeCueIndex);
        bool isHovered = (i == m_hoveredIndex);

        // Active background highlight row
        if (isActive) {
            QRect highlightRect(4, lineTop, w - 8, lineSpacing());
            p.fillRect(highlightRect, QColor(30, 30, 36, 180));
            p.setPen(QPen(BrutalistTheme::ACCENT_SIGNAL_LIME, 1));
            p.drawRect(highlightRect);
        } else if (isHovered) {
            QRect hoverRect(4, lineTop, w - 8, lineSpacing());
            p.fillRect(hoverRect, QColor(20, 20, 24, 120));
        }

        // Timestamp string
        qint64 cueMs = m_cues[i].timestampMs;
        qint64 mm = (cueMs / 1000) / 60;
        qint64 ss = (cueMs / 1000) % 60;
        qint64 cs = (cueMs % 1000) / 10;
        QString tsStr = QString("[%1:%2.%3]")
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0'))
            .arg(cs, 2, 10, QChar('0'));

        p.setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
        if (isActive) {
            p.setPen(BrutalistTheme::ACCENT_SIGNAL_LIME);
        } else if (isPast) {
            p.setPen(BrutalistTheme::TEXT_MUTED);
        } else {
            p.setPen(BrutalistTheme::TEXT_SECONDARY_DIM);
        }
        p.drawText(QRect(12, lineTop, 75, lineSpacing()), Qt::AlignVCenter | Qt::AlignLeft, tsStr);

        // Active Marker arrow
        if (isActive) {
            p.setFont(BrutalistTheme::monospaceFont(9, QFont::Bold));
            p.setPen(BrutalistTheme::ACCENT_SIGNAL_LIME);
            p.drawText(QRect(88, lineTop, 16, lineSpacing()), Qt::AlignVCenter | Qt::AlignCenter, "►");
        }

        // Lyric Text
        QRect textRect(108, lineTop, w - 120, lineSpacing());
        if (isActive) {
            p.setFont(BrutalistTheme::sansFont(11, QFont::Bold));
            p.setPen(BrutalistTheme::ACCENT_SIGNAL_LIME);
        } else if (isPast) {
            p.setFont(BrutalistTheme::sansFont(9, QFont::Normal));
            p.setPen(BrutalistTheme::TEXT_MUTED);
        } else {
            p.setFont(BrutalistTheme::sansFont(10, QFont::Normal));
            p.setPen(BrutalistTheme::TEXT_SECONDARY_DIM);
        }

        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_cues[i].text);
    }
}

void TeleprompterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = cueIndexAtY(event->pos().y());
        if (idx >= 0 && idx < m_cues.size()) {
            qint64 targetMs = m_cues[idx].timestampMs;
            setActiveCueIndex(idx);
            emit seekRequested(targetMs);
        }
    }
}

void TeleprompterWidget::mouseMoveEvent(QMouseEvent *event)
{
    int idx = cueIndexAtY(event->pos().y());
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        if (m_hoveredIndex >= 0 && m_hoveredIndex < m_cues.size()) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
        update();
    }
}

void TeleprompterWidget::leaveEvent(QEvent * /*event*/)
{
    m_hoveredIndex = -1;
    setCursor(Qt::ArrowCursor);
    update();
}

} // namespace UI
} // namespace Penguin
