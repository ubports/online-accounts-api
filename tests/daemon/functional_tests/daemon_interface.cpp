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

#include "daemon_interface.h"

#include <QDBusMetaType>
#include <climits>

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info) {
    argument.beginStructure();
    argument << info.accountId << info.details;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AccountInfo &info) {
    argument.beginStructure();
    argument >> info.accountId >> info.details;
    argument.endStructure();
    return argument;
}


DaemonInterface::DaemonInterface(QObject *parent):
    QDBusAbstractInterface(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                           ONLINE_ACCOUNTS_MANAGER_PATH,
                           ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                           QDBusConnection::sessionBus(),
                           parent)
{
    setTimeout(INT_MAX);

    qDBusRegisterMetaType<AccountInfo>();
    qDBusRegisterMetaType<QList<AccountInfo>>();

    bool ok = connect("AccountChanged", "s(ua{sv})",
                      this, SIGNAL(accountChanged(const QString&,const AccountInfo&)));
    if (Q_UNLIKELY(!ok)) {
        qCritical() << "Connection to AccountChanged signal failed";
    }
}
