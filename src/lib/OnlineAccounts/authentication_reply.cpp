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

#include "authentication_data.h"

#include <QDBusMessage>
#include "dbus_constants.h"
#include "pending_call_p.h"

using namespace OnlineAccounts;

namespace OnlineAccounts {

class AuthenticationReplyPrivate
{
public:
    inline AuthenticationReplyPrivate(AuthenticationMethod method,
                                      const PendingCall &call);
    virtual ~AuthenticationReplyPrivate() {};

    void setError(const Error &error) { m_error = error; }

    const QVariantMap &data() const { return m_replyData; }

private:
    friend class AuthenticationReply;
    AuthenticationMethod m_authenticationMethod;
    PendingCall m_pendingCall;
    Error m_error;
    QVariantMap m_replyData;
};

class OAuth2ReplyPrivate: public AuthenticationReplyPrivate
{
};

class OAuth1ReplyPrivate: public AuthenticationReplyPrivate
{
};

class PasswordReplyPrivate: public AuthenticationReplyPrivate
{
};

} // namespace

AuthenticationReplyPrivate::AuthenticationReplyPrivate(AuthenticationMethod method,
                                                       const PendingCall &call):
    m_authenticationMethod(method),
    m_pendingCall(call)
{
    const PendingCallPrivate *pCall = call.d.constData();

    if (m_authenticationMethod != AuthenticationMethodUnknown &&
        m_authenticationMethod != pCall->authenticationMethod()) {
        setError(Error(Error::WrongType, "Authentication method mismatch"));
        return;
    }

    if (!m_pendingCall.isFinished()) {
        m_pendingCall.waitForFinished();
    }

    if (Q_UNLIKELY(pCall->dbusCall().isError())) {
        // TODO: handle error
        return;
    }

    QDBusMessage msg = pCall->dbusCall().reply();
    PendingCallPrivate::InvokedMethod invokedMethod = pCall->invokedMethod();
    if (invokedMethod == PendingCallPrivate::Authenticate) {
        m_replyData = msg.arguments().at(0).toMap();
    } else if (invokedMethod == PendingCallPrivate::RequestAccess) {
        m_replyData = msg.arguments().at(1).toMap();
    } else {
        qFatal("Unknown invoked method %d", invokedMethod);
    }
}

AuthenticationReply::AuthenticationReply(const PendingCall &call):
    d_ptr(new AuthenticationReplyPrivate(AuthenticationMethodUnknown, call))
{
}

AuthenticationReply::AuthenticationReply(AuthenticationReplyPrivate *priv):
    d_ptr(priv)
{
}

AuthenticationReply::~AuthenticationReply()
{
    delete d_ptr;
}

Error AuthenticationReply::error() const
{
    Q_D(const AuthenticationReply);
    return d->m_error;
}

/* OAuth 2.0 */

OAuth2Reply::OAuth2Reply(const PendingCall &call):
    AuthenticationReply(new AuthenticationReplyPrivate(AuthenticationMethodOAuth2, call))
{
}

OAuth2Reply::~OAuth2Reply()
{
}

QByteArray OAuth2Reply::accessToken() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_ACCESS_TOKEN).toByteArray();
}

int OAuth2Reply::expiresIn() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_EXPIRES_IN).toInt();
}

QList<QByteArray> OAuth2Reply::grantedScopes() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_GRANTED_SCOPES).value<QList<QByteArray> >();
}

/* OAuth 1.0a */

OAuth1Reply::OAuth1Reply(const PendingCall &call):
    AuthenticationReply(new AuthenticationReplyPrivate(AuthenticationMethodOAuth1, call))
{
}

OAuth1Reply::~OAuth1Reply()
{
}

QByteArray OAuth1Reply::consumerKey() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_KEY).toByteArray();
}

QByteArray OAuth1Reply::consumerSecret() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_SECRET).toByteArray();
}

QByteArray OAuth1Reply::token() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_TOKEN).toByteArray();
}

QByteArray OAuth1Reply::tokenSecret() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_TOKEN_SECRET).toByteArray();
}

QByteArray OAuth1Reply::signatureMethod() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_SIGNATURE_METHOD).toByteArray();
}

/* Password */

PasswordReply::PasswordReply(const PendingCall &call):
    AuthenticationReply(new AuthenticationReplyPrivate(AuthenticationMethodPassword, call))
{
}

PasswordReply::~PasswordReply()
{
}

QByteArray PasswordReply::username() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_USERNAME).toByteArray();
}

QByteArray PasswordReply::password() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_PASSWORD).toByteArray();
}
