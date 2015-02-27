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

#include "pending_call_p.h"

using namespace OnlineAccounts;

PendingCallPrivate::PendingCallPrivate(Manager *manager,
                                       const QDBusPendingCall &call,
                                       InvokedMethod method,
                                       AuthenticationMethod authMethod):
    m_manager(manager),
    m_call(call),
    m_invokedMethod(method),
    m_authenticationMethod(authMethod)
{
}

PendingCall::PendingCall(PendingCallPrivate *priv):
    d(priv)
{
}

PendingCall::PendingCall(const PendingCall &other):
    d(other.d)
{
}

PendingCall::~PendingCall()
{
}

PendingCall &PendingCall::operator=(const PendingCall &other)
{
    d = other.d;
    return *this;
}

bool PendingCall::isFinished() const
{
    return d->m_call.isFinished();
}

void PendingCall::waitForFinished()
{
    d->m_call.waitForFinished();
}
