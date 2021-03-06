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

#include "manager_adaptor.h"

#include <QDBusMetaType>
#include <QDebug>
#include "client_registry.h"

using namespace OnlineAccountsDaemon;

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info)
{
    argument.beginStructure();
    argument << info.accountId << info.details;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                AccountInfo &info)
{
    argument.beginStructure();
    argument >> info.accountId >> info.details;
    argument.endStructure();
    return argument;
}

CallContext::CallContext(QDBusContext *dbusContext):
    m_connection(dbusContext->connection()),
    m_message(dbusContext->message())
{
    CallContextCounter::instance()->addContext(*this);
}

CallContext::CallContext(const CallContext &other):
    m_connection(other.m_connection),
    m_message(other.m_message)
{
    CallContextCounter::instance()->addContext(*this);
}

CallContext::~CallContext()
{
    CallContextCounter::instance()->removeContext(*this);
}

void CallContext::setDelayedReply(bool delayed)
{
    m_message.setDelayedReply(delayed);
}

void CallContext::sendReply(const QList<QVariant> &args) const
{
    m_connection.send(m_message.createReply(args));
}

void CallContext::sendError(const QString &name, const QString &message) const
{
    m_connection.send(m_message.createErrorReply(name, message));
}

QString CallContext::securityContext() const
{
    ClientRegistry *clientRegistry = ClientRegistry::instance();
    QString client = clientRegistry->registerClient(m_connection, m_message);
    return clientRegistry->clientSecurityContext(client);
}

pid_t CallContext::clientPid() const
{
    ClientRegistry *clientRegistry = ClientRegistry::instance();
    return clientRegistry->clientPid(m_message.service());
}

QString CallContext::clientName() const
{
    return m_message.service();
}

static CallContextCounter *m_callContextCounterInstance = 0;

CallContextCounter::CallContextCounter():
    QObject(),
    m_contexts(0)
{
}

CallContextCounter *CallContextCounter::instance()
{
    if (!m_callContextCounterInstance) {
        m_callContextCounterInstance = new CallContextCounter;
    }
    return m_callContextCounterInstance;
}

void CallContextCounter::addContext(const CallContext &)
{
    m_contexts++;
    Q_EMIT activeContextsChanged();
}

void CallContextCounter::removeContext(const CallContext &)
{
    m_contexts--;
    Q_EMIT activeContextsChanged();
}

namespace OnlineAccountsDaemon {

class ManagerAdaptorPrivate {
public:
    ManagerAdaptorPrivate();

private:
    friend class ManagerAdaptor;
};

} // namespace

ManagerAdaptorPrivate::ManagerAdaptorPrivate()
{
}

ManagerAdaptor::ManagerAdaptor(Manager *parent):
    QDBusAbstractAdaptor(parent),
    d_ptr(new ManagerAdaptorPrivate)
{
    qRegisterMetaType<AccountInfo>("AccountInfo");
    qRegisterMetaType<QList<AccountInfo> >("QList<AccountInfo>");
    qDBusRegisterMetaType<AccountInfo>();
    qDBusRegisterMetaType<QList<AccountInfo>>();
    qDBusRegisterMetaType<QList<QVariantMap>>();

    setAutoRelaySignals(false);
}

ManagerAdaptor::~ManagerAdaptor()
{
    delete d_ptr;
}

void ManagerAdaptor::notifyAccountChange(const AccountInfo &info,
                                         uint changeType)
{
    AccountInfo copy(info);
    copy.details[ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE] = changeType;
    Q_EMIT AccountChanged(copy.serviceId(), copy);
}

QVariantMap ManagerAdaptor::Authenticate(uint accountId,
                                         const QString &serviceId,
                                         bool interactive, bool invalidate,
                                         const QVariantMap &parameters)
{
    parent()->authenticate(accountId, serviceId,
                           interactive, invalidate, parameters,
                           CallContext(dbusContext()));
    return QVariantMap();
}

void ManagerAdaptor::GetAccounts(const QVariantMap &filters,
                                 QList<AccountInfo> &accounts,
                                 QList<QVariantMap> &services)
{
    accounts = parent()->getAccounts(filters,
                                     CallContext(dbusContext()),
                                     services);
}

AccountInfo ManagerAdaptor::RequestAccess(const QString &serviceId,
                                          const QVariantMap &parameters,
                                          QVariantMap &credentials)
{
    parent()->requestAccess(serviceId, parameters,
                            CallContext(dbusContext()));
    credentials = QVariantMap();
    return AccountInfo();
}

#include "manager_adaptor.moc"
