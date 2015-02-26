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

#include "dbus_constants.h"
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
        return details.value(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME).toString();
    }
    QString service() const {
        return details.value(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID).toString();
    }
    AuthenticationMethod authenticationMethod() const {
        return AuthenticationMethod(details.value(ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD).toInt());
    }

    QVariant setting(const QString &key) const {
        return details.value(ONLINE_ACCOUNTS_INFO_KEY_SETTINGS + key);
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
