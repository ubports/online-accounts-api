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

#include "manager_p.h"

#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include "pending_call_p.h"

using namespace OnlineAccounts;

namespace OnlineAccounts {

class RequestAccessReplyPrivate
{
public:
    inline RequestAccessReplyPrivate(const PendingCall &call);
    virtual ~RequestAccessReplyPrivate() {}

    Account *account();

private:
    friend class RequestAccessReply;
    PendingCall m_pendingCall;
    AccountInfo m_accountInfo;
    Error m_error;
};

} // namespace

RequestAccessReplyPrivate::RequestAccessReplyPrivate(const PendingCall &call):
    m_pendingCall(call)
{
    const PendingCallPrivate *pCall = call.d.constData();

    if (!m_pendingCall.isFinished()) {
        m_pendingCall.waitForFinished();
    }

    if (Q_UNLIKELY(pCall->dbusCall().isError())) {
        /* Treat all errors as permission denied. */
        m_error = Error(Error::PermissionDenied,
                        pCall->dbusCall().error().message());
        return;
    }

    PendingCallPrivate::InvokedMethod invokedMethod = pCall->invokedMethod();
    if (Q_UNLIKELY(invokedMethod != PendingCallPrivate::RequestAccess)) {
        qFatal("Wrong invoked method %d", invokedMethod);
    }
    QDBusPendingReply<AccountInfo,QVariantMap> reply(pCall->dbusCall());
    m_accountInfo = reply.argumentAt<0>();
}

Account *RequestAccessReplyPrivate::account()
{
    ManagerPrivate *managerPriv = m_pendingCall.d->manager()->d_ptr;
    return managerPriv->ensureAccount(m_accountInfo);
}

RequestAccessReply::RequestAccessReply(const PendingCall &call):
    d_ptr(new RequestAccessReplyPrivate(call))
{
}

RequestAccessReply::~RequestAccessReply()
{
    delete d_ptr;
}

Error RequestAccessReply::error() const
{
    Q_D(const RequestAccessReply);
    return d->m_error;
}

Account *RequestAccessReply::account()
{
    Q_D(RequestAccessReply);
    return d->account();
}
