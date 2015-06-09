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

#ifndef ONLINE_ACCOUNTS_DAEMON_ACCOUNT_INFO_H
#define ONLINE_ACCOUNTS_DAEMON_ACCOUNT_INFO_H

#include <QVariantMap>
#include "dbus_constants.h"

namespace OnlineAccountsDaemon {

struct AccountInfo {
    uint accountId;
    QVariantMap details;

    AccountInfo(): accountId(0) {}
    AccountInfo(uint accountId, const QVariantMap &details):
        accountId(accountId), details(details) {}
    QString serviceId() const {
        return details[ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID].toString();
    }
};

} // namespace

Q_DECLARE_METATYPE(OnlineAccountsDaemon::AccountInfo)

#endif // ONLINE_ACCOUNTS_DAEMON_ACCOUNT_INFO_H
