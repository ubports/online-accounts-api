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

#ifndef ONLINE_ACCOUNTS_ACCOUNT_H
#define ONLINE_ACCOUNTS_ACCOUNT_H

#include <QFuture>
#include <QObject>
#include <QVariant>

#include "global.h"

namespace OnlineAccounts {

class AuthenticationData;
class Manager;

class AccountPrivate;
class ONLINE_ACCOUNTS_EXPORT Account: public QObject
{
    Q_OBJECT

public:
    explicit Account(Manager *manager, AccountId id, QObject *parent = 0);
    ~Account();

    /* Returns false if account deleted or disabled */
    bool isValid() const;

    AccountId id() const;
    QString displayName() const;
    QString serviceId() const;
    AuthenticationMethod authenticationMethod() const;

    QVariant setting(const QString &key) const;

    template<class T> QFuture<T> authenticate(const AuthenticationData &authData);

Q_SIGNALS:
    void changed();
    void disabled();

private:
    Q_DECLARE_PRIVATE(Account)
    Q_DISABLE_COPY(Account)
    AccountPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_ACCOUNT_H
