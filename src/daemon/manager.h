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

struct AccountInfo {
    uint accountId;
    QVariantMap details;

    AccountInfo(): accountId(0) {}
    AccountInfo(uint accountId, const QVariantMap &details):
        accountId(accountId), details(details) {}
};

Q_DECLARE_METATYPE(AccountInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                AccountInfo &info);

class ManagerPrivate;
class Manager: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(bool isIdle READ isIdle NOTIFY isIdleChanged)

public:
    explicit Manager(QObject *parent = 0);
    ~Manager();

    bool isIdle() const;

public Q_SLOTS:
    QList<AccountInfo> GetAccounts(const QVariantMap &filters);
    QVariantMap Authenticate(uint accountId, const QString &serviceId,
                             bool interactive, bool invalidate,
                             const QVariantMap &parameters);
    AccountInfo RequestAccess(const QString &serviceId,
                              const QVariantMap &parameters,
                              QVariantMap &credentials);

Q_SIGNALS:
    void isIdleChanged();
    void AccountChanged(const QString &serviceId, AccountInfo accountInfo);

private:
    bool canAccess(const QString &serviceId);
    bool checkAccess(const QString &serviceId);
    QString getPeerSecurityContext();

    Q_DECLARE_PRIVATE(Manager)
    ManagerPrivate *d_ptr;
};

#endif // ONLINE_ACCOUNTS_DAEMON_MANAGER_H
