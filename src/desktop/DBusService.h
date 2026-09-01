#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "MPRIS2Adaptor.h"

namespace Penguin {
namespace Desktop {

class DBusService : public QObject {
    Q_OBJECT

public:
    explicit DBusService(MPRIS2Service *service, QObject *parent = nullptr);
    virtual ~DBusService();

    bool registerService();
    void unregisterService();

    bool isConnected() const;
    bool isRegistered() const { return m_registered; }
    QString serviceName() const { return m_serviceName; }

    // Static helper to send remote commands to an active Penguin instance
    static bool sendRemoteCommand(const QString &action, const QStringList &files = QStringList());

private:
    MPRIS2Service *m_service = nullptr;
    QString m_serviceName;
    bool m_registered = false;
};

} // namespace Desktop
} // namespace Penguin

#endif // DBUSSERVICE_H
