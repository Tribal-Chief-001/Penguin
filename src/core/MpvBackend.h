#ifndef MPVBACKEND_H
#define MPVBACKEND_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

namespace Penguin {
namespace Core {

enum class TrackType {
    Video,
    Audio,
    Subtitle
};

struct TrackInfo {
    int id = -1;
    TrackType type = TrackType::Video;
    QString title;
    QString language;
    QString codec;
    int channels = 0;       // For audio
    int sampleRate = 0;     // For audio
    int bitRate = 0;        // In bps
    int width = 0;          // For video
    int height = 0;         // For video
    double fps = 0.0;       // For video
    bool isDefault = false;
    bool isSelected = false;
    bool isExternal = false;
    QString sourcePath;
};

struct DiagnosticsData {
    double fps = 0.0;
    double nominalFps = 30.0;
    qint64 droppedFrames = 0;
    qint64 videoBitrate = 0; // bps
    qint64 audioBitrate = 0; // bps
    int videoWidth = 0;
    int videoHeight = 0;
    QString videoCodec;
    QString audioCodec;
    int audioSampleRate = 0;
    int audioChannels = 0;
    double renderTimeMs = 0.0;
    double avSkewMs = 0.0;
    QString colorSpace = "BT.709";
    int bitDepth = 8;
};

struct MediaMetadata {
    QString title;
    QString artist;
    QString album;
    QString albumArtist;
    QString genre;
    int trackNumber = 0;
    int year = 0;
    QString url;
    qint64 durationMs = 0;
    QString format;
    bool hasVideo = false;
    bool hasAudio = false;
};

enum class EngineState {
    Idle,
    Loading,
    Playing,
    Paused,
    Stopped,
    Error
};

class MpvBackend : public QObject {
    Q_OBJECT
public:
    explicit MpvBackend(QObject *parent = nullptr);
    virtual ~MpvBackend();

    // Initialization
    bool initialize(bool offscreen = true);
    void shutdown();
    bool isInitialized() const { return m_mpv != nullptr; }
    mpv_handle *handle() const { return m_mpv; }

    // Media Control
    bool loadFile(const QString &uri, bool autoPlay = true);
    void play();
    void pause();
    void togglePlayPause();
    void stop();

    // Navigation
    void seekAbsoluteMs(qint64 positionMs);
    void seekRelativeMs(qint64 offsetMs);
    void frameStepForward();
    void frameStepBackward();

    // Speed & Audio
    void setSpeed(double speed);
    double speed() const { return m_speed; }

    void setVolume(int volume);
    int volume() const { return m_volume; }

    void setMuted(bool mute);
    bool isMuted() const { return m_muted; }

    void setAudioFilter(const QString &filterString);

    // Stream & Subtitle Management
    void setAudioTrack(int trackId);
    void setSubtitleTrack(int trackId);
    void setVideoTrack(int trackId);
    bool loadExternalSubtitle(const QString &filePath);

    // Properties & State
    EngineState state() const { return m_state; }
    qint64 positionMs() const { return m_positionMs; }
    qint64 durationMs() const { return m_durationMs; }
    QString currentUri() const { return m_currentUri; }
    const MediaMetadata& metadata() const { return m_metadata; }
    const DiagnosticsData& diagnostics() const { return m_diagnostics; }
    const QList<TrackInfo>& audioTracks() const { return m_audioTracks; }
    const QList<TrackInfo>& subtitleTracks() const { return m_subtitleTracks; }
    const QList<TrackInfo>& videoTracks() const { return m_videoTracks; }
    int selectedAudioTrackId() const { return m_selectedAid; }
    int selectedSubtitleTrackId() const { return m_selectedSid; }

    // Command execution
    int command(const QStringList &args);
    int setProperty(const char *name, mpv_format format, void *data);
    int setPropertyString(const char *name, const char *data);
    int getProperty(const char *name, mpv_format format, void *data);
    QString getPropertyString(const char *name);

    // Native Window Embedding & Rendering
    void setWindowId(int64_t wid);
    int64_t windowId() const { return m_wid; }
    bool initializeRenderContext(void *(*getProcAddress)(void *, const char *), void *ctx);
    void freeRenderContext();
    void renderGL(int fbo, int width, int height, bool flipY = true);
    mpv_render_context *renderContext() const { return m_renderContext; }

signals:
    void stateChanged(EngineState newState);
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void speedChanged(double speed);
    void volumeChanged(int volume);
    void muteChanged(bool muted);
    void metadataChanged(const MediaMetadata &metadata);
    void tracksChanged(const QList<TrackInfo> &audioTracks, const QList<TrackInfo> &subTracks);
    void diagnosticsChanged(const DiagnosticsData &diagnostics);
    void endOfFileReached();
    void errorOccurred(const QString &errorMessage);
    void frameRenderNeeded();

public slots:
    void processEvents();

private:
    static void onMpvWakeup(void *ctx);
    void handleMpvPropertyChange(mpv_event_property *prop);
    void parseTrackList(const mpv_node *node);
    void parseMetadata(const mpv_node *node);
    void updateDiagnostics();
    void setState(EngineState newState);

    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_renderContext = nullptr;
    int64_t m_wid = 0;
    EngineState m_state = EngineState::Idle;
    QString m_currentUri;
    qint64 m_positionMs = 0;
    qint64 m_durationMs = 0;
    double m_speed = 1.0;
    int m_volume = 100;
    bool m_muted = false;
    bool m_isPaused = false;
    int m_selectedAid = 1;
    int m_selectedSid = -1;
    int m_selectedVid = 1;

    QList<TrackInfo> m_audioTracks;
    QList<TrackInfo> m_subtitleTracks;
    QList<TrackInfo> m_videoTracks;
    MediaMetadata m_metadata;
    DiagnosticsData m_diagnostics;
};

} // namespace Core
} // namespace Penguin

#endif // MPVBACKEND_H
