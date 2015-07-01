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

#include "daemon/dbus_constants.h"
#include "daemon_interface.h"
#include "fake_online_accounts_service.h"
#include "fake_signond.h"
#include <QDBusConnection>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <libqtdbusmock/DBusMock.h>

class FunctionalTests: public QObject
{
    Q_OBJECT

public:
    FunctionalTests();

private Q_SLOTS:
    void testManagerRequestAccess_data();
    void testManagerRequestAccess();

private:
    QtDBusTest::DBusTestRunner m_dbus;
    QtDBusMock::DBusMock m_mock;
    FakeOnlineAccountsService m_onlineAccounts;
    FakeSignond m_signond;
};

FunctionalTests::FunctionalTests():
    QObject(),
    m_dbus(TEST_DBUS_CONFIG_FILE),
    m_mock(m_dbus),
    m_onlineAccounts(&m_mock),
    m_signond(&m_mock)
{
    m_dbus.startServices();
}

void FunctionalTests::testManagerRequestAccess_data()
{
    QTest::addColumn<QString>("serviceId");
    QTest::addColumn<QVariantMap>("authParams");
    QTest::addColumn<QVariantMap>("accessReply");
    QTest::addColumn<int>("expectedAccountId");
    QTest::addColumn<QVariantMap>("expectedAccountInfo");
    QTest::addColumn<QVariantMap>("expectedCredentials");
    QTest::addColumn<QString>("errorName");

    QVariantMap authParams;
    QVariantMap accessReply;
    QVariantMap accountInfo;
    QVariantMap credentials;
    QTest::newRow("access denied") <<
        "coolmail" <<
        authParams <<
        accessReply <<
        0 <<
        accountInfo <<
        credentials <<
        ONLINE_ACCOUNTS_ERROR_PERMISSION_DENIED;
}

void FunctionalTests::testManagerRequestAccess()
{
    QFETCH(QString, serviceId);
    QFETCH(QVariantMap, authParams);
    QFETCH(QVariantMap, accessReply);
    QFETCH(int, expectedAccountId);
    QFETCH(QVariantMap, expectedAccountInfo);
    QFETCH(QVariantMap, expectedCredentials);
    QFETCH(QString, errorName);

    m_onlineAccounts.setRequestAccessReply(accessReply);

    DaemonInterface *daemon = new DaemonInterface;

    QDBusPendingReply<AccountInfo,QVariantMap> reply =
        daemon->requestAccess(serviceId, authParams);
    reply.waitForFinished();

    if (errorName.isEmpty()) {
        QVERIFY(!reply.isError());
        AccountInfo accountInfo = reply.argumentAt<0>();
        QVariantMap credentials = reply.argumentAt<1>();
        QCOMPARE(int(accountInfo.id()), expectedAccountId);
        QCOMPARE(accountInfo.data(), expectedAccountInfo);
        QCOMPARE(credentials, expectedCredentials);
    } else {
        QVERIFY(reply.isError());
        QCOMPARE(reply.error().name(), errorName);
    }

    delete daemon;
}

QTEST_MAIN(FunctionalTests)
#include "functional_tests.moc"
