#ifndef TELEPROMPTERWIDGET_H
#define TELEPROMPTERWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPropertyAnimation>
#include "LrcParser.h"

namespace Penguin {
namespace UI {

class TeleprompterWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double scrollOffset READ scrollOffset WRITE setScrollOffset)

public:
    explicit TeleprompterWidget(QWidget *parent = nullptr);
    virtual ~TeleprompterWidget() = default;

    int activeCueIndex() const { return m_activeCueIndex; }
    int cueCount() const { return m_cues.size(); }
    const QList<Core::LyricCue>& cues() const { return m_cues; }

    double scrollOffset() const { return m_scrollOffset; }
    void setScrollOffset(double offset);

public slots:
    bool loadLrcContent(const QString &lrcContent);
    bool loadLrcFile(const QString &filePath);
    void setLrcParser(const Core::LrcParser &parser);
    void setPositionMs(qint64 positionMs);
    void setActiveCueIndex(int cueIndex);
    void clearLyrics();

signals:
    void seekRequested(qint64 positionMs);
    void activeCueChanged(int cueIndex, const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateScrollTarget(bool animate = true);
    int cueIndexAtY(int y) const;
    int lineSpacing() const { return 36; }

    Core::LrcParser m_parser;
    QList<Core::LyricCue> m_cues;
    int m_activeCueIndex = -1;
    qint64 m_lastPositionMs = 0;

    double m_scrollOffset = 0.0;
    double m_targetScrollOffset = 0.0;
    QPropertyAnimation *m_scrollAnim = nullptr;

    int m_hoveredIndex = -1;
};

} // namespace UI
} // namespace Penguin

#endif // TELEPROMPTERWIDGET_H
