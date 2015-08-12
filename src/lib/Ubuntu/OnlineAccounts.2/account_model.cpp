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

    if (m_applicationId.isEmpty()) {
        QStringList parts = QString::fromUtf8(qgetenv("APP_ID")).split('_');
        if (parts.count() == 3) {
            m_applicationId = QStringList(parts.mid(0, 2)).join('_');
        }
    }

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

/*!
 * \qmltype AccountModel
 * \inqmlmodule Ubuntu.OnlineAccounts 2.0
 * \ingroup Ubuntu
 * \brief Model of available online accounts.
 *
 * The AccountModel lists all the accounts available to the application.
 * \qml
 *     import QtQuick 2.0
 *     import Ubuntu.OnlineAccounts 2.0
 *
 *     ListView {
 *         model: AccountModel {
 *             applicationId: "myapp.developer_myapp"
 *         }
 *         delegate: Text {
 *             text: model.displayName
 *         }
 *     }
 * \endqml
 *
 * The model defines the following roles:
 *
 * \list
 * \li \c displayName is the name of the account (usually the user's login)
 * \li \c accountId is a numeric ID for the account
 * \li \c serviceId is a service identifier (e.g., "myapp.developer_myapp_google")
 * \li \c authenticationMethod is the authentication method used on this
 *     account; \sa Account::authenticationMethod
 * \li \c settings is a dictionary of the settings stored into the account
 * \li \c account is the \l Account object
 * \endlist
 *
 * \sa Account
 */

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

/*!
 * \qmlproperty string AccountModel::applicationId
 *
 * The short application identifier (that is, the \c APP_ID minus the version
 * component) of the client. If not given, the identifier will be deduced from
 * the APP_ID environment variable.
 */
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

/*!
 * \qmlproperty string AccountModel::serviceId
 *
 * If this property is set, only accounts providing the given service will be
 * returned.
 */
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

/*!
 * \qmlproperty list<Account> AccountModel::accountList
 *
 * List of accounts in the model. This list has exactly the same contents as
 * the model data, and is provided as a property just as a convenience for
 * those cases when a model is not required.
 */
QList<QObject*> AccountModel::accountList() const
{
    Q_D(const AccountModel);
    QList<QObject*> objects;
    Q_FOREACH(Account *a, d->m_accounts) {
        objects.append(a);
    }
    return objects;
}

/*!
 * \qmlsignal AccountModel::accessReply(jsobject reply, jsobject authenticationData)
 *
 * Emitted when the request initiated with \l AccountModel::requestAccess()
 * completes. The \a reply object contains the access reply:
 * \list
 * \li \c account if access to an account was granted, this property will hold
 *     an \l Account object
 * \li \c errorCode error code, if an error occurred
 * \li \c errorText is a textual description of the error, not meant for the
 *     end-user; it can be used for debugging purposes
 * \endlist
 *
 * The second parameter, the \a authenticationData object, will contain the
 * authentication reply.
 */

/*!
 * \qmlmethod void AccountModel::requestAccess(string serviceId,
 *                                             jsobject parameters)
 *
 * Requests the user to grant this application access to an account providing
 * the given service. The user will be asked whether this application should be
 * given access to the desired account; if no such accounts are currently
 * registered in the system, the user will be guided to create a new one.
 *
 * It should be noted that account authorizations persist across application
 * restart; therefore, this method should be called only when the application
 * needs a new account to appear in the model.
 *
 * Each call to this method will cause the \l accessReply signal to be
 * emitted at some time later. Note that the operation will involve
 * interactions with the end-user, so don't expect a reply to be emitted
 * immediately.
 *
 * The \a parameters parameter can be used to pass authentication data
 * (similarly to how the \l Account::authenticate() method works), if it's
 * desired to perform the authentication at the same time.
 *
 * \sa accessReply
 */
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
