#include "MPRIS2Adaptor.h"
#include "PlaybackEngine.h"
#include "PlaylistManager.h"
#include "MainWindow.h"

#include <QCoreApplication>
#include <QUrl>
#include <QFileInfo>
#include <QtDBus/QDBusMetaType>

namespace Penguin {
namespace Desktop {

// =============================================================================
// MPRIS2RootAdaptor
// =============================================================================

MPRIS2RootAdaptor::MPRIS2RootAdaptor(MPRIS2Service *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

MPRIS2Service* MPRIS2RootAdaptor::service() const
{
    return static_cast<MPRIS2Service*>(parent());
}

bool MPRIS2RootAdaptor::canQuit() const
{
    return true;
}

bool MPRIS2RootAdaptor::fullscreen() const
{
    if (service()->mainWindow()) {
        return service()->mainWindow()->isFullScreen();
    }
    return false;
}

void MPRIS2RootAdaptor::setFullscreen(bool value)
{
    if (service()->mainWindow()) {
        if (value && !service()->mainWindow()->isFullScreen()) {
            service()->mainWindow()->showFullScreen();
        } else if (!value && service()->mainWindow()->isFullScreen()) {
            service()->mainWindow()->showNormal();
        }
    }
}

bool MPRIS2RootAdaptor::canSetFullscreen() const
{
    return true;
}

bool MPRIS2RootAdaptor::canRaise() const
{
    return true;
}

bool MPRIS2RootAdaptor::hasTrackList() const
{
    return false;
}

QString MPRIS2RootAdaptor::identity() const
{
    return "Penguin Media Player";
}

QString MPRIS2RootAdaptor::desktopEntry() const
{
    return "penguin";
}

QStringList MPRIS2RootAdaptor::supportedUriSchemes() const
{
    return {"file", "http", "https", "ftp"};
}

QStringList MPRIS2RootAdaptor::supportedMimeTypes() const
{
    return {
        "audio/mpeg", "audio/flac", "audio/ogg", "audio/opus", "audio/x-wav",
        "audio/aac", "video/mp4", "video/x-matroska", "video/webm", "video/quicktime", "video/x-msvideo"
    };
}

void MPRIS2RootAdaptor::Raise()
{
    if (service()->mainWindow()) {
        service()->mainWindow()->show();
        service()->mainWindow()->raise();
        service()->mainWindow()->activateWindow();
    }
}

void MPRIS2RootAdaptor::Quit()
{
    QCoreApplication::quit();
}

// =============================================================================
// MPRIS2PlayerAdaptor
// =============================================================================

MPRIS2PlayerAdaptor::MPRIS2PlayerAdaptor(MPRIS2Service *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

MPRIS2Service* MPRIS2PlayerAdaptor::service() const
{
    return static_cast<MPRIS2Service*>(parent());
}

QString MPRIS2PlayerAdaptor::playbackStatus() const
{
    if (!service()->engine()) return "Stopped";
    switch (service()->engine()->playbackState()) {
    case Core::PlaybackState::Playing:
    case Core::PlaybackState::Buffering:
        return "Playing";
    case Core::PlaybackState::Paused:
        return "Paused";
    default:
        return "Stopped";
    }
}

QString MPRIS2PlayerAdaptor::loopStatus() const
{
    if (!service()->playlistManager()) return "None";
    switch (service()->playlistManager()->loopMode()) {
    case Library::LoopMode::Track:
        return "Track";
    case Library::LoopMode::Playlist:
        return "Playlist";
    default:
        return "None";
    }
}

void MPRIS2PlayerAdaptor::setLoopStatus(const QString &status)
{
    if (!service()->playlistManager()) return;
    QString s = status.toLower();
    if (s == "track") {
        service()->playlistManager()->setLoopMode(Library::LoopMode::Track);
    } else if (s == "playlist") {
        service()->playlistManager()->setLoopMode(Library::LoopMode::Playlist);
    } else {
        service()->playlistManager()->setLoopMode(Library::LoopMode::None);
    }
    QVariantMap map;
    map["LoopStatus"] = loopStatus();
    service()->emitPropertiesChanged(map);
}

double MPRIS2PlayerAdaptor::rate() const
{
    if (service()->engine()) {
        return service()->engine()->speed();
    }
    return 1.0;
}

void MPRIS2PlayerAdaptor::setRate(double rate)
{
    if (service()->engine()) {
        double clamped = std::clamp(rate, 0.5, 2.0);
        service()->engine()->setSpeed(clamped);
        QVariantMap map;
        map["Rate"] = clamped;
        service()->emitPropertiesChanged(map);
    }
}

bool MPRIS2PlayerAdaptor::shuffle() const
{
    if (service()->playlistManager()) {
        return service()->playlistManager()->isShuffleEnabled();
    }
    return false;
}

void MPRIS2PlayerAdaptor::setShuffle(bool shuffle)
{
    if (service()->playlistManager()) {
        service()->playlistManager()->setShuffle(shuffle);
        QVariantMap map;
        map["Shuffle"] = shuffle;
        service()->emitPropertiesChanged(map);
    }
}

QVariantMap MPRIS2PlayerAdaptor::metadata() const
{
    QVariantMap map;
    if (!service()->engine()) return map;

    const auto &meta = service()->engine()->metadata();
    qint64 durationUs = service()->engine()->durationMs() * 1000;

    map["mpris:trackid"] = QVariant::fromValue(QDBusObjectPath("/org/mpris/MediaPlayer2/Track/0"));
    map["mpris:length"] = durationUs;

    QString uri = service()->engine()->currentUri();
    if (!uri.isEmpty()) {
        if (!uri.contains("://")) {
            map["xesam:url"] = QUrl::fromLocalFile(uri).toString();
        } else {
            map["xesam:url"] = uri;
        }
    }

    QString title = meta.title.isEmpty() ? QFileInfo(uri).baseName() : meta.title;
    map["xesam:title"] = title;

    if (!meta.artist.isEmpty()) {
        map["xesam:artist"] = QStringList() << meta.artist;
    } else {
        map["xesam:artist"] = QStringList();
    }

    if (!meta.album.isEmpty()) {
        map["xesam:album"] = meta.album;
        map["xesam:albumArtist"] = QStringList() << meta.artist;
    }

    if (!meta.genre.isEmpty()) {
        map["xesam:genre"] = QStringList() << meta.genre;
    }

    return map;
}

double MPRIS2PlayerAdaptor::volume() const
{
    if (service()->engine()) {
        return service()->engine()->volume() / 100.0;
    }
    return 1.0;
}

void MPRIS2PlayerAdaptor::setVolume(double vol)
{
    if (service()->engine()) {
        double clamped = std::clamp(vol, 0.0, 1.5);
        service()->engine()->setVolume(qRound(clamped * 100.0));
        QVariantMap map;
        map["Volume"] = clamped;
        service()->emitPropertiesChanged(map);
    }
}

qlonglong MPRIS2PlayerAdaptor::position() const
{
    if (service()->engine()) {
        return service()->engine()->positionMs() * 1000; // in microseconds
    }
    return 0;
}

double MPRIS2PlayerAdaptor::minimumRate() const
{
    return 0.5;
}

double MPRIS2PlayerAdaptor::maximumRate() const
{
    return 2.0;
}

bool MPRIS2PlayerAdaptor::canGoNext() const
{
    if (service()->playlistManager()) {
        return service()->playlistManager()->hasNext();
    }
    return false;
}

bool MPRIS2PlayerAdaptor::canGoPrevious() const
{
    if (service()->playlistManager()) {
        return service()->playlistManager()->hasPrevious();
    }
    return false;
}

bool MPRIS2PlayerAdaptor::canPlay() const
{
    return true;
}

bool MPRIS2PlayerAdaptor::canPause() const
{
    return true;
}

bool MPRIS2PlayerAdaptor::canSeek() const
{
    return true;
}

bool MPRIS2PlayerAdaptor::canControl() const
{
    return true;
}

void MPRIS2PlayerAdaptor::Next()
{
    if (service()->playlistManager()) {
        service()->playlistManager()->next();
    }
}

void MPRIS2PlayerAdaptor::Previous()
{
    if (service()->engine() && service()->engine()->positionMs() > 3000) {
        service()->engine()->seek(0);
    } else if (service()->playlistManager()) {
        service()->playlistManager()->previous();
    }
}

void MPRIS2PlayerAdaptor::Pause()
{
    if (service()->engine()) {
        service()->engine()->pause();
    }
}

void MPRIS2PlayerAdaptor::PlayPause()
{
    if (service()->engine()) {
        service()->engine()->togglePlayPause();
    }
}

void MPRIS2PlayerAdaptor::Stop()
{
    if (service()->engine()) {
        service()->engine()->stop();
    }
}

void MPRIS2PlayerAdaptor::Play()
{
    if (service()->engine()) {
        service()->engine()->play();
    }
}

void MPRIS2PlayerAdaptor::Seek(qlonglong offsetUs)
{
    if (service()->engine()) {
        qint64 offsetMs = offsetUs / 1000;
        service()->engine()->seekRelative(offsetMs);
        emit Seeked(service()->engine()->positionMs() * 1000);
    }
}

void MPRIS2PlayerAdaptor::SetPosition(const QDBusObjectPath &trackId, qlonglong posUs)
{
    Q_UNUSED(trackId)
    if (service()->engine()) {
        qint64 posMs = std::max(qint64(0), posUs / 1000);
        service()->engine()->seek(posMs);
        emit Seeked(posMs * 1000);
    }
}

void MPRIS2PlayerAdaptor::OpenUri(const QString &uri)
{
    QString localUri = uri;
    if (localUri.startsWith("file://")) {
        localUri = QUrl(uri).toLocalFile();
    }

    if (service()->mainWindow()) {
        service()->mainWindow()->openMedia(localUri, true);
    } else if (service()->engine()) {
        service()->engine()->loadMedia(localUri, true);
    }
}

// =============================================================================
// MPRIS2Service
// =============================================================================

MPRIS2Service::MPRIS2Service(Core::PlaybackEngine *engine,
                             Library::PlaylistManager *playlistMgr,
                             UI::MainWindow *mainWindow,
                             QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_playlistMgr(playlistMgr)
    , m_mainWindow(mainWindow)
{
    m_rootAdaptor = new MPRIS2RootAdaptor(this);
    m_playerAdaptor = new MPRIS2PlayerAdaptor(this);

    if (m_engine) {
        connect(m_engine, &Core::PlaybackEngine::playbackStateChanged, this, &MPRIS2Service::onEngineStateChanged);
        connect(m_engine, &Core::PlaybackEngine::metadataChanged, this, &MPRIS2Service::onEngineMetadataChanged);
        connect(m_engine, &Core::PlaybackEngine::durationChanged, this, &MPRIS2Service::onEngineDurationChanged);
        connect(m_engine, &Core::PlaybackEngine::volumeChanged, this, &MPRIS2Service::onEngineVolumeChanged);
        connect(m_engine, &Core::PlaybackEngine::speedChanged, this, &MPRIS2Service::onEngineSpeedChanged);
    }

    if (m_playlistMgr) {
        connect(m_playlistMgr, &Library::PlaylistManager::playlistUpdated, this, &MPRIS2Service::onPlaylistUpdated);
        connect(m_playlistMgr, &Library::PlaylistManager::shuffleChanged, this, [this](bool enabled) {
            QVariantMap m;
            m["Shuffle"] = enabled;
            emitPropertiesChanged(m);
        });
        connect(m_playlistMgr, &Library::PlaylistManager::loopModeChanged, this, [this](Library::LoopMode) {
            QVariantMap m;
            m["LoopStatus"] = m_playerAdaptor->loopStatus();
            emitPropertiesChanged(m);
        });
    }
}

void MPRIS2Service::emitSeeked(qlonglong positionMicroseconds)
{
    if (m_playerAdaptor) {
        emit m_playerAdaptor->Seeked(positionMicroseconds);
    }
}

void MPRIS2Service::emitPropertiesChanged(const QVariantMap &changedProps)
{
    if (changedProps.isEmpty()) return;

    QDBusMessage signal = QDBusMessage::createSignal(
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged"
    );

    signal << QString("org.mpris.MediaPlayer2.Player");
    signal << changedProps;
    signal << QStringList(); // empty invalidated properties

    QDBusConnection::sessionBus().send(signal);
}

void MPRIS2Service::onEngineStateChanged()
{
    QVariantMap map;
    map["PlaybackStatus"] = m_playerAdaptor->playbackStatus();
    emitPropertiesChanged(map);
}

void MPRIS2Service::onEngineMetadataChanged()
{
    QVariantMap map;
    map["Metadata"] = m_playerAdaptor->metadata();
    emitPropertiesChanged(map);
}

void MPRIS2Service::onEngineDurationChanged()
{
    QVariantMap map;
    map["Metadata"] = m_playerAdaptor->metadata();
    emitPropertiesChanged(map);
}

void MPRIS2Service::onEngineVolumeChanged()
{
    QVariantMap map;
    map["Volume"] = m_playerAdaptor->volume();
    emitPropertiesChanged(map);
}

void MPRIS2Service::onEngineSpeedChanged()
{
    QVariantMap map;
    map["Rate"] = m_playerAdaptor->rate();
    emitPropertiesChanged(map);
}

void MPRIS2Service::onPlaylistUpdated()
{
    QVariantMap map;
    map["CanGoNext"] = m_playerAdaptor->canGoNext();
    map["CanGoPrevious"] = m_playerAdaptor->canGoPrevious();
    emitPropertiesChanged(map);
}

} // namespace Desktop
} // namespace Penguin
