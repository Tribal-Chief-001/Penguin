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

    // A-B Looping
    void setLoopPointA() { if (m_backend) m_backend->setLoopPointA(); }
    void setLoopPointB() { if (m_backend) m_backend->setLoopPointB(); }
    void clearLoop() { if (m_backend) m_backend->clearLoop(); }
    qint64 loopPointA() const { return m_backend ? m_backend->loopPointA() : -1; }
    qint64 loopPointB() const { return m_backend ? m_backend->loopPointB() : -1; }
    bool isLoopActive() const { return m_backend ? m_backend->isLoopActive() : false; }

    // Delays
    void setSubtitleDelayMs(int delayMs) { if (m_backend) m_backend->setSubtitleDelayMs(delayMs); }
    void adjustSubtitleDelayMs(int deltaMs) { if (m_backend) m_backend->adjustSubtitleDelayMs(deltaMs); }
    int subtitleDelayMs() const { return m_backend ? m_backend->subtitleDelayMs() : 0; }
    void setAudioDelayMs(int delayMs) { if (m_backend) m_backend->setAudioDelayMs(delayMs); }
    void adjustAudioDelayMs(int deltaMs) { if (m_backend) m_backend->adjustAudioDelayMs(deltaMs); }
    int audioDelayMs() const { return m_backend ? m_backend->audioDelayMs() : 0; }

    // Video Equalizer & Color Science
    void setContrast(int val) { if (m_backend) m_backend->setContrast(val); }
    void setBrightness(int val) { if (m_backend) m_backend->setBrightness(val); }
    void setGamma(int val) { if (m_backend) m_backend->setGamma(val); }
    void setSaturation(int val) { if (m_backend) m_backend->setSaturation(val); }
    void setHue(int val) { if (m_backend) m_backend->setHue(val); }
    void resetVideoEqualizer() { if (m_backend) m_backend->resetVideoEqualizer(); }

    // Filters & Scaling
    void setDebandEnabled(bool enabled) { if (m_backend) m_backend->setDebandEnabled(enabled); }
    bool isDebandEnabled() const { return m_backend ? m_backend->isDebandEnabled() : false; }
    void setSharpen(double val) { if (m_backend) m_backend->setSharpen(val); }
    double sharpen() const { return m_backend ? m_backend->sharpen() : 0.0; }
    void setAspectRatio(const QString &ratio) { if (m_backend) m_backend->setAspectRatio(ratio); }
    QString aspectRatio() const { return m_backend ? m_backend->aspectRatio() : QString("auto"); }
    void setVideoZoom(double zoom) { if (m_backend) m_backend->setVideoZoom(zoom); }
    double videoZoom() const { return m_backend ? m_backend->videoZoom() : 0.0; }

    // Night Mode & Audio Enhancements
    void setNightMode(bool enabled) { if (m_backend) m_backend->setNightMode(enabled); }
    bool isNightMode() const { return m_backend ? m_backend->isNightMode() : false; }
    void setCrossfeedEnabled(bool enabled) { if (m_backend) m_backend->setCrossfeedEnabled(enabled); }
    bool isCrossfeedEnabled() const { return m_backend ? m_backend->isCrossfeedEnabled() : false; }

    // Chapters Navigation
    void nextChapter() { if (m_backend) m_backend->nextChapter(); }
    void previousChapter() { if (m_backend) m_backend->previousChapter(); }
    int chapterCount() const { return m_backend ? m_backend->chapterCount() : 0; }
    int currentChapter() const { return m_backend ? m_backend->currentChapter() : -1; }
    QString currentChapterTitle() const { return m_backend ? m_backend->currentChapterTitle() : QString(); }

    // Dual Subtitles & Language Learning
    void selectSecondarySubtitleTrack(int trackId) { if (m_backend) m_backend->selectSecondarySubtitleTrack(trackId); }
    int selectedSecondarySubtitleTrackId() const { return m_backend ? m_backend->selectedSecondarySubtitleTrackId() : -1; }
    void cycleSecondarySubtitle() { if (m_backend) m_backend->cycleSecondarySubtitle(); }

    // Pitch Shifting (Musical Semitones)
    void setPitch(double semitones) { if (m_backend) m_backend->setPitch(semitones); }
    double pitch() const { return m_backend ? m_backend->pitch() : 0.0; }
    void adjustPitch(double deltaSemitones) { if (m_backend) m_backend->adjustPitch(deltaSemitones); }

    // Bookmarks & Markers
    void addBookmark(qint64 positionMs = -1, const QString &label = QString()) { if (m_backend) m_backend->addBookmark(positionMs, label); }
    void removeBookmark(int index) { if (m_backend) m_backend->removeBookmark(index); }
    void clearBookmarks() { if (m_backend) m_backend->clearBookmarks(); }
    QList<qint64> bookmarks() const { return m_backend ? m_backend->bookmarks() : QList<qint64>(); }
    void nextBookmark() { if (m_backend) m_backend->nextBookmark(); }
    void previousBookmark() { if (m_backend) m_backend->previousBookmark(); }

    // Forensic Screenshots
    bool takeScreenshot(const QString &path = QString(), bool includeSubs = false) {
        return m_backend && m_backend->takeScreenshot(path, includeSubs);
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
