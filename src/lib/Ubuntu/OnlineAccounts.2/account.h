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

#ifndef ONLINE_ACCOUNTS_MODULE_ACCOUNT_H
#define ONLINE_ACCOUNTS_MODULE_ACCOUNT_H

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace OnlineAccounts {
class Account;
}

namespace OnlineAccountsModule {

class AccountPrivate;
class Account: public QObject
{
    Q_OBJECT
    Q_ENUMS(AuthenticationMethod ErrorCode)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY accountChanged)
    Q_PROPERTY(int accountId READ accountId CONSTANT)
    Q_PROPERTY(QString serviceId READ serviceId CONSTANT)
    Q_PROPERTY(AuthenticationMethod authenticationMethod \
               READ authenticationMethod CONSTANT)
    Q_PROPERTY(QVariantMap settings READ settings NOTIFY accountChanged)

public:
    enum AuthenticationMethod {
        // Make sure these are kept in sync with those from OnlineAccountsQt
        AuthenticationMethodUnknown = 0,
        AuthenticationMethodOAuth1,
        AuthenticationMethodOAuth2,
        AuthenticationMethodPassword,
        AuthenticationMethodSasl,
    };

    enum ErrorCode {
        // Make sure these are kept in sync with those from OnlineAccountsQt
        ErrorCodeNoError = 0,
        ErrorCodeNoAccount,
        ErrorCodeWrongType,
        ErrorCodeUserCanceled,
        ErrorCodePermissionDenied,
        ErrorCodeInteractionRequired,
    };

    explicit Account(OnlineAccounts::Account *account, QObject *parent = 0);
    ~Account();

    bool isValid() const;
    QString displayName() const;
    int accountId() const;
    QString serviceId() const;
    AuthenticationMethod authenticationMethod() const;
    QVariantMap settings() const;

    OnlineAccounts::Account *internalObject() const;

    Q_INVOKABLE void authenticate(const QVariantMap &params);

Q_SIGNALS:
    void validChanged();
    void accountChanged();
    void authenticationReply(const QVariantMap &authenticationData);

private:
    Q_DECLARE_PRIVATE(Account)
    AccountPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MODULE_ACCOUNT_H
