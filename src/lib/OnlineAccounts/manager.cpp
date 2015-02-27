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
#include <QDebug>
#include "account_p.h"
#include "authentication_data_p.h"
#include "dbus_constants.h"
#include "pending_call_p.h"

using namespace OnlineAccounts;

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

PendingCall ManagerPrivate::authenticate(const AccountInfo &info,
                                         const AuthenticationData &authData)
{
    Q_Q(Manager);
    QDBusPendingCall call = m_daemon.authenticate(info.id(),
                                                  info.service(),
                                                  authData.interactive(),
                                                  authData.mustInvalidateCachedReply(),
                                                  authData.d->m_parameters);
    return PendingCall(new PendingCallPrivate(q, call,
                                              PendingCallPrivate::Authenticate,
                                              authData.method()));
}

PendingCall ManagerPrivate::requestAccess(const QString &service,
                                          const QVariantMap &parameters)
{
    Q_Q(Manager);
    QDBusPendingCall call = m_daemon.requestAccess(service, parameters);
    return PendingCall(new PendingCallPrivate(q, call,
                                              PendingCallPrivate::RequestAccess,
                                              AuthenticationMethodUnknown));
}

Account *ManagerPrivate::ensureAccount(const AccountInfo &info)
{
    QHash<AccountId,AccountData>::iterator i = m_accounts.find(info.id());
    if (i == m_accounts.end()) {
        i = m_accounts.insert(info.id(), AccountData(info));
    }

    AccountData &accountData = i.value();
    if (!accountData.account) {
        accountData.account =
            new Account(new AccountPrivate(q_ptr, accountData.info), this);
    }

    return accountData.account;
}

void ManagerPrivate::retrieveAccounts()
{
    if (Q_UNLIKELY(m_getAccountsCall)) return;

    QVariantMap filters;
    filters["applicationId"] = m_applicationId;
    m_getAccountsCall =
        new QDBusPendingCallWatcher(m_daemon.getAccounts(filters));
    QObject::connect(m_getAccountsCall,
                     SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(onGetAccountsFinished()));
}

void ManagerPrivate::onGetAccountsFinished()
{
    Q_Q(Manager);

    Q_ASSERT(m_getAccountsCall);

    QDBusPendingReply<QList<AccountInfo> > reply = *m_getAccountsCall;
    if (Q_UNLIKELY(reply.isError())) {
        qWarning() << "GetAccounts call failed:" << reply.error();
        /* No special handling of the error: the Manager will simply not have
         * any account */
    } else {
        QList<AccountInfo> accountInfos = reply.argumentAt<0>();
        Q_FOREACH(const AccountInfo &info, accountInfos) {
            m_accounts.insert(info.id(), AccountData(info));
        }
    }
    m_getAccountsCall->deleteLater();
    m_getAccountsCall = 0;

    Q_EMIT q->ready();
}

Manager::Manager(const QString &applicationId, QObject *parent):
    QObject(parent),
    d_ptr(new ManagerPrivate(this, applicationId))
{
}

Manager::~Manager()
{
    delete d_ptr;
    d_ptr = 0;
}

bool Manager::isReady() const
{
    Q_D(const Manager);
    return !d->m_getAccountsCall;
}

void Manager::waitForReady()
{
    Q_D(Manager);
    if (d->m_getAccountsCall) {
        d->m_getAccountsCall->waitForFinished();
    }
}

QList<Account*> Manager::availableAccounts(const QString &service)
{
    Q_D(Manager);

    QList<Account*> result;
    for (QHash<AccountId,AccountData>::iterator i = d->m_accounts.begin();
         i != d->m_accounts.end(); i++) {
        AccountData &accountData = i.value();
        if (!service.isEmpty() && accountData.info.service() != service) continue;

        if (!accountData.account) {
            accountData.account =
                new Account(new AccountPrivate(this, accountData.info), this);
        }

        result.append(accountData.account);
    }

    return result;
}

Account *Manager::account(AccountId accountId)
{
    Q_D(Manager);

    QHash<AccountId,AccountData>::iterator i = d->m_accounts.find(accountId);
    if (Q_UNLIKELY(i == d->m_accounts.end())) return 0;

    AccountData &accountData = i.value();
    if (!accountData.account) {
        accountData.account =
            new Account(new AccountPrivate(this, accountData.info), this);
    }

    return accountData.account;
}

PendingCall Manager::requestAccess(const QString &service,
                                   const AuthenticationData &authData)
{
    Q_D(Manager);
    return d->requestAccess(service, authData.d->m_parameters);
}
