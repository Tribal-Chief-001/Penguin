#include "DBusService.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QCoreApplication>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

namespace Penguin {
namespace Desktop {

DBusService::DBusService(MPRIS2Service *service, QObject *parent)
    : QObject(parent)
    , m_service(service)
{
}

DBusService::~DBusService()
{
    unregisterService();
}

bool DBusService::isConnected() const
{
    return QDBusConnection::sessionBus().isConnected();
}

bool DBusService::registerService()
{
    if (m_registered) return true;

    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qWarning() << "DBusService: Session bus not available (headless/container mode).";
        return false;
    }

    if (!m_service) {
        qWarning() << "DBusService: Null MPRIS2Service passed.";
        return false;
    }

    // Export object with adaptors at standard MPRIS2 path
    if (!bus.registerObject("/org/mpris/MediaPlayer2", m_service, QDBusConnection::ExportAdaptors)) {
        qWarning() << "DBusService: Failed to register object /org/mpris/MediaPlayer2:" << bus.lastError().message();
        return false;
    }

    // Try primary name
    QString primaryName = "org.mpris.MediaPlayer2.penguin";
    if (bus.registerService(primaryName)) {
        m_serviceName = primaryName;
        m_registered = true;
        return true;
    }

    // If primary name is already registered by another instance, register instance name
    QString instanceName = QString("org.mpris.MediaPlayer2.penguin.instance%1").arg(QCoreApplication::applicationPid());
    if (bus.registerService(instanceName)) {
        m_serviceName = instanceName;
        m_registered = true;
        return true;
    }

    qWarning() << "DBusService: Failed to register service names:" << bus.lastError().message();
    return false;
}

void DBusService::unregisterService()
{
    if (m_registered) {
        auto bus = QDBusConnection::sessionBus();
        if (bus.isConnected()) {
            bus.unregisterObject("/org/mpris/MediaPlayer2");
            bus.unregisterService(m_serviceName);
        }
        m_registered = false;
    }
}

bool DBusService::sendRemoteCommand(const QString &action, const QStringList &files)
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        return false;
    }

    auto iface = bus.interface();
    if (!iface || !iface->isServiceRegistered("org.mpris.MediaPlayer2.penguin")) {
        return false;
    }

    QDBusInterface rootIface("org.mpris.MediaPlayer2.penguin",
                             "/org/mpris/MediaPlayer2",
                             "org.mpris.MediaPlayer2",
                             bus);

    QDBusInterface playerIface("org.mpris.MediaPlayer2.penguin",
                               "/org/mpris/MediaPlayer2",
                               "org.mpris.MediaPlayer2.Player",
                               bus);

    bool handled = false;

    if (!files.isEmpty()) {
        for (const QString &file : files) {
            QString uri = file;
            if (!uri.contains("://")) {
                uri = QUrl::fromLocalFile(QFileInfo(file).absoluteFilePath()).toString();
            }
            playerIface.call("OpenUri", uri);
        }
        rootIface.call("Raise");
        handled = true;
    }

    if (action == "play-pause" || action == "playpause") {
        playerIface.call("PlayPause");
        handled = true;
    } else if (action == "next") {
        playerIface.call("Next");
        handled = true;
    } else if (action == "previous" || action == "prev") {
        playerIface.call("Previous");
        handled = true;
    } else if (action == "stop") {
        playerIface.call("Stop");
        handled = true;
    } else if (action == "raise") {
        rootIface.call("Raise");
        handled = true;
    }

    return handled;
}

} // namespace Desktop
} // namespace Penguin
