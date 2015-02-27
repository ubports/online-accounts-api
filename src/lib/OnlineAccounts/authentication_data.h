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

#ifndef ONLINE_ACCOUNTS_AUTHENTICATION_DATA_H
#define ONLINE_ACCOUNTS_AUTHENTICATION_DATA_H

#include <QByteArray>
#include <QList>
#include <QSharedDataPointer>

#include "error.h"
#include "global.h"
#include "pending_call.h"

namespace OnlineAccounts {

class Manager;

class AuthenticationDataPrivate;
class ONLINE_ACCOUNTS_EXPORT AuthenticationData
{
public:
    AuthenticationData(const AuthenticationData &other);
    virtual ~AuthenticationData();

    AuthenticationMethod method() const;

    void setInteractive(bool interactive);
    bool interactive() const;

    void invalidateCachedReply();
    bool mustInvalidateCachedReply() const;

protected:
    AuthenticationData(AuthenticationDataPrivate *priv);
    QSharedDataPointer<AuthenticationDataPrivate> d;

private:
    friend class Manager;
    friend class ManagerPrivate;
    AuthenticationData();
};

class AuthenticationReplyPrivate;
class ONLINE_ACCOUNTS_EXPORT AuthenticationReply
{
public:
    AuthenticationReply(const PendingCall &call);
    virtual ~AuthenticationReply();

    bool hasError() const { return error().isValid(); }
    Error error() const;

protected:
    AuthenticationReply(AuthenticationReplyPrivate *priv);
    AuthenticationReplyPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(AuthenticationReply)
    Q_DISABLE_COPY(AuthenticationReply)
};

/* OAuth 2.0 */

class ONLINE_ACCOUNTS_EXPORT OAuth2Data: public AuthenticationData
{
public:
    OAuth2Data();

    void setClientId(const QByteArray &id);
    QByteArray clientId() const;

    void setClientSecret(const QByteArray &secret);
    QByteArray clientSecret() const;

    void setScopes(const QList<QByteArray> &scopes);
    QList<QByteArray> scopes() const;
};

class OAuth2ReplyPrivate;
class ONLINE_ACCOUNTS_EXPORT OAuth2Reply: public AuthenticationReply
{
public:
    OAuth2Reply(const PendingCall &call);
    ~OAuth2Reply();

    QByteArray accessToken() const;
    int expiresIn() const;
    QList<QByteArray> grantedScopes() const;

private:
    Q_DECLARE_PRIVATE(OAuth2Reply)
    Q_DISABLE_COPY(OAuth2Reply)
};

/* OAuth 1.0a */

class ONLINE_ACCOUNTS_EXPORT OAuth1Data: public AuthenticationData
{
public:
    OAuth1Data();

    void setConsumerKey(const QByteArray &consumerkey);
    QByteArray consumerKey() const;

    void setConsumerSecret(const QByteArray &consumerSecret);
    QByteArray consumerSecret() const;
};

class OAuth1ReplyPrivate;
class ONLINE_ACCOUNTS_EXPORT OAuth1Reply: public AuthenticationReply
{
public:
    OAuth1Reply(const PendingCall &call);
    ~OAuth1Reply();

    QByteArray consumerKey() const;
    QByteArray consumerSecret() const;
    QByteArray token() const;
    QByteArray tokenSecret() const;
    QByteArray signatureMethod() const;

private:
    Q_DECLARE_PRIVATE(OAuth1Reply)
    Q_DISABLE_COPY(OAuth1Reply)
};

/* Password */

class ONLINE_ACCOUNTS_EXPORT PasswordData: public AuthenticationData
{
public:
    PasswordData();
};

class PasswordReplyPrivate;
class ONLINE_ACCOUNTS_EXPORT PasswordReply: public AuthenticationReply
{
public:
    PasswordReply(const PendingCall &call);
    ~PasswordReply();

    QByteArray username() const;
    QByteArray password() const;

private:
    Q_DECLARE_PRIVATE(PasswordReply)
    Q_DISABLE_COPY(PasswordReply)
};

} // namespace

#endif // ONLINE_ACCOUNTS_AUTHENTICATION_DATA_H
