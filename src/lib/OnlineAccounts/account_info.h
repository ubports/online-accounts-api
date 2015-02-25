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

#ifndef ONLINE_ACCOUNTS_ACCOUNT_INFO_H
#define ONLINE_ACCOUNTS_ACCOUNT_INFO_H

#include <QVariantMap>

#include "global.h"

class QDBusArgument;

namespace OnlineAccounts {

class AccountInfo {
public:
    AccountInfo(): accountId(0) {}
    AccountInfo(AccountId accountId, const QVariantMap &details):
        accountId(accountId), details(details) {}

    AccountId id() const { return accountId; }
    QString displayName() const {
        return details.value("displayName").toString();
    }

private:
    friend QDBusArgument &operator<<(QDBusArgument &, const AccountInfo &);
    friend const QDBusArgument &operator>>(const QDBusArgument &, AccountInfo &);
    AccountId accountId;
    QVariantMap details;
};

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, AccountInfo &info);

} //namespace

Q_DECLARE_METATYPE(OnlineAccounts::AccountInfo)

#endif // ONLINE_ACCOUNTS_ACCOUNT_INFO_H
