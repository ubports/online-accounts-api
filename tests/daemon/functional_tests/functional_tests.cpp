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
#include <Accounts/Account>
#include <Accounts/Manager>
#include <Accounts/Service>
#include <QDBusConnection>
#include <QDir>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <libqtdbusmock/DBusMock.h>

class FunctionalTests: public QObject
{
    Q_OBJECT

    struct EnvSetup {
        EnvSetup();
    };

public:
    FunctionalTests();

private Q_SLOTS:
    void testGetAccountsFiltering_data();
    void testGetAccountsFiltering();
    void testRequestAccess_data();
    void testRequestAccess();

private:
    void clearDb();

private:
    EnvSetup m_env;
    QtDBusTest::DBusTestRunner m_dbus;
    QtDBusMock::DBusMock m_mock;
    FakeOnlineAccountsService m_onlineAccounts;
    FakeSignond m_signond;
};

FunctionalTests::EnvSetup::EnvSetup() {
    qputenv("ACCOUNTS", "/tmp/");
    qputenv("AG_APPLICATIONS", TEST_DATA_DIR);
    qputenv("AG_SERVICES", TEST_DATA_DIR);
    qputenv("AG_SERVICE_TYPES", TEST_DATA_DIR);
    qputenv("AG_PROVIDERS", TEST_DATA_DIR);
    qputenv("XDG_DATA_HOME", TEST_DATA_DIR);
}

FunctionalTests::FunctionalTests():
    QObject(),
    m_dbus(TEST_DBUS_CONFIG_FILE),
    m_mock(m_dbus),
    m_onlineAccounts(&m_mock),
    m_signond(&m_mock)
{
    clearDb();

    m_dbus.startServices();
}

void FunctionalTests::clearDb()
{
    QDir dbroot(QString::fromLatin1(qgetenv("ACCOUNTS")));
    dbroot.remove("accounts.db");
}

void FunctionalTests::testGetAccountsFiltering_data()
{
    QTest::addColumn<QVariantMap>("filters");
    QTest::addColumn<QList<int> >("expectedAccountIds");

    QVariantMap filters;
    QTest::newRow("empty filters") <<
        filters <<
        (QList<int>() << 1 << 2 << 3);
}

void FunctionalTests::testGetAccountsFiltering()
{
    QFETCH(QVariantMap, filters);
    QFETCH(QList<int>, expectedAccountIds);

    /* Populate the accounts DB */
    Accounts::Manager *manager = new Accounts::Manager(this);
    Accounts::Service coolMail = manager->service("coolmail");
    Accounts::Service coolShare = manager->service("coolshare");
    Accounts::Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);
    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount");
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->syncAndBlock();
    int firstAccountId = account1->id() - 1;

    Accounts::Account *account2 = manager->createAccount("cool");
    QVERIFY(account2 != 0);
    account2->setEnabled(true);
    account2->setDisplayName("CoolAccount");
    account2->selectService(coolMail);
    account2->setEnabled(true);
    account2->syncAndBlock();

    Accounts::Account *account3 = manager->createAccount("cool");
    QVERIFY(account3 != 0);
    account3->setEnabled(true);
    account3->setDisplayName("CoolAccount");
    account3->selectService(coolMail);
    account3->setEnabled(true);
    account3->syncAndBlock();

    delete manager;

    DaemonInterface *daemon = new DaemonInterface;

    QDBusPendingReply<QList<AccountInfo> > reply =
        daemon->getAccounts(filters);
    reply.waitForFinished();

    QVERIFY(!reply.isError());
    QList<AccountInfo> accountInfos = reply.argumentAt<0>();
    QList<int> accountIds;
    Q_FOREACH(const AccountInfo &info, accountInfos) {
        accountIds.append(info.id() + firstAccountId);
    }
    QCOMPARE(accountIds.toSet(), expectedAccountIds.toSet());

    delete daemon;

}

void FunctionalTests::testRequestAccess_data()
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

void FunctionalTests::testRequestAccess()
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
