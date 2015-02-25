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

#include <QDBusConnection>
#include <QObject>

class QDBusPendingCallWatcher;

namespace OnlineAccounts {

class ManagerPrivate: public QObject
{
    Q_OBJECT

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

private:
    QString m_applicationId;
    QDBusConnection m_conn;
    QDBusPendingCallWatcher *m_getAccountsCall;
    QList<Account*> m_accounts;
    mutable Manager *q_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MANAGER_P_H
