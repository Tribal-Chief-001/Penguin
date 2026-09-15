#ifndef AUDIODECKWIDGET_H
#define AUDIODECKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>

#include "PlaybackEngine.h"
#include "VUMeterWidget.h"
#include "EqualizerRackWidget.h"
#include "TeleprompterWidget.h"
#include "PlaylistMatrixWidget.h"
#include "TickScrubberWidget.h"

namespace Penguin {
namespace Library {
class PlaylistManager;
}

namespace UI {

enum class RepeatMode {
    Off,
    RepeatAll,
    RepeatOne
};

class KineticDeckVisualizer : public QLabel {
    Q_OBJECT

public:
    explicit KineticDeckVisualizer(QWidget *parent = nullptr);
    virtual ~KineticDeckVisualizer() = default;

    void setPlaybackState(Core::PlaybackState state);
    void setAudioLevels(double lPeak, double rPeak, double lRms, double rRms);
    void reset();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onAnimationTick();

private:
    Core::PlaybackState m_state = Core::PlaybackState::Stopped;
    double m_rotationAngle = 0.0;
    double m_rotationSpeed = 0.0;
    double m_leftPeakDb = -60.0;
    double m_rightPeakDb = -60.0;
    double m_leftRmsDb = -60.0;
    double m_rightRmsDb = -60.0;
    double m_smoothedEnergy = 0.0;
    double m_pulsePhase = 0.0;
    QTimer *m_animTimer = nullptr;
};

class AudioDeckWidget : public QWidget {
    Q_OBJECT

public:
    explicit AudioDeckWidget(Core::PlaybackEngine *engine = nullptr, QWidget *parent = nullptr);
    virtual ~AudioDeckWidget() = default;

    void setPlaybackEngine(Core::PlaybackEngine *engine);
    Core::PlaybackEngine* playbackEngine() const { return m_engine; }

    void setPlaylistManager(Library::PlaylistManager *playlistMgr);
    Library::PlaylistManager* playlistManager() const { return m_playlistMgr; }

    VUMeterWidget* vuMeter() const { return m_vuMeter; }
    EqualizerRackWidget* equalizerRack() const { return m_eqRack; }
    TeleprompterWidget* teleprompter() const { return m_teleprompter; }
    PlaylistMatrixWidget* playlistMatrix() const { return m_playlistMatrix; }
    TickScrubberWidget* scrubber() const { return m_scrubber; }

    RepeatMode repeatMode() const { return m_repeatMode; }
    bool isShuffleEnabled() const { return m_shuffleEnabled; }

public slots:
    void updateUIFromEngine();
    void playTrackAtIndex(int index);
    void nextTrack();
    void previousTrack();
    void toggleShuffle();
    void cycleRepeatMode();

signals:
    void switchModeRequested();

private slots:
    void onPlayPauseClicked();
    void onJumpBackwardClicked();
    void onJumpForwardClicked();
    void onVolumeSliderChanged(int value);
    void onMuteClicked();

    // Engine signal handlers
    void onEnginePlaybackStateChanged(Core::PlaybackState state);
    void onEnginePositionChanged(qint64 posMs, const QString &smpte);
    void onEngineDurationChanged(qint64 durMs, const QString &smpte);
    void onEngineMetadataChanged(const Core::MediaMetadata &meta);
    void onEngineVuLevelsChanged(double lPeak, double rPeak, double lRms, double rRms);
    void onEngineActiveLyricChanged(int cueIndex, const QString &lyricText);
    void onEngineMediaFinished();

    // Playlist handlers
    void onPlaylistTrackDoubleClicked(int index, const PlaylistItem &item);

private:
    void setupUI();
    void connectEngineSignals();
    void updateTrackCounter();

    Core::PlaybackEngine *m_engine = nullptr;

    // Metadata Masthead Labels
    QLabel *m_trackCounterLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_artistLabel = nullptr;
    QLabel *m_albumLabel = nullptr;
    QLabel *m_formatBadge = nullptr;
    KineticDeckVisualizer *m_coverArtBox = nullptr;

    // Racks & Viewports
    VUMeterWidget *m_vuMeter = nullptr;
    EqualizerRackWidget *m_eqRack = nullptr;
    TeleprompterWidget *m_teleprompter = nullptr;
    PlaylistMatrixWidget *m_playlistMatrix = nullptr;
    TickScrubberWidget *m_scrubber = nullptr;

    // Bottom Dock Controls
    QPushButton *m_prevBtn = nullptr;
    QPushButton *m_jumpBackBtn = nullptr;
    QPushButton *m_playPauseBtn = nullptr;
    QPushButton *m_jumpFwdBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;
    QPushButton *m_shuffleBtn = nullptr;
    QPushButton *m_repeatBtn = nullptr;
    QPushButton *m_muteBtn = nullptr;
    QSlider *m_volumeSlider = nullptr;

    RepeatMode m_repeatMode = RepeatMode::Off;
    bool m_shuffleEnabled = false;
    Library::PlaylistManager *m_playlistMgr = nullptr;
};

} // namespace UI
} // namespace Penguin

#endif // AUDIODECKWIDGET_H
