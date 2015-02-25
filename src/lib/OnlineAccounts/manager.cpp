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

#include "manager_p.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

using namespace OnlineAccounts;

#define ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME \
    "com.ubuntu.OnlineAccounts.Manager"
#define ONLINE_ACCOUNTS_MANAGER_PATH "/com/ubuntu/OnlineAccounts/Manager"
#define ONLINE_ACCOUNTS_MANAGER_INTERFACE ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME

ManagerPrivate::ManagerPrivate(Manager *q, const QString &applicationId):
    QObject(),
    m_applicationId(applicationId),
    m_daemon(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
             ONLINE_ACCOUNTS_MANAGER_PATH,
             ONLINE_ACCOUNTS_MANAGER_INTERFACE,
             QDBusConnection::sessionBus()),
    m_getAccountsCall(0),
    q_ptr(q)
{
    retrieveAccounts();
}

ManagerPrivate::~ManagerPrivate()
{
    delete m_getAccountsCall;
    m_getAccountsCall = 0;
}

void ManagerPrivate::retrieveAccounts()
{
    if (Q_UNLIKELY(m_getAccountsCall)) return;

    QVariantMap filters;
    filters["applicationId"] = m_applicationId;
    m_getAccountsCall =
        new QDBusPendingCallWatcher(m_daemon.getAccounts(filters));
}
