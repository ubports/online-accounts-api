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

#include "authentication_data.h"

#include "OnlineAccounts/PendingCall"
#include <QDebug>

static void commonParamsFromMap(OnlineAccounts::AuthenticationData &data,
                                const QVariantMap &params)
{
    data.setInteractive(params.value("interactive", true).toBool());
    if (params["invalidateCachedReply"].toBool()) {
        data.invalidateCachedReply();
    }
}

QVariantMap replyToMap(const OnlineAccounts::PendingCall &call,
                       OnlineAccounts::AuthenticationMethod method)
{
    QVariantMap map;
    switch (method) {
    case OnlineAccounts::AuthenticationMethodOAuth1:
        {
            OnlineAccounts::OAuth1Reply reply(call);
            map["ConsumerKey"] = QString::fromUtf8(reply.consumerKey());
            map["ConsumerSecret"] = QString::fromUtf8(reply.consumerSecret());
            map["Token"] = QString::fromUtf8(reply.token());
            map["TokenSecret"] = QString::fromUtf8(reply.tokenSecret());
            map["SignatureMethod"] = QString::fromUtf8(reply.signatureMethod());
            return map;
        }
    case OnlineAccounts::AuthenticationMethodOAuth2:
        {
            OnlineAccounts::OAuth2Reply reply(call);
            map["AccessToken"] = QString::fromUtf8(reply.accessToken());
            map["ExpiresIn"] = reply.expiresIn();
            QStringList grantedScopes;
            Q_FOREACH(const QByteArray &s, reply.grantedScopes()) {
                grantedScopes.append(s);
            }
            map["GrantedScopes"] = grantedScopes;
            return map;
        }
    case OnlineAccounts::AuthenticationMethodPassword:
        {
            OnlineAccounts::PasswordReply reply(call);
            map["Username"] = reply.username();
            map["Password"] = reply.password();
            return map;
        }
    default:
        return map;
    }
}

OnlineAccounts::AuthenticationData
authenticationDataFromMap(const QVariantMap &params,
                          OnlineAccounts::AuthenticationMethod method)
{
    switch (method) {
    case OnlineAccounts::AuthenticationMethodOAuth1:
        {
            OnlineAccounts::OAuth1Data data;
            if (params.contains("ConsumerKey")) {
                data.setConsumerKey(params["ConsumerKey"].toByteArray());
                data.setConsumerSecret(params["ConsumerSecret"].toByteArray());
            }
            commonParamsFromMap(data, params);
            return data;
        }
    case OnlineAccounts::AuthenticationMethodOAuth2:
        {
            OnlineAccounts::OAuth2Data data;
            if (params.contains("ClientId")) {
                data.setClientId(params["ClientId"].toByteArray());
                data.setClientSecret(params["ClientSecret"].toByteArray());
            }
            if (params.contains("Scopes")) {
                QList<QByteArray> scopes;
                Q_FOREACH(const QString &s, params["Scopes"].toStringList()) {
                    scopes.append(s.toUtf8());
                }
                data.setScopes(scopes);
            }
            commonParamsFromMap(data, params);
            return data;
        }
    case OnlineAccounts::AuthenticationMethodPassword:
    default:
        {
            OnlineAccounts::PasswordData data;
            commonParamsFromMap(data, params);
            return data;
        }
    }
}
