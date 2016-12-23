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

#ifndef ONLINE_ACCOUNTS_PENDING_CALL_H
#define ONLINE_ACCOUNTS_PENDING_CALL_H

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QScopedPointer>

#include "global.h"

namespace OnlineAccounts {

class AuthenticationReplyPrivate;
class ManagerPrivate;

class PendingCallPrivate;
class ONLINE_ACCOUNTS_EXPORT PendingCall
{
public:
    PendingCall(const PendingCall &other);
    ~PendingCall();
    PendingCall &operator=(const PendingCall &other);

    bool isFinished() const;
    void waitForFinished();

protected:
    friend class PendingCallPrivate;
    friend class PendingCallWatcher;
    friend class AuthenticationReplyPrivate;
    friend class RequestAccessReplyPrivate;
    friend class ManagerPrivate;
    QExplicitlySharedDataPointer<PendingCallPrivate> d;

private:
    PendingCall(PendingCallPrivate *priv);
};

class PendingCallWatcherPrivate;
class ONLINE_ACCOUNTS_EXPORT PendingCallWatcher:
    public QObject, public PendingCall
{
    Q_OBJECT

public:
    PendingCallWatcher(const PendingCall &call, QObject *parent = 0);
    ~PendingCallWatcher();

Q_SIGNALS:
    void finished();

private:
    Q_DECLARE_PRIVATE(PendingCallWatcher)
    QScopedPointer<PendingCallWatcherPrivate> d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_PENDING_CALL_H
