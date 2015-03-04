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

#include "OnlineAccounts/Account"
#include "OnlineAccounts/Manager"
#include "OnlineAccounts/OAuth2Data"
#include "OnlineAccounts/account_info.h"
#include "OnlineAccounts/dbus_constants.h"
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

    OrgFreedesktopDBusMockInterface &mocked() {
        return m_mock.mockInterface(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                                    ONLINE_ACCOUNTS_MANAGER_PATH,
                                    ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                                    QDBusConnection::SessionBus);
    }

    void addMockedMethod(const QString &name,
                                        const QString &in_sig,
                                        const QString &out_sig,
                                        const QString &code)
    {
        return mocked().AddMethod(ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                                  name, in_sig, out_sig, code).waitForFinished();
    }

    void emitAccountChanged(const QString &service,
                            uint accountId,
                            const QVariantMap &changes)
    {
        QVariantList args;
        args << service;
        OnlineAccounts::AccountInfo accountInfo(accountId, changes);
        args << QVariant::fromValue(accountInfo);
        mocked().EmitSignal(ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                            "AccountChanged", "s(ua{sv})",
                            args);
    }

private Q_SLOTS:
    void testManagerReady_data();
    void testManagerReady();
    void testManagerAvailableAccounts_data();
    void testManagerAvailableAccounts();
    void testManagerAccount();
    void testManagerRequestAccess_data();
    void testManagerRequestAccess();
    void testAccountData_data();
    void testAccountData();
    void testAccountChanges();
    void testPendingCallWatcher();

private:
    QtDBusTest::DBusTestRunner m_dbus;
    QtDBusMock::DBusMock m_mock;
};

FunctionalTests::FunctionalTests():
    QObject(),
    m_mock(m_dbus)
{
    m_mock.registerCustomMock(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                              ONLINE_ACCOUNTS_MANAGER_PATH,
                              ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                              QDBusConnection::SessionBus);
    m_dbus.startServices();
}

void FunctionalTests::testManagerReady_data()
{
    QTest::addColumn<bool>("haveGetAccountsMethod");

    QTest::newRow("no GetAccounts method") << false;
    QTest::newRow("with GetAccounts method") << true;
}

void FunctionalTests::testManagerReady()
{
    QFETCH(bool, haveGetAccountsMethod);

    if (haveGetAccountsMethod) {
        addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", "ret = []");
    }
    OnlineAccounts::Manager manager("my-app");

    QSignalSpy ready(&manager, SIGNAL(ready()));

    manager.waitForReady();
    QVERIFY(manager.isReady());
    QCOMPARE(ready.count(), 1);
}

void FunctionalTests::testManagerAvailableAccounts_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<QList<int> >("expectedIds");
    QTest::addColumn<QStringList>("expectedDisplayNames");

    QTest::newRow("no accounts") <<
        "ret = []" <<
        QList<int>() <<
        QStringList();

    QTest::newRow("one account, no data") <<
        "ret = [(1, {'displayName': 'Tom'})]" <<
        (QList<int>() << 1) <<
        (QStringList() << "Tom");
}

void FunctionalTests::testManagerAvailableAccounts()
{
    QFETCH(QString, reply);
    QFETCH(QList<int>, expectedIds);
    QFETCH(QStringList, expectedDisplayNames);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", reply);
    OnlineAccounts::Manager manager("my-app");

    manager.waitForReady();
    QList<int> ids;
    QStringList displayNames;
    Q_FOREACH(OnlineAccounts::Account *account, manager.availableAccounts()) {
        ids.append(account->id());
        displayNames.append(account->displayName());
    }

    QCOMPARE(ids, expectedIds);
    QCOMPARE(displayNames, expectedDisplayNames);
}

void FunctionalTests::testManagerAccount()
{
    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    "ret = [(1, {'displayName': 'John'})]");
    OnlineAccounts::Manager manager("my-app");

    manager.waitForReady();

    // retrieve an invalid account
    OnlineAccounts::Account *account = manager.account(4);
    QVERIFY(!account);

    // valid account
    account = manager.account(1);
    QVERIFY(account);
    QCOMPARE(account->displayName(), QString("John"));
}

void FunctionalTests::testManagerRequestAccess_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<bool>("waitForFinished");
    QTest::addColumn<int>("errorCode");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<QString>("accessToken");

    QTest::newRow("dbus error") <<
        "raise dbus.exceptions.DBusException('org.foo.bar', 'not foobarized')" <<
        true <<
        int(OnlineAccounts::Error::PermissionDenied) <<
        "" <<
        "";

    QTest::newRow("access granted") <<
        "ret = ((1, {'displayName': 'Bob'}),{'AccessToken':'GoOn'})" <<
        true <<
        int(0) <<
        "Bob" <<
        "GoOn";
    QTest::newRow("access granted, no wait") <<
        "ret = ((1, {'displayName': 'Bob'}),{'AccessToken':'GoOn'})" <<
        false <<
        int(0) <<
        "Bob" <<
        "GoOn";
}

void FunctionalTests::testManagerRequestAccess()
{
    QFETCH(QString, reply);
    QFETCH(bool, waitForFinished);
    QFETCH(int, errorCode);
    QFETCH(QString, displayName);
    QFETCH(QString, accessToken);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", "ret = []");
    addMockedMethod("RequestAccess", "sa{sv}", "(ua{sv})a{sv}", reply);
    OnlineAccounts::Manager manager("my-app");

    manager.waitForReady();

    OnlineAccounts::OAuth2Data oauth;
    oauth.setClientId("happy app");
    OnlineAccounts::PendingCall call = manager.requestAccess("my-service",
                                                             oauth);
    if (waitForFinished) {
        call.waitForFinished();
        QVERIFY(call.isFinished());
    }

    OnlineAccounts::RequestAccessReply accessReply(call);
    QCOMPARE(int(accessReply.error().code()), errorCode);

    OnlineAccounts::Account *account = accessReply.account();
    if (errorCode > 0) {
        QVERIFY(!account);
    } else {
        QVERIFY(account);
        QCOMPARE(account->displayName(), displayName);
    }

    OnlineAccounts::OAuth2Reply oauthReply(call);
    QCOMPARE(int(oauthReply.error().code()), errorCode);
    QCOMPARE(oauthReply.accessToken(), accessToken.toUtf8());
}

void FunctionalTests::testAccountData_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<int>("accountId");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<QString>("serviceId");
    QTest::addColumn<int>("authenticationMethod");
    QTest::addColumn<QVariantMap>("settings");

    QTest::newRow("empty account") <<
        "ret = [(1, {})]" <<
        1 <<
        "" << "" << 0 << QVariantMap();

    QTest::newRow("no settings") <<
        "ret = [(3, {"
        "  'displayName': 'Bob',"
        "  'serviceId': 'MyService',"
        "  'authMethod': 1,"
        "})]" <<
        3 <<
        "Bob" <<
        "MyService" <<
        1 <<
        QVariantMap();

    QVariantMap settings;
    settings.insert("Host", "example.com");
    settings.insert("Port", int(7000));
    QTest::newRow("with settings") <<
        "ret = [(4, {"
        "  'displayName': 'Tom',"
        "  'serviceId': 'MyService',"
        "  'authMethod': 2,"
        "  'settings/Host': 'example.com',"
        "  'settings/Port': 7000,"
        "})]" <<
        4 <<
        "Tom" <<
        "MyService" <<
        2 <<
        settings;
}

void FunctionalTests::testAccountData()
{
    QFETCH(QString, reply);
    QFETCH(int, accountId);
    QFETCH(QString, displayName);
    QFETCH(QString, serviceId);
    QFETCH(int, authenticationMethod);
    QFETCH(QVariantMap, settings);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", reply);
    OnlineAccounts::Manager manager("my-app");

    manager.waitForReady();

    QList<OnlineAccounts::Account*> accounts = manager.availableAccounts();
    QCOMPARE(accounts.count(), 1);

    OnlineAccounts::Account *account = accounts.first();
    QVERIFY(account);
    QVERIFY(account->isValid());
    QCOMPARE(int(account->id()), accountId);
    QCOMPARE(account->displayName(), displayName);
    QCOMPARE(account->serviceId(), serviceId);
    QCOMPARE(int(account->authenticationMethod()), authenticationMethod);

    Q_FOREACH(const QString &key, settings.keys()) {
        QCOMPARE(account->setting(key), settings.value(key));
    }

    delete account;
}

void FunctionalTests::testPendingCallWatcher()
{
    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", "ret = []");
    addMockedMethod("RequestAccess", "sa{sv}", "(ua{sv})a{sv}",
                    "ret = ((1, {'displayName': 'Bob'}),{})");
    OnlineAccounts::Manager manager("my-app");

    manager.waitForReady();

    OnlineAccounts::OAuth2Data oauth;
    oauth.setClientId("happy app");
    OnlineAccounts::PendingCall call = manager.requestAccess("my-service",
                                                             oauth);
    // Test also the PendingCall assignment operator
    OnlineAccounts::PendingCall otherCall(call);
    call = otherCall;

    QVERIFY(!call.isFinished());
    QVERIFY(!otherCall.isFinished());
    OnlineAccounts::PendingCallWatcher *watcher =
        new OnlineAccounts::PendingCallWatcher(otherCall);
    QSignalSpy finished(watcher, SIGNAL(finished()));

    QVERIFY(finished.wait());
    QCOMPARE(finished.count(), 1);
    QVERIFY(call.isFinished());
    QVERIFY(otherCall.isFinished());

    delete watcher;
}

void FunctionalTests::testAccountChanges()
{
    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", "ret = []");
    OnlineAccounts::Manager manager("my-app");
    QSignalSpy accountAvailable(&manager,
                                SIGNAL(accountAvailable(OnlineAccounts::Account*)));

    manager.waitForReady();

    QVariantMap changes;
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE,
                   uint(ONLINE_ACCOUNTS_INFO_CHANGE_ENABLED));
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME, "John");
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID, "coolService");
    emitAccountChanged("coolService", 5, changes);

    QVERIFY(accountAvailable.wait());
    QCOMPARE(accountAvailable.count(), 1);

    OnlineAccounts::Account *account =
        accountAvailable.at(0).at(0).value<OnlineAccounts::Account*>();
    QSignalSpy changed(account, SIGNAL(changed()));
    QSignalSpy disabled(account, SIGNAL(disabled()));

    QVERIFY(account);
    QVERIFY(account->isValid());
    QCOMPARE(account->id(), OnlineAccounts::AccountId(5));
    QCOMPARE(account->displayName(), QString("John"));
    QCOMPARE(account->serviceId(), QString("coolService"));

    /* Now change some of the account data */
    accountAvailable.clear();
    changes.clear();
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE,
                   uint(ONLINE_ACCOUNTS_INFO_CHANGE_UPDATED));
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME, "Bob");
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID, "coolService");
    emitAccountChanged("coolService", 5, changes);

    QVERIFY(changed.wait());
    QCOMPARE(accountAvailable.count(), 0);
    QCOMPARE(changed.count(), 1);
    QCOMPARE(disabled.count(), 0);
    QVERIFY(account->isValid());
    QCOMPARE(account->id(), OnlineAccounts::AccountId(5));
    QCOMPARE(account->displayName(), QString("Bob"));
    QCOMPARE(account->serviceId(), QString("coolService"));

    /* Now disable the account */
    changed.clear();
    changes.clear();
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE,
                   uint(ONLINE_ACCOUNTS_INFO_CHANGE_DISABLED));
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME, "Bob");
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID, "coolService");
    emitAccountChanged("coolService", 5, changes);

    QVERIFY(disabled.wait());
    QCOMPARE(accountAvailable.count(), 0);
    QCOMPARE(changed.count(), 0);
    QCOMPARE(disabled.count(), 1);
    QVERIFY(!account->isValid());
}

QTEST_MAIN(FunctionalTests)
#include "functional_tests.moc"
