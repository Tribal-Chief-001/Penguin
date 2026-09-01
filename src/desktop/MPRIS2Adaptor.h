#ifndef MPRIS2ADAPTOR_H
#define MPRIS2ADAPTOR_H

#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QVariantMap>
#include <QStringList>

namespace Penguin {

namespace Core {
class PlaybackEngine;
}
namespace Library {
class PlaylistManager;
}
namespace UI {
class MainWindow;
}

namespace Desktop {

class MPRIS2Service;

// 1. Root Interface Adaptor: org.mpris.MediaPlayer2
class MPRIS2RootAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_CLASSINFO("D-Bus Introspection", ""
        "  <interface name=\"org.mpris.MediaPlayer2\">\n"
        "    <method name=\"Raise\"/>\n"
        "    <method name=\"Quit\"/>\n"
        "    <property name=\"CanQuit\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"Fullscreen\" type=\"b\" access=\"readwrite\"/>\n"
        "    <property name=\"CanSetFullscreen\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"CanRaise\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"HasTrackList\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"Identity\" type=\"s\" access=\"read\"/>\n"
        "    <property name=\"DesktopEntry\" type=\"s\" access=\"read\"/>\n"
        "    <property name=\"SupportedUriSchemes\" type=\"as\" access=\"read\"/>\n"
        "    <property name=\"SupportedMimeTypes\" type=\"as\" access=\"read\"/>\n"
        "  </interface>\n")

    Q_PROPERTY(bool CanQuit READ canQuit)
    Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen)
    Q_PROPERTY(bool CanRaise READ canRaise)
    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    Q_PROPERTY(QString Identity READ identity)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)

public:
    explicit MPRIS2RootAdaptor(MPRIS2Service *parent);

    bool canQuit() const;
    bool fullscreen() const;
    void setFullscreen(bool value);
    bool canSetFullscreen() const;
    bool canRaise() const;
    bool hasTrackList() const;
    QString identity() const;
    QString desktopEntry() const;
    QStringList supportedUriSchemes() const;
    QStringList supportedMimeTypes() const;

public slots:
    void Raise();
    void Quit();

private:
    MPRIS2Service *service() const;
};

// 2. Player Interface Adaptor: org.mpris.MediaPlayer2.Player
class MPRIS2PlayerAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_CLASSINFO("D-Bus Introspection", ""
        "  <interface name=\"org.mpris.MediaPlayer2.Player\">\n"
        "    <method name=\"Next\"/>\n"
        "    <method name=\"Previous\"/>\n"
        "    <method name=\"Pause\"/>\n"
        "    <method name=\"PlayPause\"/>\n"
        "    <method name=\"Stop\"/>\n"
        "    <method name=\"Play\"/>\n"
        "    <method name=\"Seek\">\n"
        "      <arg direction=\"in\" type=\"x\" name=\"Offset\"/>\n"
        "    </method>\n"
        "    <method name=\"SetPosition\">\n"
        "      <arg direction=\"in\" type=\"o\" name=\"TrackId\"/>\n"
        "      <arg direction=\"in\" type=\"x\" name=\"Position\"/>\n"
        "    </method>\n"
        "    <method name=\"OpenUri\">\n"
        "      <arg direction=\"in\" type=\"s\" name=\"Uri\"/>\n"
        "    </method>\n"
        "    <signal name=\"Seeked\">\n"
        "      <arg type=\"x\" name=\"Position\"/>\n"
        "    </signal>\n"
        "    <property name=\"PlaybackStatus\" type=\"s\" access=\"read\"/>\n"
        "    <property name=\"LoopStatus\" type=\"s\" access=\"readwrite\"/>\n"
        "    <property name=\"Rate\" type=\"d\" access=\"readwrite\"/>\n"
        "    <property name=\"Shuffle\" type=\"b\" access=\"readwrite\"/>\n"
        "    <property name=\"Metadata\" type=\"a{sv}\" access=\"read\"/>\n"
        "    <property name=\"Volume\" type=\"d\" access=\"readwrite\"/>\n"
        "    <property name=\"Position\" type=\"x\" access=\"read\"/>\n"
        "    <property name=\"MinimumRate\" type=\"d\" access=\"read\"/>\n"
        "    <property name=\"MaximumRate\" type=\"d\" access=\"read\"/>\n"
        "    <property name=\"CanGoNext\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"CanGoPrevious\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"CanPlay\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"CanPause\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"CanSeek\" type=\"b\" access=\"read\"/>\n"
        "    <property name=\"CanControl\" type=\"b\" access=\"read\"/>\n"
        "  </interface>\n")

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ minimumRate)
    Q_PROPERTY(double MaximumRate READ maximumRate)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)

public:
    explicit MPRIS2PlayerAdaptor(MPRIS2Service *parent);

    QString playbackStatus() const;
    QString loopStatus() const;
    void setLoopStatus(const QString &status);
    double rate() const;
    void setRate(double rate);
    bool shuffle() const;
    void setShuffle(bool shuffle);
    QVariantMap metadata() const;
    double volume() const;
    void setVolume(double volume);
    qlonglong position() const;
    double minimumRate() const;
    double maximumRate() const;
    bool canGoNext() const;
    bool canGoPrevious() const;
    bool canPlay() const;
    bool canPause() const;
    bool canSeek() const;
    bool canControl() const;

public slots:
    void Next();
    void Previous();
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
    void OpenUri(const QString &Uri);

signals:
    void Seeked(qlonglong Position);

private:
    MPRIS2Service *service() const;
};

// 3. Service Bridge coordinating Adaptors with Engine, Playlist, and MainWindow
class MPRIS2Service : public QObject {
    Q_OBJECT

public:
    explicit MPRIS2Service(Core::PlaybackEngine *engine,
                           Library::PlaylistManager *playlistMgr,
                           UI::MainWindow *mainWindow,
                           QObject *parent = nullptr);
    virtual ~MPRIS2Service() = default;

    Core::PlaybackEngine* engine() const { return m_engine; }
    Library::PlaylistManager* playlistManager() const { return m_playlistMgr; }
    UI::MainWindow* mainWindow() const { return m_mainWindow; }
    MPRIS2PlayerAdaptor* playerAdaptor() const { return m_playerAdaptor; }

    void emitSeeked(qlonglong positionMicroseconds);
    void emitPropertiesChanged(const QVariantMap &changedProps);

public slots:
    void onEngineStateChanged();
    void onEngineMetadataChanged();
    void onEngineDurationChanged();
    void onEngineVolumeChanged();
    void onEngineSpeedChanged();
    void onPlaylistUpdated();

private:
    Core::PlaybackEngine *m_engine = nullptr;
    Library::PlaylistManager *m_playlistMgr = nullptr;
    UI::MainWindow *m_mainWindow = nullptr;

    MPRIS2RootAdaptor *m_rootAdaptor = nullptr;
    MPRIS2PlayerAdaptor *m_playerAdaptor = nullptr;
};

} // namespace Desktop
} // namespace Penguin

#endif // MPRIS2ADAPTOR_H
