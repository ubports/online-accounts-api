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

#ifndef ONLINE_ACCOUNTS_DBUS_INTERFACE_H
#define ONLINE_ACCOUNTS_DBUS_INTERFACE_H

#include <QDBusAbstractInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QList>

#include "account_info.h"

namespace OnlineAccounts {

/* Avoid using QDBusInterface which does a blocking introspection call.
 */
class DBusInterface: public QDBusAbstractInterface
{
public:
    DBusInterface(const QString &service,
                  const QString &path,
                  const char *interface,
                  const QDBusConnection &connection,
                  QObject *parent = 0);
    virtual ~DBusInterface();

    QDBusPendingReply<QList<AccountInfo> > getAccounts(const QVariantMap &filters);

    QDBusPendingCall authenticate(AccountId accountId, const QString &service,
                                  bool interactive, bool invalidate,
                                  const QVariantMap &parameters);

    QDBusPendingCall requestAccess(const QString &service,
                                   const QVariantMap &parameters);

Q_SIGNALS:
    void accountChanged(const QString &service, const AccountInfo &info);

private:
    bool connect(const char *signal, const char *signature,
                 QObject *receiver, const char *slot);
};

}

#endif // ONLINE_ACCOUNTS_DBUS_INTERFACE_H
