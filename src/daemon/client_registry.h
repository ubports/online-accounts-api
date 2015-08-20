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

#ifndef ONLINE_ACCOUNTS_DAEMON_CLIENT_REGISTRY_H
#define ONLINE_ACCOUNTS_DAEMON_CLIENT_REGISTRY_H

#include <QObject>
#include <QStringList>
#include <sys/types.h>

class QDBusConnection;
class QDBusMessage;

namespace OnlineAccountsDaemon {

class ClientRegistryPrivate;
class ClientRegistry: public QObject
{
    Q_OBJECT

public:
    ~ClientRegistry();

    static ClientRegistry *instance();

    QString registerClient(const QDBusConnection &connection,
                           const QDBusMessage &message);
    void registerClient(const QString &client);
    void registerActiveClients(const QStringList &clients);
    QStringList clients() const;
    bool hasClients() const { return !clients().isEmpty(); }

    QString clientSecurityContext(const QString &client) const;
    pid_t clientPid(const QString &client) const;

Q_SIGNALS:
    void hasClientsChanged();

private:
    ClientRegistry();
    Q_DECLARE_PRIVATE(ClientRegistry)
    ClientRegistryPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_CLIENT_REGISTRY_H
