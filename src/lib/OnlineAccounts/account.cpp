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

#include "account_p.h"

#include "manager_p.h"

using namespace OnlineAccounts;

AccountPrivate::AccountPrivate(Manager *manager, const AccountInfo &info):
    m_manager(manager),
    m_info(info),
    m_isValid(true),
    q_ptr(0)
{
}

AccountPrivate::~AccountPrivate()
{
}

void AccountPrivate::setInvalid()
{
    Q_Q(Account);
    m_isValid = false;
    Q_EMIT q->disabled();
}

void AccountPrivate::update(const AccountInfo &info)
{
    Q_Q(Account);

    if (info != m_info) {
        m_info = info;
        Q_EMIT q->changed();
    }
}

Service AccountPrivate::service() const
{
    return m_manager->d_ptr->service(m_info.service());
}

Account::Account(AccountPrivate *priv, QObject *parent):
    QObject(parent),
    d_ptr(priv)
{
    priv->q_ptr = this;
}

Account::~Account()
{
    delete d_ptr;
    d_ptr = 0;
}

bool Account::isValid() const
{
    Q_D(const Account);
    return d->m_isValid;
}

Service Account::service() const
{
    Q_D(const Account);
    return d->service();
}

AccountId Account::id() const
{
    Q_D(const Account);
    return d->m_info.id();
}

QString Account::displayName() const
{
    Q_D(const Account);
    return d->m_info.displayName();
}

QString Account::serviceId() const
{
    Q_D(const Account);
    return d->m_info.service();
}

AuthenticationMethod Account::authenticationMethod() const
{
    Q_D(const Account);
    return d->m_info.authenticationMethod();
}

QStringList Account::keys() const
{
    Q_D(const Account);
    return d->m_info.keys();
}

QVariant Account::setting(const QString &key) const
{
    Q_D(const Account);
    return d->m_info.setting(key);
}

PendingCall Account::authenticate(const AuthenticationData &authData)
{
    Q_D(Account);
    ManagerPrivate *mPriv = d->m_manager->d_ptr;
    return mPriv->authenticate(d->m_info, authData);
}
