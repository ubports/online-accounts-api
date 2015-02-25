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

#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

using namespace OnlineAccounts;

ManagerPrivate::ManagerPrivate(Manager *q, const QString &applicationId):
    QObject(),
    m_applicationId(applicationId),
    m_conn(QDBusConnection::sessionBus()),
    m_getAccountsCall(0),
    q_ptr(q)
{
}

ManagerPrivate::~ManagerPrivate()
{
}

void ManagerPrivate::retrieveAccounts()
{
    if (Q_UNLIKELY(m_getAccountsCall)) return;


}
