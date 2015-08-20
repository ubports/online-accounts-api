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

#include "authenticator.h"

#include <Accounts/AuthData>
#include <QDebug>
#include <SignOn/AuthSession>
#include <SignOn/Identity>
#include <SignOn/SessionData>
#include "dbus_constants.h"

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

static QVariantMap mergeMaps(const QVariantMap &map1,
                             const QVariantMap &map2)
{
    if (map1.isEmpty()) return map2;
    if (map2.isEmpty()) return map1;

    QVariantMap map = map1;
    //map2 values will overwrite map1 values for the same keys.
    QMapIterator<QString, QVariant> it(map2);
    while (it.hasNext()) {
        it.next();
        map.insert(it.key(), it.value());
    }
    return map;
}

class AuthenticatorPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Authenticator)

public:
    AuthenticatorPrivate(Authenticator *q);

    void authenticate(const Accounts::AuthData &authData,
                      const QVariantMap &parameters);
    static QString signonErrorName(int type);

private Q_SLOTS:
    void onAuthSessionResponse(const SignOn::SessionData &sessionData);
    void onAuthSessionError(const SignOn::Error &error);

private:
    SignOn::AuthSession *m_authSession;
    SignOn::Identity *m_identity;
    QVariantMap m_parameters;
    QVariantMap m_reply;
    QString m_errorName;
    QString m_errorMessage;
    Authenticator *q_ptr;
};

} // namespace

AuthenticatorPrivate::AuthenticatorPrivate(Authenticator *q):
    QObject(q),
    m_authSession(0),
    m_identity(0),
    q_ptr(q)
{
}

void AuthenticatorPrivate::authenticate(const Accounts::AuthData &authData,
                                        const QVariantMap &parameters)
{
    if (!m_identity) {
        m_identity =
            SignOn::Identity::existingIdentity(authData.credentialsId(), this);
    }
    if (!m_authSession) {
        m_authSession = m_identity->createSession(authData.method());
        QObject::connect(m_authSession,
                         SIGNAL(response(const SignOn::SessionData&)),
                         this,
                         SLOT(onAuthSessionResponse(const SignOn::SessionData&)));
        QObject::connect(m_authSession, SIGNAL(error(const SignOn::Error&)),
                         this, SLOT(onAuthSessionError(const SignOn::Error&)));
    }

    QVariantMap allSessionData =
        mergeMaps(authData.parameters(), mergeMaps(parameters, m_parameters));
    m_authSession->process(allSessionData, authData.mechanism());
}

void AuthenticatorPrivate::onAuthSessionResponse(const SignOn::SessionData &sessionData)
{
    Q_Q(Authenticator);
    m_reply = sessionData.toMap();
    Q_EMIT q->finished();
}

QString AuthenticatorPrivate::signonErrorName(int type)
{
#define HANDLE_CASE(name) \
    case SignOn::Error::name: return ONLINE_ACCOUNTS_ERROR_PREFIX #name

    switch (type) {
    HANDLE_CASE(MechanismNotAvailable);
    HANDLE_CASE(MissingData);
    HANDLE_CASE(InvalidCredentials);
    HANDLE_CASE(NotAuthorized);
    HANDLE_CASE(WrongState);
    HANDLE_CASE(OperationNotSupported);
    HANDLE_CASE(NoConnection);
    HANDLE_CASE(Network);
    HANDLE_CASE(Ssl);
    HANDLE_CASE(Runtime);
    HANDLE_CASE(SessionCanceled);
    HANDLE_CASE(TimedOut);
    HANDLE_CASE(OperationFailed);
    HANDLE_CASE(TOSNotAccepted);
    HANDLE_CASE(ForgotPassword);
    HANDLE_CASE(MethodOrMechanismNotAllowed);
    HANDLE_CASE(IncorrectDate);
    case SignOn::Error::UserInteraction:
        return ONLINE_ACCOUNTS_ERROR_INTERACTION_REQUIRED;
    default:
        qWarning() << "Unhandled signond error code:" << type;
        return ONLINE_ACCOUNTS_ERROR_PREFIX "UnknownError";
    };
}

void AuthenticatorPrivate::onAuthSessionError(const SignOn::Error &error)
{
    Q_Q(Authenticator);
    m_errorName = signonErrorName(error.type());
    m_errorMessage = error.message();
    Q_EMIT q->finished();
}

Authenticator::Authenticator(QObject *parent):
    QObject(parent),
    d_ptr(new AuthenticatorPrivate(this))
{
}

Authenticator::~Authenticator()
{
    delete d_ptr;
}

void Authenticator::setInteractive(bool interactive)
{
    Q_D(Authenticator);
    d->m_parameters["UiPolicy"] =
        interactive ? SignOn::DefaultPolicy : SignOn::NoUserInteractionPolicy;
}

void Authenticator::invalidateCache()
{
    Q_D(Authenticator);
    /* This works for OAuth 1.0 and 2.0; other authentication plugins should
     * implement a similar flag. */
    d->m_parameters["ForceTokenRefresh"] = true;
}

void Authenticator::authenticate(const Accounts::AuthData &authData,
                                 const QVariantMap &parameters)
{
    Q_D(Authenticator);
    d->authenticate(authData, parameters);
}

QVariantMap Authenticator::reply() const
{
    Q_D(const Authenticator);
    return d->m_reply;
}

QString Authenticator::errorName() const
{
    Q_D(const Authenticator);
    return d->m_errorName;
}

QString Authenticator::errorMessage() const
{
    Q_D(const Authenticator);
    return d->m_errorMessage;
}

#include "authenticator.moc"
