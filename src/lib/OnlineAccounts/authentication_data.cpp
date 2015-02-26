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

#include "authentication_data_p.h"

#include <QSharedData>
#include "dbus_constants.h"

using namespace OnlineAccounts;

AuthenticationDataPrivate::AuthenticationDataPrivate(AuthenticationMethod method):
    m_method(method),
    m_interactive(true),
    m_invalidateCachedReply(false)
{
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

/* OAuth 2.0 */

OAuth2Data::OAuth2Data():
    AuthenticationData(new AuthenticationDataPrivate(AuthenticationMethodOAuth2))
{
}

void OAuth2Data::setClientId(const QByteArray &id)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CLIENT_ID] = id;
}

QByteArray OAuth2Data::clientId() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CLIENT_ID].toByteArray();
}

void OAuth2Data::setClientSecret(const QByteArray &secret)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CLIENT_SECRET] = secret;
}

QByteArray OAuth2Data::clientSecret() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CLIENT_SECRET].toByteArray();
}

void OAuth2Data::setScopes(const QList<QByteArray> &scopes)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_SCOPES] = QVariant::fromValue(scopes);
}

QList<QByteArray> OAuth2Data::scopes() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_SCOPES].value<QList<QByteArray> >();
}

/* OAuth 1.0a */

OAuth1Data::OAuth1Data():
    AuthenticationData(new AuthenticationDataPrivate(AuthenticationMethodOAuth1))
{
}

void OAuth1Data::setConsumerKey(const QByteArray &key)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_KEY] = key;
}

QByteArray OAuth1Data::consumerKey() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_KEY].toByteArray();
}

void OAuth1Data::setConsumerSecret(const QByteArray &secret)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_SECRET] = secret;
}

QByteArray OAuth1Data::consumerSecret() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_SECRET].toByteArray();
}

/* Password */

PasswordData::PasswordData():
    AuthenticationData(new AuthenticationDataPrivate(AuthenticationMethodPassword))
{
}
