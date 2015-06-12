/*
 * This file is part of OnlineAccountsDaemon
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

#ifndef ONLINE_ACCOUNTS_DAEMON_ASYNC_OPERATION_H
#define ONLINE_ACCOUNTS_DAEMON_ASYNC_OPERATION_H

#include <QObject>
#include <QList>
#include <QString>
#include <QVariant>

namespace OnlineAccountsDaemon {

class CallContext;

class AsyncOperationPrivate;
class AsyncOperation: public QObject
{
    Q_OBJECT

public:
    explicit AsyncOperation(const CallContext &context,
                            QObject *parent = 0);
    ~AsyncOperation();

protected:
    void setReply(const QList<QVariant> &reply);
    void setError(const QString &name, const QString &message);

private:
    Q_DECLARE_PRIVATE(AsyncOperation)
    AsyncOperationPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_ASYNC_OPERATION_H
