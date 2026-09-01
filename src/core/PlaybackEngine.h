#ifndef PLAYBACKENGINE_H
#define PLAYBACKENGINE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QTimer>
#include <memory>

#include "MpvBackend.h"
#include "EqualizerDSP.h"
#include "VUMeterDSP.h"
#include "TimecodeFormatter.h"
#include "LrcParser.h"
#include "SubtitleLoader.h"

namespace Penguin {
namespace Core {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused,
    Buffering,
    Error
};

class PlaybackEngine : public QObject {
    Q_OBJECT
public:
    explicit PlaybackEngine(QObject *parent = nullptr);
    virtual ~PlaybackEngine();

    // Initialization
    bool initialize(bool offscreen = true);
    void shutdown();
    bool isInitialized() const;

    // Media Controls
    bool loadMedia(const QString &uri, bool autoPlay = true);
    void play();
    void pause();
    void togglePlayPause();
    void stop();

    // Seeking & Stepping
    void seek(qint64 positionMs);
    void seekRelative(qint64 offsetMs);
    void frameStep(int direction); // +1 = forward, -1 = backward

    // Audio & Rate Controls
    void setSpeed(double rate);
    double speed() const;

    void setVolume(int volume);
    int volume() const;

    void setMuted(bool mute);
    bool isMuted() const;

    // Equalizer Subsystem
    void setEqualizerBand(int bandIndex, double gainDb);
    double equalizerBand(int bandIndex) const;
    void setEqualizerPreset(const QString &presetName);
    QString currentEqualizerPreset() const;
    void resetEqualizer();
    const EqualizerDSP& equalizer() const { return m_equalizer; }
    void applyEqualizerToEngine();

    // Stream & Subtitle Management
    void setAudioTrack(int trackId);
    void setSubtitleTrack(int trackId);
    bool loadExternalSubtitle(const QString &filePath);
    bool loadExternalLrc(const QString &filePath);

    // State & Metadata Getters
    PlaybackState playbackState() const { return m_playbackState; }
    qint64 positionMs() const { return m_backend ? m_backend->positionMs() : 0; }
    qint64 durationMs() const { return m_backend ? m_backend->durationMs() : 0; }
    QString smptePosition() const;
    QString smpteDuration() const;
    QString smpteRemaining() const;

    QString currentUri() const { return m_backend ? m_backend->currentUri() : QString(); }
    const MediaMetadata& metadata() const { return m_backend ? m_backend->metadata() : m_emptyMetadata; }
    const DiagnosticsData& telemetry() const { return m_backend ? m_backend->diagnostics() : m_emptyDiagnostics; }
    const QList<TrackInfo>& audioTracks() const { return m_backend ? m_backend->audioTracks() : m_emptyTracks; }
    const QList<TrackInfo>& subtitleTracks() const { return m_backend ? m_backend->subtitleTracks() : m_emptyTracks; }
    const VUMeterDSP& vuMeter() const { return m_vuMeter; }
    const LrcParser& lrcParser() const { return m_lrcParser; }
    const SubtitleLoader& subtitleLoader() const { return m_subtitleLoader; }

    // Direct access to lower-level backend for video rendering surface
    MpvBackend* backend() const { return m_backend.get(); }
    void setWindowId(int64_t wid) {
        if (m_backend) m_backend->setWindowId(wid);
    }
    void renderGL(int fbo, int width, int height, bool flipY = true) {
        if (m_backend) m_backend->renderGL(fbo, width, height, flipY);
    }
    bool initializeRenderContext(void *(*getProcAddress)(void *, const char *), void *ctx) {
        return m_backend && m_backend->initializeRenderContext(getProcAddress, ctx);
    }

signals:
    void playbackStateChanged(PlaybackState state);
    void positionChanged(qint64 positionMs, const QString &smpteTimecode);
    void durationChanged(qint64 durationMs, const QString &smpteTimecode);
    void speedChanged(double speed);
    void volumeChanged(int volume);
    void muteChanged(bool muted);
    void tracksChanged(const QList<TrackInfo> &audioTracks, const QList<TrackInfo> &subTracks);
    void metadataChanged(const MediaMetadata &metadata);
    void telemetryUpdated(const DiagnosticsData &telemetry);
    void vuLevelsChanged(double leftPeakDb, double rightPeakDb, double leftRmsDb, double rightRmsDb);
    void equalizerChanged(const EqualizerDSP &eq);
    void activeLyricChanged(int cueIndex, const QString &lyricText);
    void activeSubtitleChanged(const QString &subtitleText);
    void mediaLoaded(const QString &uri);
    void mediaFinished();
    void errorOccurred(const QString &errorMessage);

private slots:
    void onBackendStateChanged(EngineState state);
    void onBackendPositionChanged(qint64 positionMs);
    void onBackendDurationChanged(qint64 durationMs);
    void onBackendMetadataChanged(const MediaMetadata &meta);
    void onBackendTracksChanged(const QList<TrackInfo> &audioTracks, const QList<TrackInfo> &subTracks);
    void onBackendDiagnosticsChanged(const DiagnosticsData &diag);
    void onBackendEndOfFile();
    void onVuTimerTick();

private:
    void autoDiscoverSidecars(const QString &mediaUri);

    std::unique_ptr<MpvBackend> m_backend;
    EqualizerDSP m_equalizer;
    VUMeterDSP m_vuMeter;
    LrcParser m_lrcParser;
    SubtitleLoader m_subtitleLoader;

    PlaybackState m_playbackState = PlaybackState::Stopped;
    QTimer m_vuTimer;

    int m_lastLyricIndex = -2;
    QString m_lastSubtitleText;

    // Fallback static objects
    static const MediaMetadata m_emptyMetadata;
    static const DiagnosticsData m_emptyDiagnostics;
    static const QList<TrackInfo> m_emptyTracks;
};

} // namespace Core
} // namespace Penguin

#endif // PLAYBACKENGINE_H
