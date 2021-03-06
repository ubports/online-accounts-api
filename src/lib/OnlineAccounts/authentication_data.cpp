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

#include "authentication_data_p.h"

#include <QDBusMetaType>
#include "OnlineAccountsDaemon/dbus_constants.h"

using namespace OnlineAccounts;

AuthenticationDataPrivate::AuthenticationDataPrivate(AuthenticationMethod method):
    m_method(method),
    m_interactive(true),
    m_invalidateCachedReply(false)
{
}

AuthenticationData::AuthenticationData(AuthenticationMethod method):
    d(new AuthenticationDataPrivate(method))
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

bool AuthenticationData::mustInvalidateCachedReply() const
{
    return d->m_invalidateCachedReply;
}

void AuthenticationData::setParameters(const QVariantMap &parameters)
{
    d->m_parameters = parameters;
}

QVariantMap AuthenticationData::parameters() const
{
    return d->m_parameters;
}

/* OAuth 2.0 */

OAuth2Data::OAuth2Data():
    AuthenticationData(new AuthenticationDataPrivate(AuthenticationMethodOAuth2))
{
    qDBusRegisterMetaType<QList<QByteArray>>();
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

/* SASL */

SaslData::SaslData():
    AuthenticationData(new AuthenticationDataPrivate(AuthenticationMethodSasl))
{
}

void SaslData::setService(const QString &service)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_SERVICE] = service;
}

QString SaslData::service() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_SERVICE].toString();
}

void SaslData::setMechanismList(const QByteArray &mechanisms)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_MECHANISMS] = mechanisms;
}

QByteArray SaslData::mechanismList() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_MECHANISMS].toByteArray();
}

void SaslData::setServerFqdn(const QString &fqdn)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_FQDN] = fqdn;
}

QString SaslData::serverFqdn() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_FQDN].toString();
}

void SaslData::setLocalIp(const QString &localIp)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_LOCAL_IP] = localIp;
}

QString SaslData::localIp() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_LOCAL_IP].toString();
}

void SaslData::setRemoteIp(const QString &remoteIp)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_REMOTE_IP] = remoteIp;
}

QString SaslData::remoteIp() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_REMOTE_IP].toString();
}

void SaslData::setChallenge(const QByteArray &challenge)
{
    d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CHALLENGE] = challenge;
}

QByteArray SaslData::challenge() const
{
    return d->m_parameters[ONLINE_ACCOUNTS_AUTH_KEY_CHALLENGE].toByteArray();
}

/* Password */

PasswordData::PasswordData():
    AuthenticationData(new AuthenticationDataPrivate(AuthenticationMethodPassword))
{
}
