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

#ifndef ONLINE_ACCOUNTS_MANAGER_H
#define ONLINE_ACCOUNTS_MANAGER_H

#include <QList>
#include <QObject>

#include "error.h"
#include "global.h"
#include "pending_call.h"

namespace OnlineAccounts {

class Account;
class AuthenticationData;

class ManagerPrivate;
class ONLINE_ACCOUNTS_EXPORT Manager: public QObject
{
    Q_OBJECT

public:
    explicit Manager(const QString &applicationId, QObject *parent = 0);
    ~Manager();

    QList<Account*> availableAccounts(const QString &service = QString());

    PendingCall requestAccess(const QString &service,
                              const AuthenticationData &authData);

Q_SIGNALS:
    void accountAvailable(Account *account);

private:
    Q_DECLARE_PRIVATE(Manager)
    Q_DISABLE_COPY(Manager)
    ManagerPrivate *d_ptr;
};

class RequestAccessReplyPrivate;
class ONLINE_ACCOUNTS_EXPORT RequestAccessReply
{
public:
    RequestAccessReply(const PendingCall &call);
    virtual ~RequestAccessReply();

    bool hasError() const { return error().isValid(); }
    Error error() const;

    Account *account() const;

private:
    Q_DECLARE_PRIVATE(RequestAccessReply)
    Q_DISABLE_COPY(RequestAccessReply)
    RequestAccessReplyPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MANAGER_H
