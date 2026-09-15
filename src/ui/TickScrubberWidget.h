#ifndef TICKSCRUBBERWIDGET_H
#define TICKSCRUBBERWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QToolTip>

namespace Penguin {
namespace UI {

struct ChapterMarker {
    qint64 timestampMs = 0;
    QString title;
};

class TickScrubberWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qint64 positionMs READ positionMs WRITE setPositionMs NOTIFY positionChanged)
    Q_PROPERTY(qint64 durationMs READ durationMs WRITE setDurationMs NOTIFY durationChanged)

public:
    explicit TickScrubberWidget(QWidget *parent = nullptr);
    virtual ~TickScrubberWidget() = default;

    qint64 positionMs() const { return m_positionMs; }
    qint64 durationMs() const { return m_durationMs; }
    double fps() const { return m_fps; }
    bool isDropFrame() const { return m_dropFrame; }
    bool isScrubbing() const { return m_isDragging; }

    const QList<ChapterMarker>& chapters() const { return m_chapters; }

public slots:
    void setPositionMs(qint64 posMs);
    void setDurationMs(qint64 durationMs);
    void setFps(double fps);
    void setDropFrame(bool dropFrame);
    void setChapters(const QList<ChapterMarker> &chapters);
    void addChapter(qint64 timestampMs, const QString &title);
    void clearChapters();

signals:
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void seekRequested(qint64 positionMs);
    void scrubbingStarted();
    void scrubbingEnded();
    void hoverPositionChanged(qint64 positionMs);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    qint64 xToPositionMs(int x) const;
    int positionMsToX(qint64 ms) const;
    QRect trackRect() const;
    qint64 applyMagneticSnap(int x, qint64 rawMs) const;

    qint64 m_positionMs = 0;
    qint64 m_durationMs = 0;
    double m_fps = 30.0;
    bool m_dropFrame = false;
    QList<ChapterMarker> m_chapters;

    bool m_isDragging = false;
    bool m_isHovered = false;
    int m_hoverX = 0;
    mutable int m_snappedChapterIndex = -1;
    mutable bool m_isMagneticSnapped = false;
};

} // namespace UI
} // namespace Penguin

#endif // TICKSCRUBBERWIDGET_H
