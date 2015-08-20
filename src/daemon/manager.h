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

#ifndef ONLINE_ACCOUNTS_DAEMON_MANAGER_H
#define ONLINE_ACCOUNTS_DAEMON_MANAGER_H

#include <QDBusArgument>
#include <QDBusContext>
#include <QList>
#include <QObject>
#include <QVariantMap>
#include "account_info.h"

namespace OnlineAccountsDaemon {

class CallContext;
class ManagerAdaptor;

class ManagerPrivate;
class Manager: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(bool isIdle READ isIdle NOTIFY isIdleChanged)

public:
    explicit Manager(QObject *parent = 0);
    ~Manager();

    bool isIdle() const;

    QList<AccountInfo> getAccounts(const QVariantMap &filters,
                                   const CallContext &context);
    void authenticate(uint accountId, const QString &serviceId,
                      bool interactive, bool invalidate,
                      const QVariantMap &parameters,
                      const CallContext &context);
    void requestAccess(const QString &serviceId,
                       const QVariantMap &parameters,
                       const CallContext &context);

public Q_SLOTS:
    void onDisconnected();

Q_SIGNALS:
    void isIdleChanged();

private:
    friend class ManagerAdaptor;
    Q_DECLARE_PRIVATE(Manager)
    ManagerPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_MANAGER_H
