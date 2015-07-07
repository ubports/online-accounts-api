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

#ifndef ONLINE_ACCOUNTS_DAEMON_MANAGER_ADAPTOR_H
#define ONLINE_ACCOUNTS_DAEMON_MANAGER_ADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QString>
#include <QVariantMap>
#include "manager.h"

namespace OnlineAccountsDaemon {

class CallContext {
public:
    explicit CallContext(QDBusContext *dbusContext);
    CallContext(const CallContext &other);
    virtual ~CallContext();

    void setDelayedReply(bool delayed);
    void sendReply(const QList<QVariant> &args) const;
    void sendError(const QString &name, const QString &message) const;

    QString securityContext() const;
    QString clientName() const;

private:
    QDBusConnection m_connection;
    QDBusMessage m_message;
};

class CallContextCounter: public QObject
{
    Q_OBJECT

public:
    static CallContextCounter *instance();

    int activeContexts() const { return m_contexts; }

Q_SIGNALS:
    void activeContextsChanged();

protected:
    void addContext(const CallContext &context);
    void removeContext(const CallContext &context);

private:
    friend class CallContext;
    CallContextCounter();

    int m_contexts;
};

class ManagerAdaptorPrivate;
class ManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.ubuntu.OnlineAccounts.Manager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.ubuntu.OnlineAccounts.Manager\">\n"
"    <method name=\"GetAccounts\">\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"filters\"/>\n"
"      <arg direction=\"out\" type=\"a(ua{sv})\" name=\"accounts\"/>\n"
"    </method>\n"
"    <method name=\"Authenticate\">\n"
"      <arg direction=\"in\" type=\"u\" name=\"accountId\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"serviceId\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"interactive\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"invalidate\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"parameters\"/>\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"credentials\"/>\n"
"    </method>\n"
"    <method name=\"RequestAccess\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"serviceId\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"parameters\"/>\n"
"      <arg direction=\"out\" type=\"(ua{sv})\" name=\"account\"/>\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"credentials\"/>\n"
"    </method>\n"
"    <signal name=\"AccountChanged\">\n"
"      <arg type=\"s\" name=\"serviceId\"/>\n"
"      <arg type=\"(ua{sv})\" name=\"account\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")

public:
    explicit ManagerAdaptor(Manager *parent);
    ~ManagerAdaptor();

    inline Manager *parent() const
    { return static_cast<Manager *>(QObject::parent()); }

    inline QDBusContext *dbusContext() const
    { return static_cast<QDBusContext *>(parent()); }

    void notifyAccountChange(const AccountInfo &info, uint changeType);

public Q_SLOTS:
    QVariantMap Authenticate(uint accountId, const QString &serviceId,
                             bool interactive, bool invalidate,
                             const QVariantMap &parameters);
    QList<AccountInfo> GetAccounts(const QVariantMap &filters);
    AccountInfo RequestAccess(const QString &serviceId,
                              const QVariantMap &parameters,
                              QVariantMap &credentials);

Q_SIGNALS:
    void AccountChanged(const QString &serviceId, AccountInfo account);

private:
    Q_DECLARE_PRIVATE(ManagerAdaptor)
    ManagerAdaptorPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_MANAGER_ADAPTOR_H
