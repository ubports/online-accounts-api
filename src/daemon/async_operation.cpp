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

#include "async_operation.h"

#include <QDebug>
#include "manager_adaptor.h"

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

class AsyncOperationPrivate
{
    Q_DECLARE_PUBLIC(AsyncOperation)

public:
    AsyncOperationPrivate(AsyncOperation *q, const CallContext &context);

private:
    CallContext m_context;
    mutable AsyncOperation *q_ptr;
};

} // namespace

AsyncOperationPrivate::AsyncOperationPrivate(AsyncOperation *q,
                                             const CallContext &context):
    m_context(context),
    q_ptr(q)
{
}

AsyncOperation::AsyncOperation(const CallContext &context, QObject *parent):
    QObject(parent),
    d_ptr(new AsyncOperationPrivate(this, context))
{
}

AsyncOperation::~AsyncOperation()
{
    delete d_ptr;
}

const CallContext &AsyncOperation::context() const
{
    Q_D(const AsyncOperation);
    return d->m_context;
}

void AsyncOperation::setReply(const QList<QVariant> &reply)
{
    Q_D(AsyncOperation);
    d->m_context.sendReply(reply);
    this->deleteLater();
}

void AsyncOperation::setError(const QString &name, const QString &message)
{
    Q_D(AsyncOperation);
    d->m_context.sendError(name, message);
    this->deleteLater();
}
