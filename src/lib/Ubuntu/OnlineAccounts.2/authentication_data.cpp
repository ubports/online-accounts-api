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

QVariantMap replyToMap(const OnlineAccounts::PendingCall &call)
{
    OnlineAccounts::AuthenticationReply reply(call);
    return reply.data();
}

OnlineAccounts::AuthenticationData
authenticationDataFromMap(const QVariantMap &params,
                          OnlineAccounts::AuthenticationMethod method)
{
    OnlineAccounts::AuthenticationData data(method);
    QVariantMap cleanedParams(params);
    data.setInteractive(params.value("interactive", true).toBool());
    if (params["invalidateCachedReply"].toBool()) {
        data.invalidateCachedReply();
    }
    cleanedParams.remove("interactive");
    cleanedParams.remove("invalidateCachedReply");
    data.setParameters(cleanedParams);
    return data;
}
