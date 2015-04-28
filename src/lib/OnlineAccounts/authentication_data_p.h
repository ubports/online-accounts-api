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

#ifndef ONLINE_ACCOUNTS_AUTHENTICATION_DATA_P_H
#define ONLINE_ACCOUNTS_AUTHENTICATION_DATA_P_H

#include "authentication_data.h"

#include <QSharedData>
#include <QVariantMap>

namespace OnlineAccounts {

class AuthenticationDataPrivate: public QSharedData
{
public:
    inline AuthenticationDataPrivate(AuthenticationMethod method);
    virtual ~AuthenticationDataPrivate() {};

    friend class AuthenticationData;

    AuthenticationMethod m_method;
    bool m_interactive;
    bool m_invalidateCachedReply;
    QVariantMap m_parameters;
};

} // namespace

#endif // ONLINE_ACCOUNTS_AUTHENTICATION_DATA_P_H
