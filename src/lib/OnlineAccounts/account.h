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

#include <QObject>
#include <QVariant>

#include "global.h"
#include "pending_call.h"

namespace OnlineAccounts {

class AuthenticationData;
class Manager;

class AccountPrivate;
class ONLINE_ACCOUNTS_EXPORT Account: public QObject
{
    Q_OBJECT

public:
    ~Account();

    /* Returns false if account deleted or disabled */
    bool isValid() const;

    AccountId id() const;
    QString displayName() const;
    QString serviceId() const;
    AuthenticationMethod authenticationMethod() const;

    QVariant setting(const QString &key) const;

    PendingCall authenticate(const AuthenticationData &authData);

Q_SIGNALS:
    void changed();
    void disabled();

protected:
    explicit Account(Manager *manager, AccountId id, QObject *parent = 0);

private:
    friend class Manager;
    Q_DECLARE_PRIVATE(Account)
    AccountPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_ACCOUNT_H
