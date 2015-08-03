/*
 * This file is part of OnlineAccountsModule
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

#include "account_model.h"

#include "account.h"
#include "authentication_data.h"

#include "OnlineAccounts/Account"
#include "OnlineAccounts/AuthenticationData"
#include "OnlineAccounts/Manager"
#include "OnlineAccounts/PendingCall"
#include <QDebug>
#include <QQmlEngine>
#include <QVariantMap>

using namespace OnlineAccountsModule;

namespace OnlineAccountsModule {

class AccountModelPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AccountModel)

public:
    AccountModelPrivate(AccountModel *q);

    void queueUpdate();
    Account *handleAccount(OnlineAccounts::Account *account);

private Q_SLOTS:
    void update();
    void updateAccountList();
    void onAccountAvailable(OnlineAccounts::Account *account);
    void onAccessRequestFinished();
    void onAccountValidChanged();
    void onAccountChanged();

private:
    QHash<int, QByteArray> roleNames;
    OnlineAccounts::Manager *m_manager;
    QList<Account*> m_accounts;
    QString m_applicationId;
    QString m_serviceId;
    bool m_updateQueued;
    bool m_applicationIdChanged;
    bool m_serviceIdChanged;
    mutable AccountModel *q_ptr;
};

} // namespace

AccountModelPrivate::AccountModelPrivate(AccountModel *q):
    QObject(q),
    m_manager(0),
    m_updateQueued(true), // because componentComplete will be called
    m_applicationIdChanged(false),
    m_serviceIdChanged(false),
    q_ptr(q)
{
    roleNames[AccountModel::DisplayNameRole] = "displayName";
    roleNames[AccountModel::ValidRole] = "valid";
    roleNames[AccountModel::AccountIdRole] = "accountId";
    roleNames[AccountModel::ServiceIdRole] = "serviceId";
    roleNames[AccountModel::AuthenticationMethodRole] = "authenticationMethod";
    roleNames[AccountModel::SettingsRole] = "settings";
    roleNames[AccountModel::AccountRole] = "account";
}

void AccountModelPrivate::queueUpdate()
{
    if (m_updateQueued) return;

    m_updateQueued = true;
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void AccountModelPrivate::updateAccountList()
{
    Q_Q(AccountModel);

    m_serviceIdChanged = false;
    auto accountObjects = m_manager->availableAccounts(m_serviceId);
    q->beginResetModel();
    m_accounts.clear();
    Q_FOREACH(OnlineAccounts::Account *account, accountObjects) {
        handleAccount(account);
    }
    q->endResetModel();
    Q_EMIT q->accountListChanged();
}

Account *AccountModelPrivate::handleAccount(OnlineAccounts::Account *account)
{
    /* First, check if the account is already handled */
    Q_FOREACH(Account *a, m_accounts) {
        if (account == a->internalObject()) {
            return a;
        }
    }
    Account *a = new Account(account, this);
    QQmlEngine::setObjectOwnership(a, QQmlEngine::CppOwnership);
    QObject::connect(a, SIGNAL(validChanged()),
                     this, SLOT(onAccountValidChanged()));
    QObject::connect(a, SIGNAL(accountChanged()),
                     this, SLOT(onAccountChanged()));
    m_accounts.append(a);
    return a;
}

void AccountModelPrivate::update()
{
    m_updateQueued = false;

    if (m_applicationIdChanged) {
        delete m_manager;
        m_manager = new OnlineAccounts::Manager(m_applicationId);
        QObject::connect(m_manager, SIGNAL(ready()),
                         this, SLOT(updateAccountList()));
        QObject::connect(m_manager,
                         SIGNAL(accountAvailable(OnlineAccounts::Account*)),
                         this,
                         SLOT(onAccountAvailable(OnlineAccounts::Account*)));
        m_applicationIdChanged = false;
    }

    if (m_serviceIdChanged && m_manager->isReady()) {
        updateAccountList();
    }
}

void AccountModelPrivate::onAccountAvailable(OnlineAccounts::Account *account)
{
    Q_Q(AccountModel);

    if (!m_serviceId.isEmpty() && account->serviceId() != m_serviceId) {
        // ignore this account
        return;
    }

    int index = m_accounts.count();
    q->beginInsertRows(QModelIndex(), index, index);
    handleAccount(account);
    q->endInsertRows();
    Q_EMIT q->accountListChanged();
}

void AccountModelPrivate::onAccessRequestFinished()
{
    Q_Q(AccountModel);

    auto watcher = qobject_cast<OnlineAccounts::PendingCallWatcher*>(sender());

    OnlineAccounts::RequestAccessReply reply(*watcher);
    QVariantMap accountData;
    QVariantMap authenticationData;
    if (reply.hasError()) {
        accountData["errorCode"] = reply.error().code();
        accountData["errorText"] = reply.error().text();
    } else {
        OnlineAccounts::Account *account = reply.account();
        accountData["account"] =
            QVariant::fromValue<QObject*>(handleAccount(account));
        auto method = account->authenticationMethod();
        authenticationData = replyToMap(*watcher, method);
    }

    Q_EMIT q->accessReply(accountData, authenticationData);
}

void AccountModelPrivate::onAccountValidChanged()
{
    Q_Q(AccountModel);

    Account *account = qobject_cast<Account*>(sender());
    int i = m_accounts.indexOf(account);
    if (Q_UNLIKELY(i < 0)) {
        qWarning() << "Got signal from unhandled account!";
        return;
    }

    Q_ASSERT(!account->isValid());
    q->beginRemoveRows(QModelIndex(), i, i);
    QObject::disconnect(account, 0, this, 0);
    account->deleteLater();
    m_accounts.removeAt(i);
    q->endRemoveRows();
    Q_EMIT q->accountListChanged();
}

void AccountModelPrivate::onAccountChanged()
{
    Q_Q(AccountModel);

    Account *account = qobject_cast<Account*>(sender());
    int i = m_accounts.indexOf(account);
    if (Q_UNLIKELY(i < 0)) {
        qWarning() << "Got signal from unhandled account!";
        return;
    }

    QModelIndex idx = q->index(i, 0);
    q->dataChanged(idx, idx);
}

AccountModel::AccountModel(QObject *parent):
    QAbstractListModel(parent),
    d_ptr(new AccountModelPrivate(this))
{
    QObject::connect(this, SIGNAL(modelReset()),
                     this, SIGNAL(countChanged()));
    QObject::connect(this, SIGNAL(rowsInserted(const QModelIndex &,int,int)),
                     this, SIGNAL(countChanged()));
    QObject::connect(this, SIGNAL(rowsRemoved(const QModelIndex &,int,int)),
                     this, SIGNAL(countChanged()));
}

AccountModel::~AccountModel()
{
    delete d_ptr;
}

void AccountModel::classBegin()
{
}

void AccountModel::componentComplete()
{
    Q_D(AccountModel);
    d->update();
}

void AccountModel::setApplicationId(const QString &applicationId)
{
    Q_D(AccountModel);
    if (applicationId == d->m_applicationId) return;

    d->m_applicationId = applicationId;
    d->m_applicationIdChanged = true;
    d->queueUpdate();
    Q_EMIT applicationIdChanged();
}

QString AccountModel::applicationId() const
{
    Q_D(const AccountModel);
    return d->m_applicationId;
}

void AccountModel::setServiceId(const QString &serviceId)
{
    Q_D(AccountModel);
    if (serviceId == d->m_serviceId) return;

    d->m_serviceId = serviceId;
    d->m_serviceIdChanged = true;
    d->queueUpdate();
    Q_EMIT serviceIdChanged();
}

QString AccountModel::serviceId() const
{
    Q_D(const AccountModel);
    return d->m_serviceId;
}

QList<Account*> AccountModel::accountList() const
{
    Q_D(const AccountModel);
    return d->m_accounts;
}

void AccountModel::requestAccess(const QString &service,
                                 const QVariantMap &parameters)
{
    Q_D(AccountModel);
    OnlineAccounts::PendingCall call =
        d->m_manager->requestAccess(service,
                                    authenticationDataFromMap(parameters));
    OnlineAccounts::PendingCallWatcher *watcher =
        new OnlineAccounts::PendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished()),
                     d, SLOT(onAccessRequestFinished()));
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const AccountModel);
    Q_UNUSED(parent);
    return d->m_accounts.count();
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    Q_D(const AccountModel);

    int i = index.row();
    if (i < 0 || i >= d->m_accounts.count()) return QVariant();

    Account *account = d->m_accounts.at(i);
    QVariant ret;

    switch (role) {
    case Qt::DisplayRole:
        ret = QString("%1 - %2").
            arg(account->displayName()).
            arg(account->serviceId());
        break;
    case DisplayNameRole:
        ret = account->displayName();
        break;
    case ValidRole:
        ret = account->isValid();
        break;
    case AccountIdRole:
        ret = account->accountId();
        break;
    case ServiceIdRole:
        ret = account->serviceId();
        break;
    case AuthenticationMethodRole:
        ret = account->authenticationMethod();
        break;
    case SettingsRole:
        ret = account->settings();
        break;
    case AccountRole:
        ret = QVariant::fromValue<QObject*>(account);
        break;
    }

    return ret;
}

QHash<int, QByteArray> AccountModel::roleNames() const
{
    Q_D(const AccountModel);
    return d->roleNames;
}

#include "account_model.moc"
