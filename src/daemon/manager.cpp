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

#include "manager.h"

#include <Accounts/Account>
#include <Accounts/Application>
#include <Accounts/Manager>
#include <Accounts/Service>
#include <QDBusMessage>
#include <QDebug>
#include "aacontext.h"

static const char FORBIDDEN_ERROR[] = "com.ubuntu.OnlineAccounts.Error.Forbidden";

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info)
{
    argument.beginStructure();
    argument << info.accountId << info.details;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                AccountInfo &info)
{
    argument.beginStructure();
    argument >> info.accountId >> info.details;
    argument.endStructure();
    return argument;
}

class ManagerPrivate {
    Q_DECLARE_PUBLIC(Manager)

public:
    ManagerPrivate(Manager *q);

    AccountInfo readAccountInfo(Accounts::Account *account,
                                const Accounts::Service &service);
    QList<AccountInfo> getAccounts(const QVariantMap &filters,
                                   const QString &context);
    bool canAccess(const QString &context, const QString &serviceId);

private:
    Accounts::Manager m_manager;
    bool m_isIdle;
    AppArmorContext m_apparmor;
    mutable Manager *q_ptr;
};

ManagerPrivate::ManagerPrivate(Manager *q):
    m_isIdle(true),
    q_ptr(q)
{
}

AccountInfo ManagerPrivate::readAccountInfo(Accounts::Account *account,
                                            const Accounts::Service &service)
{
    Q_UNUSED(account);
    Q_UNUSED(service);
    //TODO
    return AccountInfo();
}

QList<AccountInfo> ManagerPrivate::getAccounts(const QVariantMap &filters,
                                               const QString &context)
{
    QString desiredApplicationId = filters.value("applicationId").toString();
    QString desiredServiceId = filters.value("serviceId").toString();
    Accounts::AccountId desiredAccountId = filters.value("accountId").toUInt();

    Accounts::Application application = desiredApplicationId.isEmpty() ?
        Accounts::Application() : m_manager.application(desiredApplicationId);

    QList<AccountInfo> accounts;

    Q_FOREACH(Accounts::AccountId accountId, m_manager.accountListEnabled()) {
        if (desiredAccountId != 0 && accountId != desiredAccountId) {
            continue;
        }

        Accounts::Account *account = m_manager.account(accountId);
        if (Q_UNLIKELY(!account)) continue;

        Q_FOREACH(Accounts::Service service, account->enabledServices()) {
            if (!desiredServiceId.isEmpty() &&
                service.name() != desiredServiceId) {
                continue;
            }

            if (!canAccess(context, service.name())) {
                continue;
            }

            if (application.isValid() &&
                application.serviceUsage(service).isEmpty()) {
                /* The application does not support this service */
                continue;
            }

            accounts.append(readAccountInfo(account, service));
        }
    }

    return accounts;
}

bool ManagerPrivate::canAccess(const QString &context,
                               const QString &serviceId)
{
    // Could not determine peer's AppArmor context, so deny access
    if (context.isEmpty()) {
        return false;
    }
    // Unconfined processes can access anything
    if (context == "unconfined") {
        return true;
    }

    // Try to extract the click package name from the AppArmor context.
    int pos = context.indexOf('_');
    if (pos < 0) {
        qWarning() << "AppArmor context doesn't contain package ID: " << context;
        return false;
    }
    QString pkgname = context.left(pos);

    // Do the same on the service ID: we are only dealing with
    // confined apps at this point, so only $pkgname prefixed
    // services are accessible.
    pos = serviceId.indexOf('_');
    if (pos < 0) {
        return false;
    }
    return serviceId.left(pos) == pkgname;
}

Manager::Manager(QObject *parent):
    QObject(parent),
    d_ptr(new ManagerPrivate(this))
{
}

Manager::~Manager()
{
    delete d_ptr;
}

bool Manager::isIdle() const
{
    Q_D(const Manager);
    return d->m_isIdle;
}

QList<AccountInfo> Manager::GetAccounts(const QVariantMap &filters)
{
    Q_D(Manager);
    QString context = d->m_apparmor.getPeerSecurityContext(connection(),
                                                           message());
    return d->getAccounts(filters, context);
}

QVariantMap Manager::Authenticate(uint accountId, const QString &serviceId,
                                  bool interactive, bool invalidate,
                                  const QVariantMap &parameters)
{
    Q_D(Manager);
    Q_UNUSED(accountId);
    Q_UNUSED(interactive);
    Q_UNUSED(invalidate);
    Q_UNUSED(parameters);

    QString context = d->m_apparmor.getPeerSecurityContext(connection(),
                                                           message());
    if (!d->canAccess(context, serviceId)) {
        sendErrorReply(FORBIDDEN_ERROR,
                       QString("Access to service ID %1 forbidden").arg(serviceId));
        return QVariantMap();
    }

    return QVariantMap();
}

AccountInfo Manager::RequestAccess(const QString &serviceId,
                                   const QVariantMap &parameters,
                                   QVariantMap &credentials)
{
    Q_D(Manager);
    Q_UNUSED(parameters);

    QString context = d->m_apparmor.getPeerSecurityContext(connection(),
                                                           message());
    if (!d->canAccess(context, serviceId)) {
        sendErrorReply(FORBIDDEN_ERROR,
                       QString("Access to service ID %1 forbidden").arg(serviceId));
        return AccountInfo();
    }

    credentials = QVariantMap();
    return AccountInfo();
}
