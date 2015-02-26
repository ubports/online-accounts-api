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

#ifndef ONLINE_ACCOUNTS_MANAGER_P_H
#define ONLINE_ACCOUNTS_MANAGER_P_H

#include "manager.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include "account_info.h"
#include "dbus_interface.h"

class QDBusPendingCallWatcher;

namespace OnlineAccounts {

struct AccountData {
    AccountInfo info;
    QPointer<Account> account;

    AccountData(const AccountInfo &info): info(info) {}
};

class ManagerPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Manager)

public:
    inline ManagerPrivate(Manager *q, const QString &applicationId);
    ~ManagerPrivate();

    PendingCall authenticate(AccountId accountId, const QString &service,
                             bool interactive, bool invalidate,
                             const QVariantMap &parameters);
    PendingCall requestAccess(const QString &service,
                              const QVariantMap &parameters);

private:
    void retrieveAccounts();

private Q_SLOTS:
    void onGetAccountsFinished();

private:
    QString m_applicationId;
    DBusInterface m_daemon;
    QDBusPendingCallWatcher *m_getAccountsCall;
    QHash<AccountId,AccountData> m_accounts;
    mutable Manager *q_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MANAGER_P_H
