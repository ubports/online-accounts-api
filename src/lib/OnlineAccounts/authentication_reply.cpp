/*
 * This file is part of libOnlineAccounts
 *
 * Copyright (C) 2015-2016 Canonical Ltd.
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

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDebug>
#include "OnlineAccountsDaemon/dbus_constants.h"
#include "error_p.h"
#include "pending_call_p.h"

using namespace OnlineAccounts;

static QVariant expandDBusArguments(const QVariant &variant)
{
    if (variant.userType() == qMetaTypeId<QDBusArgument>()) {
        QDBusArgument argument = variant.value<QDBusArgument>();
        if (argument.currentType() == QDBusArgument::MapType) {
            /* Assume that all maps are a{sv} */
            QVariantMap map = qdbus_cast<QVariantMap>(argument);
            QVariantMap expandedMap;
            QMapIterator<QString, QVariant> it(map);
            while (it.hasNext()) {
                it.next();
                expandedMap.insert(it.key(), expandDBusArguments(it.value()));
            }
            return expandedMap;
        } else if (argument.currentType() == QDBusArgument::ArrayType) {
            if (argument.currentSignature() == "aay") {
                QList<QByteArray> arrayList;
                argument >> arrayList;
                return QVariant::fromValue(arrayList);
            } else {
                /* We don't know how to handle other types */
                qWarning() << "unhandled type" << argument.currentSignature();
                return argument.asVariant();
            }
        } else {
            /* We don't know how to handle other types */
            return argument.asVariant();
        }
    } else {
        return variant;
    }
}

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

class SaslReplyPrivate: public AuthenticationReplyPrivate
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
        pCall->authenticationMethod() != AuthenticationMethodUnknown &&
        m_authenticationMethod != pCall->authenticationMethod()) {
        setError(Error(Error::WrongType, "Authentication method mismatch"));
        return;
    }

    if (!m_pendingCall.isFinished()) {
        m_pendingCall.waitForFinished();
    }

    if (Q_UNLIKELY(pCall->dbusCall().isError())) {
        setError(errorFromDBus(pCall->dbusCall().error()));
        return;
    }

    QDBusMessage msg = pCall->dbusCall().reply();
    PendingCallPrivate::InvokedMethod invokedMethod = pCall->invokedMethod();
    if (invokedMethod == PendingCallPrivate::Authenticate) {
        m_replyData = expandDBusArguments(msg.arguments().at(0)).toMap();
    } else if (invokedMethod == PendingCallPrivate::RequestAccess) {
        m_replyData = expandDBusArguments(msg.arguments().at(1)).toMap();
    } else {
        qFatal("Unknown invoked method %d", invokedMethod);
    }
    qDebug() << "reply data:" << m_replyData;
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

QVariantMap AuthenticationReply::data() const
{
    Q_D(const AuthenticationReply);
    return d->data();
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

/* SASL */

SaslReply::SaslReply(const PendingCall &call):
    AuthenticationReply(new AuthenticationReplyPrivate(AuthenticationMethodSasl, call))
{
}

SaslReply::~SaslReply()
{
}

QString SaslReply::chosenMechanism() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_CHOSEN_MECHANISM).toString();
}

QByteArray SaslReply::response() const
{
    Q_D(const AuthenticationReply);
    return d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_RESPONSE).toByteArray();
}

SaslReply::State SaslReply::state() const
{
    Q_D(const AuthenticationReply);
    int state = d->data().value(ONLINE_ACCOUNTS_AUTH_KEY_STATE).toInt();
    switch (state) {
    case ONLINE_ACCOUNTS_AUTH_SASL_STATE_FINISHED: return Finished;
    case ONLINE_ACCOUNTS_AUTH_SASL_STATE_CONTINUE: return Continue;
    default:
        qWarning() << "Unknown SASL state" << state;
        return Finished;
    }
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
