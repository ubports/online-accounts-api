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

#include <QSharedData>

using namespace OnlineAccounts;

#define AD_D(Class) Class##Private * const d = priv<Class##Private>()

namespace OnlineAccounts {

class AuthenticationDataPrivate: public QSharedData
{
public:
    inline AuthenticationDataPrivate();
    virtual ~AuthenticationDataPrivate() {};

    virtual AuthenticationDataPrivate *clone() = 0;

    friend class AuthenticationData;

    AuthenticationMethod m_method;
    bool m_interactive;
    bool m_invalidateCachedReply;
};

} // namespace

AuthenticationDataPrivate::AuthenticationDataPrivate():
    m_method(AuthenticationMethodUnknown),
    m_interactive(true),
    m_invalidateCachedReply(false)
{
}

template<> AuthenticationDataPrivate *
QSharedDataPointer<AuthenticationDataPrivate>::clone()
{
    return d->clone();
}

AuthenticationData::AuthenticationData(AuthenticationDataPrivate *priv):
    d(priv)
{
}

AuthenticationData::AuthenticationData(const AuthenticationData &other):
    d(other.d)
{
}

AuthenticationData::~AuthenticationData()
{
}

AuthenticationMethod AuthenticationData::method() const
{
    return d->m_method;
}

void AuthenticationData::setInteractive(bool interactive)
{
    d->m_interactive = interactive;
}

bool AuthenticationData::interactive() const
{
    return d->m_interactive;
}

void AuthenticationData::invalidateCachedReply()
{
    d->m_invalidateCachedReply = true;
}

namespace OnlineAccounts {

class OAuth2DataPrivate: public AuthenticationDataPrivate
{
    friend class OAuth2Data;

public:
    OAuth2DataPrivate();

    AuthenticationDataPrivate *clone() Q_DECL_OVERRIDE;

private:
    QByteArray m_clientId;
    QByteArray m_clientSecret;
    QList<QByteArray> m_scopes;
};

} // namespace

OAuth2DataPrivate::OAuth2DataPrivate()
{
}

AuthenticationDataPrivate *OAuth2DataPrivate::clone()
{
    return new OAuth2DataPrivate(*this);
}

OAuth2Data::OAuth2Data():
    AuthenticationData(new OAuth2DataPrivate)
{
}

void OAuth2Data::setClientId(const QByteArray &id)
{
    AD_D(OAuth2Data);
    d->m_clientId = id;
}

QByteArray OAuth2Data::clientId() const
{
    AD_D(const OAuth2Data);
    return d->m_clientId;
}
