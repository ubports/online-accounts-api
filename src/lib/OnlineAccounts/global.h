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

#ifndef ONLINE_ACCOUNTS_GLOBAL_H
#define ONLINE_ACCOUNTS_GLOBAL_H

#include <QtGlobal>

#if defined(BUILDING_ONLINE_ACCOUNTS)
#  define ONLINE_ACCOUNTS_EXPORT Q_DECL_EXPORT
#else
#  define ONLINE_ACCOUNTS_EXPORT Q_DECL_IMPORT
#endif

#if defined(BUILDING_ONLINE_ACCOUNTS)
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(DBG_ONLINE_ACCOUNTS)
#endif

namespace OnlineAccounts {

typedef uint AccountId;

enum AuthenticationMethod {
    AuthenticationMethodUnknown = 0,
    AuthenticationMethodOAuth1,
    AuthenticationMethodOAuth2,
    AuthenticationMethodPassword,
    AuthenticationMethodSasl,
};

} // namespace

#endif // ONLINE_ACCOUNTS_GLOBAL_H
