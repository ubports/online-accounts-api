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

class QDBusConnection;

namespace OnlineAccounts {

class Account;
class AuthenticationData;
class RequestAccessReplyPrivate;

class ManagerPrivate;
class ONLINE_ACCOUNTS_EXPORT Manager: public QObject
{
    Q_OBJECT

public:
    explicit Manager(const QString &applicationId, QObject *parent = 0);
    Manager(const QString &applicationId, const QDBusConnection &bus,
            QObject *parent = 0);
    ~Manager();

    bool isReady() const;
    void waitForReady();

    QList<Account*> availableAccounts(const QString &service = QString());
    Account *account(AccountId accountId);
    Account *account(AccountId accountId, const QString &service);

    PendingCall requestAccess(const QString &service,
                              const AuthenticationData &authData);

Q_SIGNALS:
    void ready();
    void accountAvailable(OnlineAccounts::Account *account);

private:
    friend class Account;
    friend class RequestAccessReplyPrivate;
    Q_DECLARE_PRIVATE(Manager)
    Q_DISABLE_COPY(Manager)
    ManagerPrivate *d_ptr;
};

class ONLINE_ACCOUNTS_EXPORT RequestAccessReply
{
public:
    RequestAccessReply(const PendingCall &call);
    virtual ~RequestAccessReply();

    bool hasError() const { return error().isValid(); }
    Error error() const;

    Account *account();

private:
    Q_DECLARE_PRIVATE(RequestAccessReply)
    Q_DISABLE_COPY(RequestAccessReply)
    RequestAccessReplyPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MANAGER_H
