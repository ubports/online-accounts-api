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
#include "client_registry.h"
#include "dbus_constants.h"
#include "manager_adaptor.h"
#include "state_saver.h"

static const char FORBIDDEN_ERROR[] = "com.ubuntu.OnlineAccounts.Error.Forbidden";

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

struct ActiveAccount {
    ActiveAccount(): accountService(0) {}

    bool isValid() const { return accountService != 0; }
    Accounts::AccountService *accountService;
    QSet<QString> clients;
};

typedef QPair<Accounts::AccountId,QString> AccountCoordinates;
typedef QHash<QString,Accounts::Application> ClientMap;

class ManagerPrivate {
    Q_DECLARE_PUBLIC(Manager)

public:
    ManagerPrivate(Manager *q);

    void loadActiveAccounts();
    ActiveAccount &addActiveAccount(Accounts::AccountId accountId,
                                    const QString &serviceName,
                                    const QString &client);
    ActiveAccount &addActiveAccount(Accounts::AccountId accountId,
                                    const QString &serviceName,
                                    const QStringList &clients);

    int authMethod(const Accounts::AuthData &authData);
    AccountInfo readAccountInfo(const Accounts::AccountService *as);
    QList<AccountInfo> getAccounts(const QVariantMap &filters,
                                   const CallContext &context);
    bool canAccess(const QString &context, const QString &serviceId);

    void notifyAccountChange(const ActiveAccount &account, uint change);

private:
    ManagerAdaptor *m_adaptor;
    Accounts::Manager m_manager;
    StateSaver m_stateSaver;
    bool m_mustEmitNotifications;
    QHash<AccountCoordinates,ActiveAccount> m_activeAccounts;
    ClientMap m_clients;
    bool m_isIdle;
    mutable Manager *q_ptr;
};

} // namespace

ManagerPrivate::ManagerPrivate(Manager *q):
    m_adaptor(new ManagerAdaptor(q)),
    m_mustEmitNotifications(false),
    m_isIdle(true),
    q_ptr(q)
{
    loadActiveAccounts();
}

void ManagerPrivate::loadActiveAccounts()
{
    QSet<Client> oldClients = m_stateSaver.clients().toSet();
    ClientRegistry *clientRegistry = ClientRegistry::instance();
    QStringList oldClientNames;
    Q_FOREACH(const Client &client, oldClients) {
        oldClientNames.append(client.first);
    }
    clientRegistry->registerActiveClients(oldClientNames);

    /* Build the table of the active clients */
    QStringList activeClientNames = clientRegistry->clients();
    Q_FOREACH(const Client &client, oldClients) {
        if (activeClientNames.contains(client.first)) {
            m_clients[client.first] = m_manager.application(client.second);
        }
    }

    QList<AccountInfo> oldAccounts = m_stateSaver.accounts();
    Q_FOREACH(const AccountInfo &accountInfo, oldAccounts) {
        // If no clients are interested in this account, ignore it
        Accounts::Service service =
            m_manager.service(accountInfo.serviceId());
        if (Q_UNLIKELY(!service.isValid())) continue;

        QStringList clients;
        for (auto i = m_clients.constBegin();
             i != m_clients.constEnd(); i++) {
            const Accounts::Application &application = i.value();
            if (!application.serviceUsage(service).isEmpty()) {
                clients.append(i.key());
            }
        }
        if (clients.isEmpty()) {
            // no one is interested in this account
            continue;
        }

        ActiveAccount &activeAccount =
            addActiveAccount(accountInfo.accountId, service.name(), clients);
        if (Q_UNLIKELY(!activeAccount.isValid())) continue;

        if (!activeAccount.accountService->isEnabled()) {
            // the account got disabled while this daemon was not running
            notifyAccountChange(activeAccount,
                                ONLINE_ACCOUNTS_INFO_CHANGE_DISABLED);
        } else {
            AccountInfo newAccountInfo =
                readAccountInfo(activeAccount.accountService);
            if (newAccountInfo.details != accountInfo.details) {
                notifyAccountChange(activeAccount,
                                    ONLINE_ACCOUNTS_INFO_CHANGE_UPDATED);
            }
        }
    }

    /* Last, go through all active clients and see if there are new accounts
     * for any of them. */
    Q_FOREACH(Accounts::AccountId accountId, m_manager.accountListEnabled()) {
        Accounts::Account *account = m_manager.account(accountId);
        if (Q_UNLIKELY(!account)) continue;

        Q_FOREACH(Accounts::Service service, account->enabledServices()) {
            QStringList interestedClients;
            for (auto i = m_clients.constBegin();
                 i != m_clients.constEnd(); i++) {
                const Accounts::Application &application = i.value();

                if (!application.serviceUsage(service).isEmpty()) {
                    interestedClients.append(i.key());
                }
            }
            AccountCoordinates coords(accountId, service.name());
            if (m_activeAccounts.contains(coords)) continue;

            ActiveAccount &activeAccount =
                addActiveAccount(accountId, service.name(), interestedClients);
            notifyAccountChange(activeAccount,
                                ONLINE_ACCOUNTS_INFO_CHANGE_ENABLED);
        }
    }
}

void ManagerPrivate::notifyAccountChange(const ActiveAccount &account,
                                         uint change)
{
    AccountInfo info = readAccountInfo(account.accountService);
    m_adaptor->notifyAccountChange(info, change);
}

ActiveAccount &ManagerPrivate::addActiveAccount(Accounts::AccountId accountId,
                                                const QString &serviceName,
                                                const QString &client)
{
    return addActiveAccount(accountId, serviceName, QStringList() << client);
}

ActiveAccount &ManagerPrivate::addActiveAccount(Accounts::AccountId accountId,
                                                const QString &serviceName,
                                                const QStringList &clients)
{
    ActiveAccount &activeAccount =
        m_activeAccounts[AccountCoordinates(accountId, serviceName)];
    activeAccount.clients += clients.toSet();
    if (!activeAccount.accountService) {
        Accounts::Account *account = m_manager.account(accountId);
        if (Q_UNLIKELY(!account)) return activeAccount;

        Accounts::Service service = m_manager.service(serviceName);
        activeAccount.accountService =
            new Accounts::AccountService(account, service);
    }

    return activeAccount;
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

AccountInfo ManagerPrivate::readAccountInfo(const Accounts::AccountService *as)
{
    QVariantMap info;

    info[ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME] = as->account()->displayName();
    info[ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID] = as->service().name();

    info[ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD] = authMethod(as->authData());
    QString settingsPrefix(QStringLiteral(ONLINE_ACCOUNTS_INFO_KEY_SETTINGS));
    Q_FOREACH(const QString &key, as->allKeys()) {
        info[settingsPrefix + key] = as->value(key);
    }

    return AccountInfo(as->account()->id(), info);
}

QList<AccountInfo> ManagerPrivate::getAccounts(const QVariantMap &filters,
                                               const CallContext &context)
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

            if (!canAccess(context.securityContext(), service.name())) {
                continue;
            }

            if (application.isValid() &&
                application.serviceUsage(service).isEmpty()) {
                /* The application does not support this service */
                continue;
            }

            ActiveAccount &activeAccount =
                addActiveAccount(accountId, service.name(),
                                 context.clientName());
            accounts.append(readAccountInfo(activeAccount.accountService));
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

QList<AccountInfo> Manager::getAccounts(const QVariantMap &filters,
                                        const CallContext &context)
{
    Q_D(Manager);
    return d->getAccounts(filters, context);
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
