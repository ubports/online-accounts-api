/*
 * This file is part of OnlineAccountsDaemon
 *
 * Copyright (C) 2015 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "client_registry.h"

#include <QByteArray>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QHash>
#include <QVariantMap>
#include <sys/apparmor.h>

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

class ClientRegistryPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ClientRegistry)

public:
    ClientRegistryPrivate(ClientRegistry *q);

    void registerClient(const QString &client);
    QString getSecurityContext(const QString &client) const;
    pid_t getPid(const QString &client) const;

private Q_SLOTS:
    void onServiceUnregistered(const QString &client);

private:
    static ClientRegistry *m_instance;
    QDBusConnection m_connection;
    QDBusServiceWatcher m_watcher;
    QHash<QString,QString> m_clientContexts;
    ClientRegistry *q_ptr;
};

ClientRegistry *ClientRegistryPrivate::m_instance = 0;

} // namespace

ClientRegistryPrivate::ClientRegistryPrivate(ClientRegistry *q):
    QObject(q),
    m_connection(QDBusConnection::sessionBus()),
    q_ptr(q)
{
    m_watcher.setConnection(m_connection);
    QObject::connect(&m_watcher, SIGNAL(serviceUnregistered(const QString&)),
                     this, SLOT(onServiceUnregistered(const QString&)));
}

void ClientRegistryPrivate::registerClient(const QString &client)
{
    Q_Q(ClientRegistry);

    if (m_clientContexts.contains(client)) return;

    bool wasEmpty = m_clientContexts.isEmpty();
    m_clientContexts.insert(client, getSecurityContext(client));
    m_watcher.addWatchedService(client);
    if (wasEmpty) {
        Q_EMIT q->hasClientsChanged();
    }
}

QString ClientRegistryPrivate::getSecurityContext(const QString &client) const
{
    QString dbusService = qEnvironmentVariableIsEmpty("OAD_TESTING") ?
        "org.freedesktop.DBus" : "mocked.org.freedesktop.dbus";
    QDBusMessage msg =
        QDBusMessage::createMethodCall(dbusService,
                                       "/org/freedesktop/DBus",
                                       "org.freedesktop.DBus",
                                       "GetConnectionCredentials");
    msg << client;
    QDBusReply<QVariantMap> reply = m_connection.call(msg, QDBus::Block);

    QString context;
    if (reply.isValid()) {
        QVariantMap map = reply.value();
        QByteArray label = map.value("LinuxSecurityLabel").toByteArray();
        if (!label.isEmpty()) {
            aa_splitcon(label.data(), NULL);
            context = QString::fromUtf8(label);
        }
    } else {
        QDBusError error = reply.error();
        qWarning() << "Error getting app ID:" << error.name() <<
            error.message();
        context = QStringLiteral("unconfined");
    }
    qDebug() << "Client security context:" << context;

    return context;
}

pid_t ClientRegistryPrivate::getPid(const QString &client) const
{
    QDBusReply<uint> reply = m_connection.interface()->servicePid(client);
    return pid_t(reply.value());
}

void ClientRegistryPrivate::onServiceUnregistered(const QString &client)
{
    Q_Q(ClientRegistry);

    qDebug() << "Client disappeared:" << client;
    m_clientContexts.remove(client);
    if (m_clientContexts.isEmpty()) {
        Q_EMIT q->hasClientsChanged();
    }
}

ClientRegistry::ClientRegistry():
    QObject(),
    d_ptr(new ClientRegistryPrivate(this))
{
}

ClientRegistry::~ClientRegistry()
{
    delete d_ptr;
}

ClientRegistry *ClientRegistry::instance()
{
    if (!ClientRegistryPrivate::m_instance) {
        ClientRegistryPrivate::m_instance = new ClientRegistry();
    }
    return ClientRegistryPrivate::m_instance;
}

QString ClientRegistry::registerClient(const QDBusConnection &connection,
                                       const QDBusMessage &message)
{
    Q_UNUSED(connection); // only needed for p2p clients

    QString client = message.service();
    registerClient(client);
    return client;
}

void ClientRegistry::registerClient(const QString &client)
{
    Q_D(ClientRegistry);
    d->registerClient(client);
}

QStringList ClientRegistry::clients() const
{
    Q_D(const ClientRegistry);
    return d->m_clientContexts.keys();
}

void ClientRegistry::registerActiveClients(const QStringList &clients)
{
    Q_D(const ClientRegistry);
    QStringList activeServices =
        d->m_connection.interface()->registeredServiceNames().value();
    Q_FOREACH(const QString &client, clients) {
        if (activeServices.contains(client)) {
            registerClient(client);
        }
    }
}

QString ClientRegistry::clientSecurityContext(const QString &client) const
{
    Q_D(const ClientRegistry);
    QHash<QString,QString>::const_iterator i =
        d->m_clientContexts.find(client);
    if (i != d->m_clientContexts.constEnd()) {
        return i.value();
    }

    return d->getSecurityContext(client);
}

pid_t ClientRegistry::clientPid(const QString &client) const
{
    Q_D(const ClientRegistry);
    return d->getPid(client);
}

#include "client_registry.moc"
