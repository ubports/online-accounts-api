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

#include "authentication_request.h"

#include <QDebug>
#include "authenticator.h"
#include "manager_adaptor.h"

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

class AuthenticationRequestPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AuthenticationRequest)

public:
    AuthenticationRequestPrivate(AuthenticationRequest *q);

private Q_SLOTS:
    void onFinished();

private:
    Authenticator m_authenticator;
    mutable AuthenticationRequest *q_ptr;
};

} // namespace

AuthenticationRequestPrivate::AuthenticationRequestPrivate(AuthenticationRequest *q):
    QObject(q),
    q_ptr(q)
{
    QObject::connect(&m_authenticator, SIGNAL(finished()),
                     this, SLOT(onFinished()));
}

void AuthenticationRequestPrivate::onFinished()
{
    Q_Q(AuthenticationRequest);
    if (m_authenticator.isError()) {
        q->setError(m_authenticator.errorName(),
                    m_authenticator.errorMessage());
    } else {
        q->setReply(QList<QVariant>() << m_authenticator.reply());
    }
}

AuthenticationRequest::AuthenticationRequest(const CallContext &context, QObject *parent):
    AsyncOperation(context, parent),
    d_ptr(new AuthenticationRequestPrivate(this))
{
}

AuthenticationRequest::~AuthenticationRequest()
{
    delete d_ptr;
}

void AuthenticationRequest::setInteractive(bool interactive)
{
    Q_D(AuthenticationRequest);
    d->m_authenticator.setInteractive(interactive);
}

void AuthenticationRequest::invalidateCache()
{
    Q_D(AuthenticationRequest);
    d->m_authenticator.invalidateCache();
}

void AuthenticationRequest::authenticate(const Accounts::AuthData &authData,
                                         const QVariantMap &parameters)
{
    Q_D(AuthenticationRequest);
    QVariantMap p(parameters);
    p.insert("requestorPid", context().clientPid());
    d->m_authenticator.authenticate(authData, p);
}

#include "authentication_request.moc"
