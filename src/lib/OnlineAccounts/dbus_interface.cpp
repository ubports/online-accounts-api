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

#include "dbus_interface.h"

#include <QDBusMetaType>
#include <QDebug>
#include <climits>

using namespace OnlineAccounts;

DBusInterface::DBusInterface(const QString &service,
                             const QString &path,
                             const char *interface,
                             const QDBusConnection &connection,
                             QObject *parent):
    QDBusAbstractInterface(service, path, interface, connection, parent)
{
    setTimeout(INT_MAX);

    qDBusRegisterMetaType<AccountInfo>();
    qDBusRegisterMetaType<QList<AccountInfo>>();

    bool ok = connect("AccountChanged", "s(ua{sv})",
                      this, SLOT(onAccountChanged(const QString&,const OnlineAccounts::AccountInfo&)));
    if (Q_UNLIKELY(!ok)) {
        qCritical() << "Connection to AccountChanged signal failed";
    }
}

DBusInterface::~DBusInterface()
{
}

QDBusPendingCall DBusInterface::getAccounts(const QVariantMap &filters)
{
    return asyncCall(QStringLiteral("GetAccounts"), filters);
}

QDBusPendingCall DBusInterface::authenticate(AccountId accountId,
                                             const QString &service,
                                             bool interactive,
                                             bool invalidate,
                                             const QVariantMap &parameters)
{
    return asyncCall(QStringLiteral("Authenticate"), accountId, service,
                     interactive, invalidate, parameters);
}

QDBusPendingCall DBusInterface::requestAccess(const QString &service,
                                              const QVariantMap &parameters)
{
    return asyncCall(QStringLiteral("RequestAccess"), service, parameters);
}

void DBusInterface::onAccountChanged(const QString &service,
                                     const AccountInfo &info)
{
    Q_EMIT accountChanged(service, info);
}

bool DBusInterface::connect(const char *signal, const char *signature,
                            QObject *receiver, const char *slot)
{
    return connection().connect(service(), path(), interface(),
                                QLatin1String(signal),
                                QLatin1String(signature),
                                receiver, slot);
}
