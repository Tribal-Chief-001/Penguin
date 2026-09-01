#include "PlaybackEngine.h"
#include <QFileInfo>
#include <QDir>
#include <cmath>

namespace Penguin {
namespace Core {

const MediaMetadata PlaybackEngine::m_emptyMetadata;
const DiagnosticsData PlaybackEngine::m_emptyDiagnostics;
const QList<TrackInfo> PlaybackEngine::m_emptyTracks;

PlaybackEngine::PlaybackEngine(QObject *parent)
    : QObject(parent)
    , m_backend(std::make_unique<MpvBackend>(this))
{
    // Connect backend signals
    connect(m_backend.get(), &MpvBackend::stateChanged, this, &PlaybackEngine::onBackendStateChanged);
    connect(m_backend.get(), &MpvBackend::positionChanged, this, &PlaybackEngine::onBackendPositionChanged);
    connect(m_backend.get(), &MpvBackend::durationChanged, this, &PlaybackEngine::onBackendDurationChanged);
    connect(m_backend.get(), &MpvBackend::metadataChanged, this, &PlaybackEngine::onBackendMetadataChanged);
    connect(m_backend.get(), &MpvBackend::tracksChanged, this, &PlaybackEngine::onBackendTracksChanged);
    connect(m_backend.get(), &MpvBackend::diagnosticsChanged, this, &PlaybackEngine::onBackendDiagnosticsChanged);
    connect(m_backend.get(), &MpvBackend::endOfFileReached, this, &PlaybackEngine::onBackendEndOfFile);
    connect(m_backend.get(), &MpvBackend::speedChanged, this, &PlaybackEngine::speedChanged);
    connect(m_backend.get(), &MpvBackend::volumeChanged, this, &PlaybackEngine::volumeChanged);
    connect(m_backend.get(), &MpvBackend::muteChanged, this, &PlaybackEngine::muteChanged);
    connect(m_backend.get(), &MpvBackend::errorOccurred, this, &PlaybackEngine::errorOccurred);

    // Setup 60Hz VU meter simulation & physics timer
    connect(&m_vuTimer, &QTimer::timeout, this, &PlaybackEngine::onVuTimerTick);
    m_vuTimer.setInterval(16); // ~60fps
}

PlaybackEngine::~PlaybackEngine()
{
    shutdown();
}

bool PlaybackEngine::initialize(bool offscreen)
{
    bool ok = m_backend->initialize(offscreen);
    if (ok) {
        m_vuTimer.start();
    }
    return ok;
}

void PlaybackEngine::shutdown()
{
    m_vuTimer.stop();
    if (m_backend) {
        m_backend->shutdown();
    }
    m_playbackState = PlaybackState::Stopped;
}

bool PlaybackEngine::isInitialized() const
{
    return m_backend && m_backend->isInitialized();
}

void PlaybackEngine::autoDiscoverSidecars(const QString &mediaUri)
{
    QFileInfo fi(mediaUri);
    if (!fi.exists()) return;

    QString baseDir = fi.absolutePath();
    QString baseName = fi.completeBaseName();

    // Check for sidecar .lrc file
    QString lrcPath = QDir(baseDir).filePath(baseName + ".lrc");
    if (QFileInfo::exists(lrcPath)) {
        loadExternalLrc(lrcPath);
    } else {
        m_lrcParser.clear();
    }

    // Check for sidecar .srt / .ass / .vtt
    QStringList subExts = {"srt", "ass", "ssa", "vtt"};
    for (const QString &ext : subExts) {
        QString subPath = QDir(baseDir).filePath(baseName + "." + ext);
        if (QFileInfo::exists(subPath)) {
            loadExternalSubtitle(subPath);
            break;
        }
    }
}

bool PlaybackEngine::loadMedia(const QString &uri, bool autoPlay)
{
    if (!isInitialized()) {
        initialize(false);
    }

    autoDiscoverSidecars(uri);
    bool ok = m_backend->loadFile(uri, autoPlay);
    if (ok) {
        applyEqualizerToEngine();
        emit mediaLoaded(uri);
    }
    return ok;
}

void PlaybackEngine::play()
{
    if (m_backend) {
        m_backend->play();
    }
}

void PlaybackEngine::pause()
{
    if (m_backend) {
        m_backend->pause();
    }
}

void PlaybackEngine::togglePlayPause()
{
    if (m_backend) {
        m_backend->togglePlayPause();
    }
}

void PlaybackEngine::stop()
{
    if (m_backend) {
        m_backend->stop();
    }
    m_vuMeter.reset();
}

void PlaybackEngine::seek(qint64 positionMs)
{
    if (m_backend) {
        m_backend->seekAbsoluteMs(positionMs);
    }
}

void PlaybackEngine::seekRelative(qint64 offsetMs)
{
    if (m_backend) {
        m_backend->seekRelativeMs(offsetMs);
    }
}

void PlaybackEngine::frameStep(int direction)
{
    if (m_backend) {
        if (direction > 0) {
            m_backend->frameStepForward();
        } else if (direction < 0) {
            m_backend->frameStepBackward();
        }
    }
}

void PlaybackEngine::setSpeed(double rate)
{
    if (m_backend) {
        m_backend->setSpeed(rate);
    }
}

double PlaybackEngine::speed() const
{
    return m_backend ? m_backend->speed() : 1.0;
}

void PlaybackEngine::setVolume(int volume)
{
    if (m_backend) {
        m_backend->setVolume(volume);
    }
}

int PlaybackEngine::volume() const
{
    return m_backend ? m_backend->volume() : 100;
}

void PlaybackEngine::setMuted(bool mute)
{
    if (m_backend) {
        m_backend->setMuted(mute);
    }
}

bool PlaybackEngine::isMuted() const
{
    return m_backend ? m_backend->isMuted() : false;
}

void PlaybackEngine::setAudioTrack(int trackId)
{
    if (m_backend) {
        m_backend->setAudioTrack(trackId);
    }
}

void PlaybackEngine::setSubtitleTrack(int trackId)
{
    if (m_backend) {
        m_backend->setSubtitleTrack(trackId);
    }
}

bool PlaybackEngine::loadExternalSubtitle(const QString &filePath)
{
    m_subtitleLoader.loadFromFile(filePath);
    if (m_backend) {
        return m_backend->loadExternalSubtitle(filePath);
    }
    return false;
}

bool PlaybackEngine::loadExternalLrc(const QString &filePath)
{
    return m_lrcParser.loadFromFile(filePath);
}

void PlaybackEngine::setEqualizerBand(int bandIndex, double gainDb)
{
    m_equalizer.setBandGain(bandIndex, gainDb);
    applyEqualizerToEngine();
    emit equalizerChanged(m_equalizer);
}

double PlaybackEngine::equalizerBand(int bandIndex) const
{
    return m_equalizer.bandGain(bandIndex);
}

void PlaybackEngine::setEqualizerPreset(const QString &presetName)
{
    m_equalizer.setPreset(presetName);
    applyEqualizerToEngine();
    emit equalizerChanged(m_equalizer);
}

QString PlaybackEngine::currentEqualizerPreset() const
{
    return m_equalizer.currentPreset();
}

void PlaybackEngine::resetEqualizer()
{
    m_equalizer.resetFlat();
    applyEqualizerToEngine();
    emit equalizerChanged(m_equalizer);
}

void PlaybackEngine::applyEqualizerToEngine()
{
    if (m_backend) {
        QString filterStr = m_equalizer.toMpvFilterString();
        m_backend->setAudioFilter(filterStr);
    }
}

QString PlaybackEngine::smptePosition() const
{
    double fps = 30.0;
    if (m_backend && m_backend->diagnostics().nominalFps > 0.0) {
        fps = m_backend->diagnostics().nominalFps;
    }
    return TimecodeFormatter::formatTimecode(positionMs(), fps, TimecodeFormatter::isDropFrameRate(fps));
}

QString PlaybackEngine::smpteDuration() const
{
    double fps = 30.0;
    if (m_backend && m_backend->diagnostics().nominalFps > 0.0) {
        fps = m_backend->diagnostics().nominalFps;
    }
    return TimecodeFormatter::formatTimecode(durationMs(), fps, TimecodeFormatter::isDropFrameRate(fps));
}

QString PlaybackEngine::smpteRemaining() const
{
    double fps = 30.0;
    if (m_backend && m_backend->diagnostics().nominalFps > 0.0) {
        fps = m_backend->diagnostics().nominalFps;
    }
    return TimecodeFormatter::formatRemaining(positionMs(), durationMs(), fps, TimecodeFormatter::isDropFrameRate(fps));
}

void PlaybackEngine::onBackendStateChanged(EngineState state)
{
    PlaybackState newState;
    switch (state) {
    case EngineState::Playing:
        newState = PlaybackState::Playing;
        break;
    case EngineState::Paused:
        newState = PlaybackState::Paused;
        break;
    case EngineState::Loading:
        newState = PlaybackState::Buffering;
        break;
    case EngineState::Stopped:
    case EngineState::Idle:
        newState = PlaybackState::Stopped;
        break;
    case EngineState::Error:
    default:
        newState = PlaybackState::Error;
        break;
    }

    if (m_playbackState != newState) {
        m_playbackState = newState;
        emit playbackStateChanged(m_playbackState);
    }
}

void PlaybackEngine::onBackendPositionChanged(qint64 posMs)
{
    emit positionChanged(posMs, smptePosition());

    // Check synchronized lyrics
    if (!m_lrcParser.isEmpty()) {
        int cueIdx = m_lrcParser.findActiveCueIndex(posMs);
        if (cueIdx != m_lastLyricIndex) {
            m_lastLyricIndex = cueIdx;
            emit activeLyricChanged(cueIdx, m_lrcParser.cueTextAt(cueIdx));
        }
    }

    // Check subtitle cues
    if (m_subtitleLoader.isValid()) {
        QString subText = m_subtitleLoader.activeSubtitleText(posMs);
        if (subText != m_lastSubtitleText) {
            m_lastSubtitleText = subText;
            emit activeSubtitleChanged(subText);
        }
    }
}

void PlaybackEngine::onBackendDurationChanged(qint64 durMs)
{
    emit durationChanged(durMs, smpteDuration());
}

void PlaybackEngine::onBackendMetadataChanged(const MediaMetadata &meta)
{
    emit metadataChanged(meta);
}

void PlaybackEngine::onBackendTracksChanged(const QList<TrackInfo> &audioTracks, const QList<TrackInfo> &subTracks)
{
    emit tracksChanged(audioTracks, subTracks);
}

void PlaybackEngine::onBackendDiagnosticsChanged(const DiagnosticsData &diag)
{
    emit telemetryUpdated(diag);
}

void PlaybackEngine::onBackendEndOfFile()
{
    emit mediaFinished();
}

void PlaybackEngine::onVuTimerTick()
{
    double dt = 0.016; // 16 ms
    if (m_playbackState == PlaybackState::Playing && !isMuted() && volume() > 0) {
        // Calculate nominal peak targets based on volume and EQ gain
        double volScale = volume() / 100.0;
        double targetDb = -12.0 + 20.0 * std::log10(std::max(0.01, volScale));
        // Add dynamic musical variation
        static double phase = 0.0;
        phase += 0.15;
        double lVar = 4.0 * std::sin(phase) + 2.0 * std::sin(phase * 2.3);
        double rVar = 4.0 * std::cos(phase * 1.1) + 2.0 * std::sin(phase * 1.7);

        m_vuMeter.simulateSignal(targetDb + lVar, targetDb + rVar, dt);
    } else {
        m_vuMeter.updateTick(dt);
    }

    auto levels = m_vuMeter.currentLevels();
    emit vuLevelsChanged(levels.leftPeakDb, levels.rightPeakDb, levels.leftRmsDb, levels.rightRmsDb);
}

} // namespace Core
} // namespace Penguin
