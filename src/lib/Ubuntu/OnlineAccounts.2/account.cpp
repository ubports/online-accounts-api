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

#include "account.h"
#include "authentication_data.h"

#include "OnlineAccounts/Account"
#include "OnlineAccounts/AuthenticationData"
#include "OnlineAccounts/AuthenticationReply"
#include "OnlineAccounts/PendingCall"

using namespace OnlineAccountsModule;

namespace OnlineAccountsModule {

class AccountPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Account)

public:
    AccountPrivate(OnlineAccounts::Account *account, Account *q);

private Q_SLOTS:
    void onAuthenticationFinished();

private:
    OnlineAccounts::Account *m_account;
    mutable Account *q_ptr;
};

} // namespace

AccountPrivate::AccountPrivate(OnlineAccounts::Account *account, Account *q):
    m_account(account),
    q_ptr(q)
{
    QObject::connect(account, SIGNAL(changed()),
                     q, SIGNAL(accountChanged()));
    QObject::connect(account, SIGNAL(disabled()),
                     q, SIGNAL(validChanged()));
}

void AccountPrivate::onAuthenticationFinished()
{
    Q_Q(Account);

    auto watcher = qobject_cast<OnlineAccounts::PendingCallWatcher*>(sender());

    OnlineAccounts::AuthenticationReply reply(*watcher);
    QVariantMap map;
    if (reply.hasError()) {
        map["errorCode"] = reply.error().code();
        map["errorText"] = reply.error().text();
    } else {
        map = replyToMap(*watcher, m_account->authenticationMethod());
    }

    Q_EMIT q->authenticationReply(map);
}

/*!
 * \qmltype Account
 * \inqmlmodule Ubuntu.OnlineAccounts 2.0
 * \ingroup Ubuntu
 * \brief Representation of an online account
 *
 * The Account object holds the information related to an account and provides
 * methods to interact with it.
 * It's not possible to create such objects from QML; instead, they are
 * returned by the \l AccountModel in the \c account role or in the \l
 * {AccountModel::accountList} { \c accountList} property.
 *
 * Here's an example on how to use the account object in a delegate:
 *
 * \qml
 *     import QtQuick 2.0
 *     import Ubuntu.OnlineAccounts 2.0
 *
 *     ListView {
 *         model: AccountModel {}
 *         delegate: Button {
 *             text: "Authenticate " + model.displayName
 *             onClicked: model.account.authenticate({})
 *             Connections {
 *                 target: model.account
 *                 onAuthenticationReply: {
 *                     console.log("Access token is " + reply['AccessToken'])
 *                 }
 *             }
 *         }
 *     }
 * \endqml
 *
 * \target errorCode
 * \section3 Error codes used in this module
 * Some operations, such as the \l Account::authenticate() and the \l
 * AccountModel::requestAccess() methods, can fail and return one of these
 * error codes:
 * \list
 * \li \c Account.ErrorCodeNoAccount - The accounts is invalid
 * \li \c Account.ErrorCodeUserCanceled - The operation was canceled by the user
 * \li \c Account.ErrorCodePermissionDenied - The application has no
 *     permission to complete the operation
 * \endlist
 */

Account::Account(OnlineAccounts::Account *account, QObject *parent):
    QObject(parent),
    d_ptr(new AccountPrivate(account, this))
{
}

Account::~Account()
{
    delete d_ptr;
}

/*!
 * \qmlproperty bool Account::valid
 *
 * Whether the account object is valid; this is usually \c true, because the \c
 * AccountModel never gives out invalid accounts. However, it can happen that a
 * valid account becomes invalid while the application is using it (if, for
 * instance, the user deleted the account or revoked the application's access
 * rights to use it). As soon as this property becomes \c false, the
 * application should stop using this account.
 */
bool Account::isValid() const
{
    Q_D(const Account);
    return d->m_account->isValid();
}

/*!
 * \qmlproperty string Account::displayName
 *
 * The display name of the account. This is usually the user's login name, but
 * applications should not rely on the value of this property. Use it only for
 * display purposes.
 */
QString Account::displayName() const
{
    Q_D(const Account);
    return d->m_account->displayName();
}

/*!
 * \qmlproperty int Account::accountId
 *
 * Numeric identifier of the account. This property remains constant during the
 * lifetime of the account. Note, however, that if the user deletes the account
 * and re-creates it, its ID will be different.
 */
int Account::accountId() const
{
    Q_D(const Account);
    return d->m_account->id();
}

/*!
 * \qmlproperty int Account::serviceId
 *
 * Identifier for the service used with the account.
 */
QString Account::serviceId() const
{
    Q_D(const Account);
    return d->m_account->serviceId();
}

/*!
 * \qmlproperty enumeration Account::authenticationMethod
 *
 * The authentication method used when authenticating with the account.
 * Currently, these authentication methods are supported:
 * \list
 *   \li \c Account.AuthenticationMethodOAuth1 - OAuth 1.0
 *   \li \c Account.AuthenticationMethodOAuth2 - OAuth 2.0
 *   \li \c Account.AuthenticationMethodPassword - username/password
 * \endlist
 */
Account::AuthenticationMethod Account::authenticationMethod() const
{
    Q_D(const Account);
    return AuthenticationMethod(d->m_account->authenticationMethod());
}

/*!
 * \qmlproperty jsobject Account::settings
 *
 * A dictionary of the settings stored into the account.
 */
QVariantMap Account::settings() const
{
    Q_D(const Account);
    QVariantMap ret;
    Q_FOREACH(const QString &key, d->m_account->keys()) {
        ret.insert(key, d->m_account->setting(key));
    }
    return ret;
}

OnlineAccounts::Account *Account::internalObject() const
{
    Q_D(const Account);
    return d->m_account;
}

/*!
 * \qmlsignal Account::authenticationReply(jsobject authenticationData)
 *
 * Emitted when the authentication completes. The \a authenticationData object
 * will contain the authentication reply. If the authentication failed, the
 * following two keys will be present:
 * \list
 * \li \c errorCode is an \l {errorCode} {error code}
 * \li \c errorText is a textual description of the error, not meant for the
 *     end-user; it can be used for debugging purposes
 * \endlist
 */

/*!
 * \qmlmethod void Account::authenticate(jsobject params)
 *
 * Perform the authentication on this account. The \a params parameter can be
 * used to pass authentication data, such as the ClientId and ClientSecret used
 * in the OAuth flow.
 *
 * Each call to this method will cause the \l authenticationReply signal to be
 * emitted at some time later. Note that the authentication might involve
 * interactions with the network or with the end-user, so don't expect a reply
 * to be emitted immediately.
 *
 * \sa authenticationReply
 */
void Account::authenticate(const QVariantMap &params)
{
    Q_D(Account);
    auto method = d->m_account->authenticationMethod();
    OnlineAccounts::PendingCall call =
        d->m_account->authenticate(authenticationDataFromMap(params, method));
    OnlineAccounts::PendingCallWatcher *watcher =
        new OnlineAccounts::PendingCallWatcher(call, d->m_account);
    QObject::connect(watcher, SIGNAL(finished()),
                     d, SLOT(onAuthenticationFinished()));
}

#include "account.moc"
