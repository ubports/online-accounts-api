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

#include "access_request.h"

#include <Accounts/AuthData>
#include <online-accounts-client/Setup>
#include <QDBusArgument>
#include <QDebug>
#include "account_info.h"
#include "authenticator.h"

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

class AccessRequestPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AccessRequest)

public:
    AccessRequestPrivate(AccessRequest *q);

    void requestAccess(const QString &applicationId,
                       const QString &serviceId,
                       const QVariantMap &parameters,
                       pid_t clientPid);

private Q_SLOTS:
    void onSetupFinished(QVariantMap reply);
    void onAuthenticationFinished();

private:
    OnlineAccountsClient::Setup m_setup;
    Authenticator m_authenticator;
    QVariantMap m_parameters;
    AccountInfo m_accountInfo;
    AccessRequest *q_ptr;
};

} // namespace

AccessRequestPrivate::AccessRequestPrivate(AccessRequest *q):
    QObject(q),
    q_ptr(q)
{
    QObject::connect(&m_setup, SIGNAL(finished(QVariantMap)),
                     this, SLOT(onSetupFinished(QVariantMap)));
    QObject::connect(&m_authenticator, SIGNAL(finished()),
                     this, SLOT(onAuthenticationFinished()));
}

void AccessRequestPrivate::requestAccess(const QString &applicationId,
                                         const QString &serviceId,
                                         const QVariantMap &parameters,
                                         pid_t clientPid)
{
    m_parameters = parameters;

    m_setup.setApplicationId(applicationId);
    m_setup.setServiceId(serviceId);
    m_setup.setClientPid(clientPid);
    m_setup.exec();
}

void AccessRequestPrivate::onSetupFinished(QVariantMap reply)
{
    Q_Q(AccessRequest);
    uint accountId = reply[QStringLiteral("accountId")].toUInt();
    if (accountId == 0) {
        q->setError(ONLINE_ACCOUNTS_ERROR_PERMISSION_DENIED,
                    "Authorization was not granted");
    } else {
        Q_EMIT q->loadRequest(accountId, m_setup.serviceId());
    }
}

void AccessRequestPrivate::onAuthenticationFinished()
{
    Q_Q(AccessRequest);

    /* We don't check for authentication errors here. Even if an error
     * occurred, we still have to return the account info to the client.
     * So, if an error occurred, we'll just return empty authentication data to
     * the client.
     */
    QList<QVariant> args;
    QDBusArgument infoArg;
    infoArg << m_accountInfo;
    args << QVariant::fromValue(infoArg);
    args << m_authenticator.reply();
    q->setReply(args);
}

AccessRequest::AccessRequest(const CallContext &context, QObject *parent):
    AsyncOperation(context, parent),
    d_ptr(new AccessRequestPrivate(this))
{
}

AccessRequest::~AccessRequest()
{
    delete d_ptr;
}

void AccessRequest::requestAccess(const QString &applicationId,
                                  const QString &serviceId,
                                  const QVariantMap &parameters,
                                  pid_t clientPid)
{
    Q_D(AccessRequest);
    d->requestAccess(applicationId, serviceId, parameters, clientPid);
}

void AccessRequest::setAccountInfo(const AccountInfo &accountInfo,
                                   const Accounts::AuthData &authData)
{
    Q_D(AccessRequest);
    d->m_accountInfo = accountInfo;
    d->m_authenticator.authenticate(authData, d->m_parameters);
}

#include "access_request.moc"
