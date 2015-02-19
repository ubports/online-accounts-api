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

#include "global.h"

namespace OnlineAccounts {

class AuthenticationData;

class ManagerPrivate;
class ONLINE_ACCOUNTS_EXPORT Manager: public QObject
{
    Q_OBJECT

public:
    explicit Manager(const QString &applicationId, QObject *parent = 0);
    ~Manager();

    QList<AccountId> listAccounts(const QString &service = QString());

    void requestAccess(const QString &service,
                       const AuthenticationData &authData);

Q_SIGNALS:
    void accountEnabled(AccountId account);

private:
    Q_DECLARE_PRIVATE(Manager)
    ManagerPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MANAGER_H