#ifndef VIEWFINDERWIDGET_H
#define VIEWFINDERWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <memory>

#include "PlaybackEngine.h"
#include "TickScrubberWidget.h"
#include "DiagnosticsHUDWidget.h"

namespace Penguin {
namespace UI {

class VideoSurfaceWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoSurfaceWidget(QWidget *parent = nullptr);
    virtual ~VideoSurfaceWidget() = default;

    void setPlaybackEngine(Core::PlaybackEngine *engine);
    void setReticlesVisible(bool visible) { m_showReticles = visible; update(); }
    bool isReticlesVisible() const { return m_showReticles; }

    void setMediaTitle(const QString &title) { m_title = title; update(); }

signals:
    void clicked();
    void doubleClicked();
    void mouseMoved();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void attachWindowId();

    Core::PlaybackEngine *m_engine = nullptr;
    bool m_showReticles = true;
    QString m_title;
    QTimer m_singleClickTimer;
};

class ViewfinderWidget : public QWidget {
    Q_OBJECT

public:
    explicit ViewfinderWidget(Core::PlaybackEngine *engine = nullptr, QWidget *parent = nullptr);
    virtual ~ViewfinderWidget() = default;

    void setPlaybackEngine(Core::PlaybackEngine *engine);
    Core::PlaybackEngine* playbackEngine() const { return m_engine; }

    TickScrubberWidget* scrubberWidget() const { return m_scrubber; }
    DiagnosticsHUDWidget* diagnosticsHUD() const { return m_hud; }
    VideoSurfaceWidget* videoSurface() const { return m_videoSurface; }

    bool isOsdVisible() const;
    bool isReticlesVisible() const;

    void setHeaderAndDocksVisible(bool visible);
    void resetAutohideTimer();

public slots:
    void toggleOsd();
    void toggleReticles();
    void setOsdVisible(bool visible);
    void setReticlesVisible(bool visible);
    void updateUIFromEngine();

signals:
    void switchModeRequested();
    void fullscreenToggleRequested();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onAutohideTimeout();
    void onPlayPauseClicked();
    void onStopClicked();
    void onStepBackwardClicked();
    void onStepForwardClicked();
    void onJumpBackwardClicked();
    void onJumpForwardClicked();
    void onSpeedChanged(const QString &speedStr);
    void onVolumeSliderChanged(int value);
    void onMuteClicked();
    void onAudioTrackChanged(int index);
    void onSubtitleTrackChanged(int index);

    // Engine signal handlers
    void onEnginePlaybackStateChanged(Core::PlaybackState state);
    void onEnginePositionChanged(qint64 posMs, const QString &smpte);
    void onEngineDurationChanged(qint64 durMs, const QString &smpte);
    void onEngineTracksChanged(const QList<Core::TrackInfo> &audioTracks, const QList<Core::TrackInfo> &subTracks);
    void onEngineTelemetryUpdated(const Core::DiagnosticsData &diag);
    void onEngineMetadataChanged(const Core::MediaMetadata &meta);

private:
    void setupUI();
    void connectEngineSignals();

    Core::PlaybackEngine *m_engine = nullptr;

    // Header widgets
    QWidget *m_headerWidget = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_reticleToggleBtn = nullptr;
    QPushButton *m_osdToggleBtn = nullptr;
    QPushButton *m_fsToggleBtn = nullptr;
    QPushButton *m_switchModeBtn = nullptr;

    // Viewport & Overlays
    VideoSurfaceWidget *m_videoSurface = nullptr;
    DiagnosticsHUDWidget *m_hud = nullptr;

    // Scrubber
    TickScrubberWidget *m_scrubber = nullptr;

    // Bottom Dock Controls
    QWidget *m_dockWidget = nullptr;
    QPushButton *m_stepBackBtn = nullptr;
    QPushButton *m_jumpBackBtn = nullptr;
    QPushButton *m_playPauseBtn = nullptr;
    QPushButton *m_jumpFwdBtn = nullptr;
    QPushButton *m_stepFwdBtn = nullptr;
    QPushButton *m_stopBtn = nullptr;
    QPushButton *m_shotBtn = nullptr;
    QPushButton *m_loopBtn = nullptr;
    QPushButton *m_nightBtn = nullptr;
    QComboBox *m_speedCombo = nullptr;
    QComboBox *m_audioTrackCombo = nullptr;
    QComboBox *m_subTrackCombo = nullptr;
    QPushButton *m_muteBtn = nullptr;
    QSlider *m_volumeSlider = nullptr;

    bool m_blockTrackSignals = false;
    bool m_osdVisible = true;
    QTimer m_autohideTimer;
};

} // namespace UI
} // namespace Penguin

#endif // VIEWFINDERWIDGET_H
