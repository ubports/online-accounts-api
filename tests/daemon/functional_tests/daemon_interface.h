/*
 * This file is part of libOnlineAccounts
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

#ifndef OAD_DAEMON_INTERFACE_H
#define OAD_DAEMON_INTERFACE_H

#include "daemon/dbus_constants.h"

#include <QDBusAbstractInterface>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>
#include <QList>
#include <QVariantMap>

class AccountInfo {
public:
    AccountInfo(): accountId(0) {}
    AccountInfo(uint accountId, const QVariantMap &details):
        accountId(accountId), details(details) {}

    uint id() const { return accountId; }
    QVariantMap data() const { return details; }

private:
    friend QDBusArgument &operator<<(QDBusArgument &, const AccountInfo &);
    friend const QDBusArgument &operator>>(const QDBusArgument &, AccountInfo &);
    uint accountId;
    QVariantMap details;
};

Q_DECLARE_METATYPE(AccountInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, AccountInfo &info);

/* Avoid using QDBusInterface which does a blocking introspection call.
 */
class DaemonInterface: public QDBusAbstractInterface
{
    Q_OBJECT

public:
    DaemonInterface(QObject *parent = 0);
    ~DaemonInterface() {}

    QDBusPendingCall getAccounts(const QVariantMap &filters) {
        return asyncCall(QStringLiteral("GetAccounts"), filters);
    }

    QDBusPendingCall authenticate(uint accountId, const QString &service,
                                  bool interactive, bool invalidate,
                                  const QVariantMap &parameters) {
        return asyncCall(QStringLiteral("Authenticate"), accountId, service,
                         interactive, invalidate, parameters);
    }

    QDBusPendingCall requestAccess(const QString &service,
                                   const QVariantMap &parameters) {
        return asyncCall(QStringLiteral("RequestAccess"), service, parameters);
    }


Q_SIGNALS:
    void accountChanged(const QString &service,
                        const AccountInfo &info);

private:
    bool connect(const char *signal, const char *signature,
                 QObject *receiver, const char *slot) {
        return connection().connect(service(), path(), interface(),
                                    QLatin1String(signal),
                                    QLatin1String(signature),
                                    receiver, slot);
    }
};

#endif // OAD_DAEMON_INTERFACE_H
