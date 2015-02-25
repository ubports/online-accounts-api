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

#ifndef ONLINE_ACCOUNTS_PENDING_CALL_P_H
#define ONLINE_ACCOUNTS_PENDING_CALL_P_H

#include <QDBusPendingCall>

namespace OnlineAccounts {

class PendingCallPrivate
{
public:
    enum InvokedMethod {
        Authenticate,
        RequestAccess,
    };

    inline PendingCallPrivate();
    ~PendingCallPrivate() {};

    QDBusPendingCall dbusCall() const { return m_call; }
    InvokedMethod invokedMethod() const { return m_invokedMethod; }

    AuthenticationMethod authenticationMethod() const {
        return m_authenticationMethod;
    }

private:
    QDBusPendingCall m_call;
    InvokedMethod m_invokedMethod;
    AuthenticationMethod m_authenticationMethod;
};

} // namespace

#endif // ONLINE_ACCOUNTS_PENDING_CALL_P_H
