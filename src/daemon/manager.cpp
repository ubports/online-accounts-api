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
#include <Accounts/AccountService>
#include <Accounts/Application>
#include <Accounts/AuthData>
#include <Accounts/Manager>
#include <Accounts/Service>
#include <QDebug>
#include <QHash>
#include <QPair>
#include <QSet>
#include "dbus_constants.h"
#include "manager_adaptor.h"
#include "state_saver.h"

static const char FORBIDDEN_ERROR[] = "com.ubuntu.OnlineAccounts.Error.Forbidden";

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

struct ActiveAccount {
    ActiveAccount(): accountService(0) {}

    Accounts::AccountService *accountService;
    QSet<QString> clients;
};

typedef QPair<Accounts::AccountId,QString> AccountCoordinates;

class ManagerPrivate {
    Q_DECLARE_PUBLIC(Manager)

public:
    ManagerPrivate(Manager *q);

    void loadActiveAccounts();
    void addActiveAccount(Accounts::AccountId accountId,
                          const QString &serviceName,
                          const QString &client);

    int authMethod(const Accounts::AuthData &authData);
    AccountInfo readAccountInfo(Accounts::Account *account,
                                const Accounts::Service &service);
    QList<AccountInfo> getAccounts(const QVariantMap &filters,
                                   const QString &context);
    bool canAccess(const QString &context, const QString &serviceId);

private:
    Accounts::Manager m_manager;
    StateSaver m_stateSaver;
    bool m_mustEmitNotifications;
    QHash<AccountCoordinates,ActiveAccount> m_activeAccounts;
    bool m_isIdle;
    mutable Manager *q_ptr;
};

} // namespace

ManagerPrivate::ManagerPrivate(Manager *q):
    m_mustEmitNotifications(false),
    m_isIdle(true),
    q_ptr(q)
{
    loadActiveAccounts();
}

void ManagerPrivate::loadActiveAccounts()
{
    QStringList oldClients = m_stateSaver.clients();
    Q_FOREACH(const QString &client, oldClients) {
    }
}

void ManagerPrivate::addActiveAccount(Accounts::AccountId accountId,
                                      const QString &serviceName,
                                      const QString &client)
{
    ActiveAccount activeAccount =
        m_activeAccounts[AccountCoordinates(accountId, serviceName)];
    activeAccount.clients.insert(client);
    if (!activeAccount.accountService) {
        Accounts::Account *account = m_manager.account(accountId);
        if (Q_UNLIKELY(!account)) return;

        Accounts::Service service = m_manager.service(serviceName);
        activeAccount.accountService =
            new Accounts::AccountService(account, service);
    }
}

int ManagerPrivate::authMethod(const Accounts::AuthData &authData)
{
    QString method = authData.method();
    QString mechanism = authData.mechanism();
    if (method == "oauth2") {
        if (mechanism == "web_server" || mechanism == "user_agent") {
            return ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH2;
        } else if (mechanism == "HMAC-SHA1" || mechanism == "PLAINTEXT") {
            return ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH1;
        }
    } else if (method == "password") {
        return ONLINE_ACCOUNTS_AUTH_METHOD_PASSWORD;
    }

    return ONLINE_ACCOUNTS_AUTH_METHOD_UNKNOWN;
}

AccountInfo ManagerPrivate::readAccountInfo(Accounts::Account *account,
                                            const Accounts::Service &service)
{
    QVariantMap info;
    info[ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME] = account->displayName();
    info[ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID] = service.name();

    Accounts::AccountService as(account, service);
    info[ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD] = authMethod(as.authData());
    QString settingsPrefix(QStringLiteral(ONLINE_ACCOUNTS_INFO_KEY_SETTINGS));
    Q_FOREACH(const QString &key, as.allKeys()) {
        info[settingsPrefix + key] = as.value(key);
    }

    return AccountInfo(account->id(), info);
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
    new ManagerAdaptor(this);
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

QList<AccountInfo> Manager::getAccounts(const QVariantMap &filters,
                                        const CallContext &context)
{
    Q_D(Manager);
    return d->getAccounts(filters, context.securityContext());
}

void Manager::authenticate(uint accountId, const QString &serviceId,
                           bool interactive, bool invalidate,
                           const QVariantMap &parameters,
                           const CallContext &context)
{
    Q_D(Manager);
    Q_UNUSED(accountId);
    Q_UNUSED(interactive);
    Q_UNUSED(invalidate);
    Q_UNUSED(parameters);

    if (!d->canAccess(context.securityContext(), serviceId)) {
        context.sendError(FORBIDDEN_ERROR,
                          QString("Access to service ID %1 forbidden").arg(serviceId));
        return;
    }
}

void Manager::requestAccess(const QString &serviceId,
                            const QVariantMap &parameters,
                            const CallContext &context)
{
    Q_D(Manager);
    Q_UNUSED(parameters);

    if (!d->canAccess(context.securityContext(), serviceId)) {
        context.sendError(FORBIDDEN_ERROR,
                          QString("Access to service ID %1 forbidden").arg(serviceId));
        return;
    }
}
