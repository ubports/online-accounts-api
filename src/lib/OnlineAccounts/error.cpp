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

#include "error_p.h"

#include <QDBusError>
#include "OnlineAccountsDaemon/dbus_constants.h"

namespace OnlineAccounts {

Error errorFromDBus(const QDBusError &dbusError)
{
    Error::Code code = Error::PermissionDenied;
    QString name = dbusError.name();
    if (name == ONLINE_ACCOUNTS_ERROR_NO_ACCOUNT) {
        code = Error::NoAccount;
    } else if (name == ONLINE_ACCOUNTS_ERROR_USER_CANCELED) {
        code = Error::UserCanceled;
    } else if (name == ONLINE_ACCOUNTS_ERROR_INTERACTION_REQUIRED) {
        code = Error::InteractionRequired;
    }
    return Error(code, dbusError.message());
}

} // namespace
