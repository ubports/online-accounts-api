/*
 * This file is part of OnlineAccountsModule
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

#include "account.h"
#include "authentication_data.h"

#include "OnlineAccounts/Account"
#include "OnlineAccounts/AuthenticationData"
#include "OnlineAccounts/AuthenticationReply"
#include "OnlineAccounts/PendingCall"

using namespace OnlineAccountsModule;

namespace OnlineAccountsModule {

class AccountPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Account)

public:
    AccountPrivate(OnlineAccounts::Account *account, Account *q);

private Q_SLOTS:
    void onAuthenticationFinished();

private:
    OnlineAccounts::Account *m_account;
    mutable Account *q_ptr;
};

} // namespace

AccountPrivate::AccountPrivate(OnlineAccounts::Account *account, Account *q):
    m_account(account),
    q_ptr(q)
{
    QObject::connect(account, SIGNAL(changed()),
                     q, SIGNAL(accountChanged()));
    QObject::connect(account, SIGNAL(disabled()),
                     q, SIGNAL(validChanged()));
}

void AccountPrivate::onAuthenticationFinished()
{
    Q_Q(Account);

    auto watcher = qobject_cast<OnlineAccounts::PendingCallWatcher*>(sender());

    OnlineAccounts::AuthenticationReply reply(*watcher);
    QVariantMap map;
    if (reply.hasError()) {
        map["errorCode"] = reply.error().code();
        map["errorText"] = reply.error().text();
    } else {
        map = replyToMap(*watcher, m_account->authenticationMethod());
    }

    Q_EMIT q->authenticationReply(map);
}

Account::Account(OnlineAccounts::Account *account, QObject *parent):
    QObject(parent),
    d_ptr(new AccountPrivate(account, this))
{
}

Account::~Account()
{
    delete d_ptr;
}

bool Account::isValid() const
{
    Q_D(const Account);
    return d->m_account->isValid();
}

QString Account::displayName() const
{
    Q_D(const Account);
    return d->m_account->displayName();
}

int Account::accountId() const
{
    Q_D(const Account);
    return d->m_account->id();
}

QString Account::serviceId() const
{
    Q_D(const Account);
    return d->m_account->serviceId();
}

Account::AuthenticationMethod Account::authenticationMethod() const
{
    Q_D(const Account);
    return AuthenticationMethod(d->m_account->authenticationMethod());
}

QVariantMap Account::settings() const
{
    /* FIXME: libOnlineAccountsQt lacks a way to retrieve the list of settings;
     * until that is available, we cannot do much here */
    return QVariantMap();
}

OnlineAccounts::Account *Account::internalObject() const
{
    Q_D(const Account);
    return d->m_account;
}

void Account::authenticate(const QVariantMap &params)
{
    Q_D(Account);
    auto method = d->m_account->authenticationMethod();
    OnlineAccounts::PendingCall call =
        d->m_account->authenticate(authenticationDataFromMap(params, method));
    OnlineAccounts::PendingCallWatcher *watcher =
        new OnlineAccounts::PendingCallWatcher(call, d->m_account);
    QObject::connect(watcher, SIGNAL(finished()),
                     d, SLOT(onAuthenticationFinished()));
}

#include "account.moc"
