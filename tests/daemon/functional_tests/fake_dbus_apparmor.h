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

#ifndef OAD_FAKE_DBUS_APPARMOR_H
#define OAD_FAKE_DBUS_APPARMOR_H

#include <QVariantMap>
#include <libqtdbusmock/DBusMock.h>

class FakeDBusApparmor
{
public:
    FakeDBusApparmor(QtDBusMock::DBusMock *mock): m_mock(mock) {
        m_mock->registerTemplate("mocked.org.freedesktop.dbus",
                                 DBUS_APPARMOR_MOCK_TEMPLATE,
                                 QDBusConnection::SessionBus);
    }

    void setCredentials(const QString &service, const QVariantMap &credentials) {
        mocked().call("SetCredentials", service, credentials);
    }

private:
    OrgFreedesktopDBusMockInterface &mocked() {
        return m_mock->mockInterface("mocked.org.freedesktop.dbus",
                                     "/org/freedesktop/DBus",
                                     "org.freedesktop.DBus",
                                     QDBusConnection::SessionBus);
    }

private:
    QtDBusMock::DBusMock *m_mock;
};

#endif // OAD_FAKE_DBUS_APPARMOR_H
