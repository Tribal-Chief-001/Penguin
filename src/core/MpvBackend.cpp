#include "MpvBackend.h"
#include <QMetaObject>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <clocale>
#include <cmath>
#include <algorithm>

namespace Penguin {
namespace Core {

MpvBackend::MpvBackend(QObject *parent)
    : QObject(parent)
{
}

MpvBackend::~MpvBackend()
{
    shutdown();
}

void MpvBackend::onMpvWakeup(void *ctx)
{
    auto backend = static_cast<MpvBackend*>(ctx);
    if (backend) {
        QMetaObject::invokeMethod(backend, "processEvents", Qt::QueuedConnection);
    }
}

bool MpvBackend::initialize(bool offscreen)
{
    if (m_mpv) return true;

    // mpv requires LC_NUMERIC to be "C"
    setlocale(LC_NUMERIC, "C");

    m_mpv = mpv_create();
    if (!m_mpv) {
        emit errorOccurred("Failed to create libmpv instance");
        return false;
    }

    // Set engine options
    mpv_set_option_string(m_mpv, "ytdl", "yes");
    mpv_set_option_string(m_mpv, "audio-pitch-correction", "yes");
    mpv_set_option_string(m_mpv, "keep-open", "yes");
    mpv_set_option_string(m_mpv, "terminal", "no");
    mpv_set_option_string(m_mpv, "audio-display", "no");
    mpv_set_option_string(m_mpv, "hwdec", "auto-safe");
    mpv_set_option_string(m_mpv, "volume-max", "100");

    if (offscreen) {
        mpv_set_option_string(m_mpv, "vo", "null");
    } else {
        if (m_wid != 0) {
            mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &m_wid);
            mpv_set_option_string(m_mpv, "vo", "gpu,x11,libmpv,null");
        } else {
            mpv_set_option_string(m_mpv, "vo", "gpu,x11,libmpv,null");
        }
    }

    int err = mpv_initialize(m_mpv);
    if (err < 0) {
        emit errorOccurred(QString("mpv_initialize failed: %1").arg(mpv_error_string(err)));
        mpv_destroy(m_mpv);
        m_mpv = nullptr;
        return false;
    }

    mpv_set_wakeup_callback(m_mpv, onMpvWakeup, this);

    // Observe core properties
    mpv_observe_property(m_mpv, 1, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 2, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 3, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 4, "speed", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 5, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 6, "mute", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 7, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 8, "metadata", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 9, "eof-reached", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 10, "core-idle", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 11, "estimated-vf-fps", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 12, "frame-drop-count", MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 13, "video-bitrate", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 14, "audio-bitrate", MPV_FORMAT_DOUBLE);

    setState(EngineState::Idle);
    return true;
}

void MpvBackend::shutdown()
{
    if (m_renderContext) {
        freeRenderContext();
    }
    if (m_mpv) {
        mpv_set_wakeup_callback(m_mpv, nullptr, nullptr);
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
    setState(EngineState::Stopped);
}

void MpvBackend::setState(EngineState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(m_state);
    }
}

bool MpvBackend::loadFile(const QString &uri, bool autoPlay)
{
    if (!m_mpv && !initialize(false)) {
        return false;
    }

    m_currentUri = uri;
    m_positionMs = 0;
    m_durationMs = 0;
    m_metadata = MediaMetadata();
    m_metadata.url = uri;
    m_metadata.title = QFileInfo(uri).fileName();

    QByteArray pathBytes = uri.toUtf8();
    const char *args[] = {"loadfile", pathBytes.constData(), "replace", nullptr};
    int err = mpv_command(m_mpv, args);
    if (err < 0) {
        emit errorOccurred(QString("Failed to load file: %1").arg(mpv_error_string(err)));
        setState(EngineState::Error);
        return false;
    }

    setState(EngineState::Loading);
    if (!autoPlay) {
        pause();
    } else {
        play();
    }

    return true;
}

void MpvBackend::play()
{
    if (!m_mpv) return;
    int flag = 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
    m_isPaused = false;
    if (m_state != EngineState::Playing && m_state != EngineState::Loading) {
        setState(EngineState::Playing);
    }
}

void MpvBackend::pause()
{
    if (!m_mpv) return;
    int flag = 1;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
    m_isPaused = true;
    setState(EngineState::Paused);
}

void MpvBackend::togglePlayPause()
{
    if (m_isPaused || m_state == EngineState::Paused) {
        play();
    } else {
        pause();
    }
}

void MpvBackend::stop()
{
    if (!m_mpv) return;
    const char *args[] = {"stop", nullptr};
    mpv_command(m_mpv, args);
    m_positionMs = 0;
    setState(EngineState::Stopped);
    emit positionChanged(0);
}

void MpvBackend::seekAbsoluteMs(qint64 positionMs)
{
    if (!m_mpv) return;
    double sec = std::max(0.0, positionMs / 1000.0);
    QByteArray secStr = QByteArray::number(sec, 'f', 3);
    const char *args[] = {"seek", secStr.constData(), "absolute+exact", nullptr};
    mpv_command(m_mpv, args);
}

void MpvBackend::seekRelativeMs(qint64 offsetMs)
{
    if (!m_mpv) return;
    double sec = offsetMs / 1000.0;
    QByteArray secStr = QByteArray::number(sec, 'f', 3);
    const char *args[] = {"seek", secStr.constData(), "relative+exact", nullptr};
    mpv_command(m_mpv, args);
}

void MpvBackend::frameStepForward()
{
    if (!m_mpv) return;
    pause();
    const char *args[] = {"frame-step", nullptr};
    mpv_command(m_mpv, args);
}

void MpvBackend::frameStepBackward()
{
    if (!m_mpv) return;
    pause();
    const char *args[] = {"frame-back-step", nullptr};
    mpv_command(m_mpv, args);
}

void MpvBackend::setSpeed(double speed)
{
    m_speed = std::clamp(speed, 0.5, 2.0);
    if (m_mpv) {
        mpv_set_property(m_mpv, "speed", MPV_FORMAT_DOUBLE, &m_speed);
    }
    emit speedChanged(m_speed);
}

void MpvBackend::setVolume(int volume)
{
    double vol = std::clamp(volume, 0, 100);
    m_volume = static_cast<int>(std::round(vol));
    if (m_mpv) {
        mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    }
    emit volumeChanged(m_volume);
}

void MpvBackend::setMuted(bool mute)
{
    m_muted = mute;
    if (m_mpv) {
        int flag = mute ? 1 : 0;
        mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &flag);
    }
    emit muteChanged(m_muted);
}

void MpvBackend::setAudioFilter(const QString &filterString)
{
    m_equalizerFilter = filterString;
    updateAudioFilters();
}

void MpvBackend::setEqualizerFilter(const QString &eqFilter)
{
    m_equalizerFilter = eqFilter;
    updateAudioFilters();
}

QString MpvBackend::composeAudioFilterGraph() const
{
    QStringList filters;
    if (!m_equalizerFilter.trimmed().isEmpty()) {
        filters.append(m_equalizerFilter.trimmed());
    }
    if (m_nightMode) {
        filters.append("lavfi=[dynaudnorm=f=75:g=15:p=0.95:m=10.0:r=0.9]");
    }
    if (m_crossfeed) {
        filters.append("lavfi=[bs2b=profile=cmoy]");
    }
    if (!qFuzzyIsNull(m_pitch)) {
        double pitchScale = std::pow(2.0, m_pitch / 12.0);
        filters.append(QString("rubberband=pitch-scale=%1").arg(pitchScale, 0, 'f', 4));
    }
    return filters.join(',');
}

void MpvBackend::updateAudioFilters()
{
    if (!m_mpv) return;
    QString graph = composeAudioFilterGraph();
    mpv_set_property_string(m_mpv, "af", graph.toUtf8().constData());
}

void MpvBackend::setAudioTrack(int trackId)
{
    if (!m_mpv) return;
    m_selectedAid = trackId;
    QByteArray aidStr = (trackId >= 0) ? QByteArray::number(trackId) : "no";
    mpv_set_property_string(m_mpv, "aid", aidStr.constData());
}

void MpvBackend::setSubtitleTrack(int trackId)
{
    if (!m_mpv) return;
    m_selectedSid = trackId;
    QByteArray sidStr = (trackId >= 0) ? QByteArray::number(trackId) : "no";
    mpv_set_property_string(m_mpv, "sid", sidStr.constData());
}

void MpvBackend::setVideoTrack(int trackId)
{
    if (!m_mpv) return;
    m_selectedVid = trackId;
    QByteArray vidStr = (trackId >= 0) ? QByteArray::number(trackId) : "no";
    mpv_set_property_string(m_mpv, "vid", vidStr.constData());
}

bool MpvBackend::loadExternalSubtitle(const QString &filePath)
{
    if (!m_mpv || filePath.isEmpty()) return false;
    QByteArray pathBytes = filePath.toUtf8();
    const char *args[] = {"sub-add", pathBytes.constData(), "select", nullptr};
    int err = mpv_command(m_mpv, args);
    return err >= 0;
}

void MpvBackend::setLoopPointA()
{
    m_loopPointA = m_positionMs;
    if (m_loopPointB > 0 && m_loopPointB <= m_loopPointA) {
        m_loopPointB = -1;
    }
    m_loopActive = (m_loopPointA >= 0 && m_loopPointB > m_loopPointA);
    if (m_mpv) {
        double sec = m_loopPointA / 1000.0;
        mpv_set_property(m_mpv, "ab-loop-a", MPV_FORMAT_DOUBLE, &sec);
    }
    emit loopPointsChanged(m_loopPointA, m_loopPointB, m_loopActive);
}

void MpvBackend::setLoopPointB()
{
    if (m_loopPointA < 0) {
        m_loopPointA = 0;
    }
    m_loopPointB = m_positionMs;
    m_loopActive = (m_loopPointB > m_loopPointA);
    if (m_mpv) {
        double sec = m_loopPointB / 1000.0;
        mpv_set_property(m_mpv, "ab-loop-b", MPV_FORMAT_DOUBLE, &sec);
    }
    emit loopPointsChanged(m_loopPointA, m_loopPointB, m_loopActive);
}

void MpvBackend::clearLoop()
{
    m_loopPointA = -1;
    m_loopPointB = -1;
    m_loopActive = false;
    if (m_mpv) {
        mpv_set_property_string(m_mpv, "ab-loop-a", "no");
        mpv_set_property_string(m_mpv, "ab-loop-b", "no");
    }
    emit loopPointsChanged(m_loopPointA, m_loopPointB, m_loopActive);
}

void MpvBackend::setSubtitleDelayMs(int delayMs)
{
    m_subDelayMs = delayMs;
    if (m_mpv) {
        double sec = delayMs / 1000.0;
        mpv_set_property(m_mpv, "sub-delay", MPV_FORMAT_DOUBLE, &sec);
    }
    emit subtitleDelayChanged(m_subDelayMs);
}

void MpvBackend::adjustSubtitleDelayMs(int deltaMs)
{
    setSubtitleDelayMs(m_subDelayMs + deltaMs);
}

void MpvBackend::setAudioDelayMs(int delayMs)
{
    m_audioDelayMs = delayMs;
    if (m_mpv) {
        double sec = delayMs / 1000.0;
        mpv_set_property(m_mpv, "audio-delay", MPV_FORMAT_DOUBLE, &sec);
    }
    emit audioDelayChanged(m_audioDelayMs);
}

void MpvBackend::adjustAudioDelayMs(int deltaMs)
{
    setAudioDelayMs(m_audioDelayMs + deltaMs);
}

void MpvBackend::setContrast(int val)
{
    m_contrast = qBound(-100, val, 100);
    if (m_mpv) {
        int64_t v = m_contrast;
        mpv_set_property(m_mpv, "contrast", MPV_FORMAT_INT64, &v);
    }
    emit videoEqualizerChanged(m_contrast, m_brightness, m_gamma, m_saturation, m_hue);
}

void MpvBackend::setBrightness(int val)
{
    m_brightness = qBound(-100, val, 100);
    if (m_mpv) {
        int64_t v = m_brightness;
        mpv_set_property(m_mpv, "brightness", MPV_FORMAT_INT64, &v);
    }
    emit videoEqualizerChanged(m_contrast, m_brightness, m_gamma, m_saturation, m_hue);
}

void MpvBackend::setGamma(int val)
{
    m_gamma = qBound(-100, val, 100);
    if (m_mpv) {
        int64_t v = m_gamma;
        mpv_set_property(m_mpv, "gamma", MPV_FORMAT_INT64, &v);
    }
    emit videoEqualizerChanged(m_contrast, m_brightness, m_gamma, m_saturation, m_hue);
}

void MpvBackend::setSaturation(int val)
{
    m_saturation = qBound(-100, val, 100);
    if (m_mpv) {
        int64_t v = m_saturation;
        mpv_set_property(m_mpv, "saturation", MPV_FORMAT_INT64, &v);
    }
    emit videoEqualizerChanged(m_contrast, m_brightness, m_gamma, m_saturation, m_hue);
}

void MpvBackend::setHue(int val)
{
    m_hue = qBound(-100, val, 100);
    if (m_mpv) {
        int64_t v = m_hue;
        mpv_set_property(m_mpv, "hue", MPV_FORMAT_INT64, &v);
    }
    emit videoEqualizerChanged(m_contrast, m_brightness, m_gamma, m_saturation, m_hue);
}

void MpvBackend::resetVideoEqualizer()
{
    m_contrast = 0;
    m_brightness = 0;
    m_gamma = 0;
    m_saturation = 0;
    m_hue = 0;
    if (m_mpv) {
        int64_t zero = 0;
        mpv_set_property(m_mpv, "contrast", MPV_FORMAT_INT64, &zero);
        mpv_set_property(m_mpv, "brightness", MPV_FORMAT_INT64, &zero);
        mpv_set_property(m_mpv, "gamma", MPV_FORMAT_INT64, &zero);
        mpv_set_property(m_mpv, "saturation", MPV_FORMAT_INT64, &zero);
        mpv_set_property(m_mpv, "hue", MPV_FORMAT_INT64, &zero);
    }
    emit videoEqualizerChanged(m_contrast, m_brightness, m_gamma, m_saturation, m_hue);
}

void MpvBackend::setDebandEnabled(bool enabled)
{
    m_deband = enabled;
    if (m_mpv) {
        mpv_set_property_string(m_mpv, "deband", enabled ? "yes" : "no");
    }
}

void MpvBackend::setSharpen(double val)
{
    m_sharpen = qBound(-1.0, val, 1.0);
    if (m_mpv) {
        mpv_set_property(m_mpv, "sharpen", MPV_FORMAT_DOUBLE, &m_sharpen);
    }
}

void MpvBackend::setAspectRatio(const QString &ratio)
{
    m_aspectRatio = ratio;
    if (m_mpv) {
        if (ratio == "auto" || ratio.isEmpty()) {
            mpv_set_property_string(m_mpv, "video-aspect-override", "-1");
        } else {
            QByteArray rBytes = ratio.toUtf8();
            mpv_set_property_string(m_mpv, "video-aspect-override", rBytes.constData());
        }
    }
}

void MpvBackend::setVideoZoom(double zoom)
{
    m_zoom = qBound(0.0, zoom, 2.0);
    if (m_mpv) {
        mpv_set_property(m_mpv, "video-zoom", MPV_FORMAT_DOUBLE, &m_zoom);
    }
}

void MpvBackend::setNightMode(bool enabled)
{
    m_nightMode = enabled;
    updateAudioFilters();
    emit nightModeChanged(m_nightMode);
}

bool MpvBackend::takeScreenshot(const QString &targetFilePath, bool includeSubtitles)
{
    if (!m_mpv) return false;
    QString path = targetFilePath;
    if (path.isEmpty()) {
        QString baseDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (baseDir.isEmpty() || !QDir(baseDir).exists()) {
            baseDir = QDir::homePath();
        }
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
        path = QString("%1/penguin_screenshot_%2.png").arg(baseDir, timestamp);
    }

    QByteArray pathBytes = path.toUtf8();
    const char *mode = includeSubtitles ? "subtitles" : "video";
    const char *args[] = {"screenshot-to-file", pathBytes.constData(), mode, nullptr};
    int err = mpv_command(m_mpv, args);
    if (err >= 0) {
        emit screenshotTaken(path);
        return true;
    }
    return false;
}

void MpvBackend::setCrossfeedEnabled(bool enabled)
{
    m_crossfeed = enabled;
    updateAudioFilters();
    emit crossfeedChanged(m_crossfeed);
}

void MpvBackend::nextChapter()
{
    if (!m_mpv) return;
    const char *args[] = {"add", "chapter", "1", nullptr};
    mpv_command(m_mpv, args);
    emit chapterChanged(currentChapter(), currentChapterTitle());
}

void MpvBackend::previousChapter()
{
    if (!m_mpv) return;
    const char *args[] = {"add", "chapter", "-1", nullptr};
    mpv_command(m_mpv, args);
    emit chapterChanged(currentChapter(), currentChapterTitle());
}

int MpvBackend::chapterCount() const
{
    if (!m_mpv) return 0;
    int64_t count = 0;
    if (mpv_get_property(m_mpv, "chapters", MPV_FORMAT_INT64, &count) >= 0) {
        return static_cast<int>(count);
    }
    return 0;
}

int MpvBackend::currentChapter() const
{
    if (!m_mpv) return -1;
    int64_t ch = -1;
    if (mpv_get_property(m_mpv, "chapter", MPV_FORMAT_INT64, &ch) >= 0) {
        return static_cast<int>(ch);
    }
    return -1;
}

QString MpvBackend::currentChapterTitle() const
{
    if (!m_mpv) return QString();
    char *title = nullptr;
    if (mpv_get_property(m_mpv, "chapter-metadata/by-key/title", MPV_FORMAT_STRING, &title) >= 0 && title) {
        QString res = QString::fromUtf8(title);
        mpv_free(title);
        return res;
    }
    int ch = currentChapter();
    return (ch >= 0) ? QString("Chapter %1").arg(ch + 1) : QString();
}

void MpvBackend::selectSecondarySubtitleTrack(int trackId)
{
    m_selectedSecondarySid = trackId;
    if (m_mpv) {
        if (trackId <= 0) {
            mpv_set_property_string(m_mpv, "secondary-sid", "no");
        } else {
            QByteArray trackBytes = QByteArray::number(trackId);
            mpv_set_property_string(m_mpv, "secondary-sid", trackBytes.constData());
        }
    }
    emit secondarySubtitleChanged(m_selectedSecondarySid);
}

void MpvBackend::cycleSecondarySubtitle()
{
    if (m_subtitleTracks.isEmpty()) return;
    if (m_selectedSecondarySid <= 0) {
        selectSecondarySubtitleTrack(m_subtitleTracks.first().id);
    } else {
        int currentIndex = -1;
        for (int i = 0; i < m_subtitleTracks.size(); ++i) {
            if (m_subtitleTracks[i].id == m_selectedSecondarySid) {
                currentIndex = i;
                break;
            }
        }
        if (currentIndex >= 0 && currentIndex + 1 < m_subtitleTracks.size()) {
            selectSecondarySubtitleTrack(m_subtitleTracks[currentIndex + 1].id);
        } else {
            selectSecondarySubtitleTrack(-1); // Off
        }
    }
}

void MpvBackend::setPitch(double semitones)
{
    m_pitch = qBound(-12.0, semitones, 12.0);
    updateAudioFilters();
    emit pitchChanged(m_pitch);
}

void MpvBackend::adjustPitch(double deltaSemitones)
{
    setPitch(m_pitch + deltaSemitones);
}

void MpvBackend::addBookmark(qint64 positionMs, const QString &label)
{
    Q_UNUSED(label);
    qint64 pos = (positionMs >= 0) ? positionMs : m_positionMs;
    if (!m_bookmarks.contains(pos)) {
        m_bookmarks.append(pos);
        std::sort(m_bookmarks.begin(), m_bookmarks.end());
        emit bookmarksChanged(m_bookmarks);
    }
}

void MpvBackend::removeBookmark(int index)
{
    if (index >= 0 && index < m_bookmarks.size()) {
        m_bookmarks.removeAt(index);
        emit bookmarksChanged(m_bookmarks);
    }
}

void MpvBackend::clearBookmarks()
{
    m_bookmarks.clear();
    emit bookmarksChanged(m_bookmarks);
}

void MpvBackend::nextBookmark()
{
    if (m_bookmarks.isEmpty()) return;
    for (qint64 b : m_bookmarks) {
        if (b > m_positionMs + 500) {
            seekAbsoluteMs(b);
            return;
        }
    }
    seekAbsoluteMs(m_bookmarks.first());
}

void MpvBackend::previousBookmark()
{
    if (m_bookmarks.isEmpty()) return;
    for (int i = m_bookmarks.size() - 1; i >= 0; --i) {
        if (m_bookmarks[i] < m_positionMs - 500) {
            seekAbsoluteMs(m_bookmarks[i]);
            return;
        }
    }
    seekAbsoluteMs(m_bookmarks.last());
}

int MpvBackend::command(const QStringList &args)
{
    if (!m_mpv || args.isEmpty()) return -1;
    QVector<QByteArray> storage;
    QVector<const char*> cArgs;
    storage.reserve(args.size());
    cArgs.reserve(args.size() + 1);

    for (const QString &arg : args) {
        storage.append(arg.toUtf8());
        cArgs.append(storage.last().constData());
    }
    cArgs.append(nullptr);

    return mpv_command(m_mpv, cArgs.data());
}

int MpvBackend::setProperty(const char *name, mpv_format format, void *data)
{
    if (!m_mpv) return -1;
    return mpv_set_property(m_mpv, name, format, data);
}

int MpvBackend::setPropertyString(const char *name, const char *data)
{
    if (!m_mpv) return -1;
    return mpv_set_property_string(m_mpv, name, data);
}

int MpvBackend::getProperty(const char *name, mpv_format format, void *data)
{
    if (!m_mpv) return -1;
    return mpv_get_property(m_mpv, name, format, data);
}

QString MpvBackend::getPropertyString(const char *name)
{
    if (!m_mpv) return QString();
    char *val = mpv_get_property_string(m_mpv, name);
    if (!val) return QString();
    QString result = QString::fromUtf8(val);
    mpv_free(val);
    return result;
}

void MpvBackend::parseTrackList(const mpv_node *node)
{
    if (!node || node->format != MPV_FORMAT_NODE_ARRAY) return;

    m_audioTracks.clear();
    m_subtitleTracks.clear();
    m_videoTracks.clear();

    mpv_node_list *list = node->u.list;
    for (int i = 0; i < list->num; ++i) {
        if (list->values[i].format != MPV_FORMAT_NODE_MAP) continue;
        mpv_node_list *map = list->values[i].u.list;

        TrackInfo track;
        QString typeStr;

        for (int k = 0; k < map->num; ++k) {
            QString key = QString::fromUtf8(map->keys[k]);
            const mpv_node &v = map->values[k];

            if (key == "id" && v.format == MPV_FORMAT_INT64) {
                track.id = static_cast<int>(v.u.int64_);
            } else if (key == "type" && v.format == MPV_FORMAT_STRING) {
                typeStr = QString::fromUtf8(v.u.string);
            } else if (key == "title" && v.format == MPV_FORMAT_STRING) {
                track.title = QString::fromUtf8(v.u.string);
            } else if (key == "lang" && v.format == MPV_FORMAT_STRING) {
                track.language = QString::fromUtf8(v.u.string);
            } else if (key == "codec" && v.format == MPV_FORMAT_STRING) {
                track.codec = QString::fromUtf8(v.u.string);
            } else if (key == "selected" && v.format == MPV_FORMAT_FLAG) {
                track.isSelected = (v.u.flag != 0);
            } else if (key == "default" && v.format == MPV_FORMAT_FLAG) {
                track.isDefault = (v.u.flag != 0);
            } else if (key == "external" && v.format == MPV_FORMAT_FLAG) {
                track.isExternal = (v.u.flag != 0);
            } else if (key == "external-filename" && v.format == MPV_FORMAT_STRING) {
                track.sourcePath = QString::fromUtf8(v.u.string);
            } else if (key == "demux-w" && v.format == MPV_FORMAT_INT64) {
                track.width = static_cast<int>(v.u.int64_);
            } else if (key == "demux-h" && v.format == MPV_FORMAT_INT64) {
                track.height = static_cast<int>(v.u.int64_);
            } else if (key == "demux-fps" && v.format == MPV_FORMAT_DOUBLE) {
                track.fps = v.u.double_;
            } else if (key == "demux-samplerate" && v.format == MPV_FORMAT_INT64) {
                track.sampleRate = static_cast<int>(v.u.int64_);
            } else if (key == "demux-channel-count" && v.format == MPV_FORMAT_INT64) {
                track.channels = static_cast<int>(v.u.int64_);
            } else if (key == "demux-bitrate" && v.format == MPV_FORMAT_INT64) {
                track.bitRate = static_cast<int>(v.u.int64_);
            }
        }

        if (typeStr == "audio") {
            track.type = TrackType::Audio;
            if (track.title.isEmpty()) {
                track.title = QString("Audio Track %1 (%2)").arg(track.id).arg(track.language.isEmpty() ? "und" : track.language);
            }
            m_audioTracks.append(track);
            if (track.isSelected) m_selectedAid = track.id;
        } else if (typeStr == "sub") {
            track.type = TrackType::Subtitle;
            if (track.title.isEmpty()) {
                track.title = QString("Subtitle %1 (%2)").arg(track.id).arg(track.language.isEmpty() ? "und" : track.language);
            }
            m_subtitleTracks.append(track);
            if (track.isSelected) m_selectedSid = track.id;
        } else if (typeStr == "video") {
            track.type = TrackType::Video;
            m_videoTracks.append(track);
            if (track.isSelected) m_selectedVid = track.id;
        }
    }

    m_metadata.hasVideo = !m_videoTracks.isEmpty();
    m_metadata.hasAudio = !m_audioTracks.isEmpty();

    updateDiagnostics();
    emit tracksChanged(m_audioTracks, m_subtitleTracks);
}

void MpvBackend::parseMetadata(const mpv_node *node)
{
    if (!node || node->format != MPV_FORMAT_NODE_MAP) return;
    mpv_node_list *map = node->u.list;

    for (int i = 0; i < map->num; ++i) {
        QString key = QString::fromUtf8(map->keys[i]).toLower();
        if (map->values[i].format != MPV_FORMAT_STRING) continue;
        QString val = QString::fromUtf8(map->values[i].u.string).trimmed();

        if (key == "title") m_metadata.title = val;
        else if (key == "artist") m_metadata.artist = val;
        else if (key == "album") m_metadata.album = val;
        else if (key == "album_artist" || key == "albumartist") m_metadata.albumArtist = val;
        else if (key == "genre") m_metadata.genre = val;
        else if (key == "date" || key == "year") m_metadata.year = val.left(4).toInt();
        else if (key == "track") m_metadata.trackNumber = val.split('/').first().toInt();
    }

    emit metadataChanged(m_metadata);
}

void MpvBackend::updateDiagnostics()
{
    for (const auto &vt : m_videoTracks) {
        if (vt.isSelected) {
            m_diagnostics.videoCodec = vt.codec;
            m_diagnostics.videoWidth = vt.width;
            m_diagnostics.videoHeight = vt.height;
            if (vt.fps > 0.0) {
                m_diagnostics.nominalFps = vt.fps;
            }
            if (vt.bitRate > 0) {
                m_diagnostics.videoBitrate = vt.bitRate;
            }
            break;
        }
    }

    for (const auto &at : m_audioTracks) {
        if (at.isSelected) {
            m_diagnostics.audioCodec = at.codec;
            m_diagnostics.audioSampleRate = at.sampleRate;
            m_diagnostics.audioChannels = at.channels;
            if (at.bitRate > 0) {
                m_diagnostics.audioBitrate = at.bitRate;
            }
            break;
        }
    }

    emit diagnosticsChanged(m_diagnostics);
}

void MpvBackend::handleMpvPropertyChange(mpv_event_property *prop)
{
    if (!prop || !prop->name) return;
    QString name = QString::fromUtf8(prop->name);

    if (name == "time-pos" && prop->format == MPV_FORMAT_DOUBLE) {
        double sec = *static_cast<double*>(prop->data);
        qint64 ms = static_cast<qint64>(sec * 1000.0);
        if (std::abs(m_positionMs - ms) >= 10) {
            m_positionMs = ms;
            emit positionChanged(m_positionMs);
        }
    } else if (name == "duration" && prop->format == MPV_FORMAT_DOUBLE) {
        double sec = *static_cast<double*>(prop->data);
        m_durationMs = static_cast<qint64>(sec * 1000.0);
        m_metadata.durationMs = m_durationMs;
        emit durationChanged(m_durationMs);
    } else if (name == "pause" && prop->format == MPV_FORMAT_FLAG) {
        int paused = *static_cast<int*>(prop->data);
        m_isPaused = (paused != 0);
        if (m_isPaused) {
            setState(EngineState::Paused);
        } else if (m_state != EngineState::Loading && m_state != EngineState::Stopped) {
            setState(EngineState::Playing);
        }
    } else if (name == "speed" && prop->format == MPV_FORMAT_DOUBLE) {
        m_speed = *static_cast<double*>(prop->data);
        emit speedChanged(m_speed);
    } else if (name == "volume" && prop->format == MPV_FORMAT_DOUBLE) {
        m_volume = static_cast<int>(std::round(*static_cast<double*>(prop->data)));
        emit volumeChanged(m_volume);
    } else if (name == "mute" && prop->format == MPV_FORMAT_FLAG) {
        m_muted = (*static_cast<int*>(prop->data) != 0);
        emit muteChanged(m_muted);
    } else if (name == "track-list" && prop->format == MPV_FORMAT_NODE) {
        parseTrackList(static_cast<mpv_node*>(prop->data));
    } else if (name == "metadata" && prop->format == MPV_FORMAT_NODE) {
        parseMetadata(static_cast<mpv_node*>(prop->data));
    } else if (name == "eof-reached" && prop->format == MPV_FORMAT_FLAG) {
        int eof = *static_cast<int*>(prop->data);
        if (eof) {
            emit endOfFileReached();
        }
    } else if (name == "estimated-vf-fps" && prop->format == MPV_FORMAT_DOUBLE) {
        m_diagnostics.fps = *static_cast<double*>(prop->data);
        emit diagnosticsChanged(m_diagnostics);
    } else if (name == "frame-drop-count" && prop->format == MPV_FORMAT_INT64) {
        m_diagnostics.droppedFrames = *static_cast<int64_t*>(prop->data);
        emit diagnosticsChanged(m_diagnostics);
    } else if (name == "video-bitrate" && prop->format == MPV_FORMAT_DOUBLE) {
        m_diagnostics.videoBitrate = static_cast<qint64>(*static_cast<double*>(prop->data));
        emit diagnosticsChanged(m_diagnostics);
    } else if (name == "audio-bitrate" && prop->format == MPV_FORMAT_DOUBLE) {
        m_diagnostics.audioBitrate = static_cast<qint64>(*static_cast<double*>(prop->data));
        emit diagnosticsChanged(m_diagnostics);
    }
}

void MpvBackend::processEvents()
{
    if (!m_mpv) return;

    while (m_mpv) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (!event || event->event_id == MPV_EVENT_NONE) break;

        switch (event->event_id) {
        case MPV_EVENT_PROPERTY_CHANGE:
            handleMpvPropertyChange(static_cast<mpv_event_property*>(event->data));
            break;
        case MPV_EVENT_FILE_LOADED:
            if (!m_isPaused) {
                setState(EngineState::Playing);
            } else {
                setState(EngineState::Paused);
            }
            break;
        case MPV_EVENT_START_FILE:
            setState(EngineState::Loading);
            break;
        case MPV_EVENT_END_FILE: {
            auto endData = static_cast<mpv_event_end_file*>(event->data);
            if (endData && endData->reason == MPV_END_FILE_REASON_EOF) {
                emit endOfFileReached();
            }
            break;
        }
        case MPV_EVENT_SHUTDOWN:
            shutdown();
            break;
        default:
            break;
        }
    }
}

static void onRenderUpdate(void *ctx)
{
    auto backend = static_cast<MpvBackend*>(ctx);
    if (backend) {
        emit backend->frameRenderNeeded();
    }
}

void MpvBackend::setWindowId(int64_t wid)
{
    m_wid = wid;
    if (m_mpv && wid != 0) {
        mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &m_wid);
        mpv_set_option_string(m_mpv, "vo", "gpu,x11,libmpv,null");
    }
}

bool MpvBackend::initializeRenderContext(void *(*getProcAddress)(void *, const char *), void *ctx)
{
    if (!m_mpv) return false;
    if (m_renderContext) return true;

    mpv_opengl_init_params glParams = {
        getProcAddress,
        ctx,
        nullptr
    };

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, (void *)MPV_RENDER_API_TYPE_OPENGL},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glParams},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    int err = mpv_render_context_create(&m_renderContext, m_mpv, params);
    if (err < 0) {
        emit errorOccurred(QString("Failed to create mpv render context: %1").arg(mpv_error_string(err)));
        m_renderContext = nullptr;
        return false;
    }

    mpv_render_context_set_update_callback(m_renderContext, onRenderUpdate, this);
    return true;
}

void MpvBackend::freeRenderContext()
{
    if (m_renderContext) {
        mpv_render_context_set_update_callback(m_renderContext, nullptr, nullptr);
        mpv_render_context_free(m_renderContext);
        m_renderContext = nullptr;
    }
}

void MpvBackend::renderGL(int fbo, int width, int height, bool flipY)
{
    if (!m_renderContext || width <= 0 || height <= 0) return;

    mpv_opengl_fbo mpv_fbo{fbo, width, height, 0};
    int flip = flipY ? 1 : 0;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    mpv_render_context_render(m_renderContext, params);
}

} // namespace Core
} // namespace Penguin
