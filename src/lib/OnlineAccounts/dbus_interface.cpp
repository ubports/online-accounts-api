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
}

DBusInterface::~DBusInterface()
{
}

QDBusPendingReply<QList<AccountInfo> >
DBusInterface::getAccounts(const QVariantMap &filters)
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
