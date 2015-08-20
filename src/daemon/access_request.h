/*
 * This file is part of OnlineAccountsDaemon
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

#ifndef ONLINE_ACCOUNTS_DAEMON_ACCESS_REQUEST_H
#define ONLINE_ACCOUNTS_DAEMON_ACCESS_REQUEST_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <sys/types.h>
#include "async_operation.h"

namespace Accounts {
class AuthData;
}

namespace OnlineAccountsDaemon {

class AccountInfo;

class AccessRequestPrivate;
class AccessRequest: public AsyncOperation
{
    Q_OBJECT

public:
    explicit AccessRequest(const CallContext &context,
                           QObject *parent = 0);
    ~AccessRequest();

    void requestAccess(const QString &applicationId,
                       const QString &serviceId,
                       const QVariantMap &parameters,
                       pid_t clientPid);

    void setAccountInfo(const AccountInfo &accountInfo,
                        const Accounts::AuthData &authData);

Q_SIGNALS:
    void loadRequest(uint accountId, const QString &serviceId);

private:
    Q_DECLARE_PRIVATE(AccessRequest)
    AccessRequestPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_ACCESS_REQUEST_H
