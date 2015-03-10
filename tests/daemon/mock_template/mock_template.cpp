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
#include "OnlineAccounts/OAuth1Data"
#include "OnlineAccounts/OAuth2Data"
#include "OnlineAccounts/PasswordData"
#include "OnlineAccounts/dbus_constants.h"
#include <QDBusConnection>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <libqtdbusmock/DBusMock.h>

struct AccountDetails
{
    AccountDetails() {}
    AccountDetails(const QString &name, const QString &service,
                   const QVariantMap &settings = QVariantMap()) {
        map[ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME] = name;
        map[ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID] = service;
        QMapIterator<QString, QVariant> it(settings);
        while (it.hasNext()) {
            it.next();
            map[ONLINE_ACCOUNTS_INFO_KEY_SETTINGS + it.key()] = it.value();
        }
    }

    QVariantMap map;
};

class MockTemplateTests: public QObject
{
    Q_OBJECT

public:
    MockTemplateTests();

    OrgFreedesktopDBusMockInterface &mocked() {
        return m_mock.mockInterface(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                                    ONLINE_ACCOUNTS_MANAGER_PATH,
                                    ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                                    QDBusConnection::SessionBus);
    }

    uint addAccount(const AccountDetails &details);
    void setRequestAccessReply(const AccountDetails &details);
    void setAuthenticationReply(const QVariantMap &reply,
                                const QString &errorName = QString());

private Q_SLOTS:
    void testManagerAvailableAccounts();
    void testManagerRequestAccess();
#if 0
    void testAccountData_data();
    void testAccountData();
    void testAccountChanges();
    void testPendingCallWatcher();
    void testAuthentication();
    void testAuthenticationErrors_data();
    void testAuthenticationErrors();
#endif

private:
    QtDBusTest::DBusTestRunner m_dbus;
    QtDBusMock::DBusMock m_mock;
};

MockTemplateTests::MockTemplateTests():
    QObject(),
    m_mock(m_dbus)
{
    m_mock.registerCustomMock(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                              ONLINE_ACCOUNTS_MANAGER_PATH,
                              ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                              QDBusConnection::SessionBus);
    m_dbus.startServices();
    mocked().AddTemplate(MOCK_TEMPLATE, QVariantMap());
}

uint MockTemplateTests::addAccount(const AccountDetails &details)
{
    QDBusReply<uint> reply = mocked().call("AddAccount", details.map);
    return reply.isValid() ? reply.value() : 0;
}

void MockTemplateTests::setRequestAccessReply(const AccountDetails &details)
{
    mocked().call("SetRequestAccessReply", details.map);
}

void MockTemplateTests::setAuthenticationReply(const QVariantMap &reply,
                                               const QString &errorName)
{
    mocked().call("SetAuthenticationReply", reply, errorName);
}

void MockTemplateTests::testManagerAvailableAccounts()
{
    OnlineAccounts::Manager manager("my-app");
    QSignalSpy accountAvailable(&manager,
                                SIGNAL(accountAvailable(OnlineAccounts::Account*)));

    manager.waitForReady();
    QVERIFY(manager.availableAccounts().isEmpty());

    uint accountId = addAccount(AccountDetails("cold account", "cool service"));
    QVERIFY(accountId != 0);

    if (accountAvailable.count() == 0) {
        accountAvailable.wait();
    }
    QCOMPARE(accountAvailable.count(), 1);

    QList<OnlineAccounts::Account *> accounts = manager.availableAccounts();
    QCOMPARE(accounts.count(), 1);

    OnlineAccounts::Account *account = accounts.first();
    QCOMPARE(uint(account->id()), accountId);
    QCOMPARE(account->displayName(), QString("cold account"));
    QCOMPARE(account->serviceId(), QString("cool service"));
}

void MockTemplateTests::testManagerRequestAccess()
{
    OnlineAccounts::Manager manager("my-app");
    manager.waitForReady();

    setRequestAccessReply(AccountDetails("Tom", "photos"));
    QVariantMap replyData;
    replyData[ONLINE_ACCOUNTS_AUTH_KEY_ACCESS_TOKEN] = QByteArray("go-on");
    setAuthenticationReply(replyData);

    OnlineAccounts::OAuth2Data oauth;
    oauth.setClientId("happy app");
    OnlineAccounts::PendingCall call = manager.requestAccess("my-service",
                                                             oauth);
    OnlineAccounts::RequestAccessReply accessReply(call);
    OnlineAccounts::Account *account = accessReply.account();
    QVERIFY(account);
    QCOMPARE(account->displayName(), QString("Tom"));

    OnlineAccounts::OAuth2Reply oauthReply(call);
    QCOMPARE(oauthReply.accessToken(), QByteArray("go-on"));
}

#if 0
void MockTemplateTests::testAccountData_data()
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

void MockTemplateTests::testAccountData()
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

void MockTemplateTests::testPendingCallWatcher()
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

void MockTemplateTests::testAccountChanges()
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

void MockTemplateTests::testAuthentication()
{
    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    "ret = ["
                    "(1, {"
                    "  'displayName': 'Bob',"
                    "  'serviceId': 'MyService0',"
                    "  'authMethod': 1,"
                    "}),"
                    "(2, {"
                    "  'displayName': 'Tom',"
                    "  'serviceId': 'MyService1',"
                    "  'authMethod': 2,"
                    "}),"
                    "(3, {"
                    "  'displayName': 'Sam',"
                    "  'serviceId': 'MyService2',"
                    "  'authMethod': 3,"
                    "}),"
                    "]");
    addMockedMethod("Authenticate", "usbba{sv}", "a{sv}",
                    "if args[0] == 1:\n"
                    "  ret = {"
                    "    'ConsumerKey': args[4]['ConsumerKey'],"
                    "    'ConsumerSecret': args[4]['ConsumerSecret'],"
                    "    'Token': 'a token',"
                    "    'TokenSecret': 'a token secret',"
                    "    'SignatureMethod': 'PLAIN',"
                    "  }\n"
                    "elif args[0] == 2:\n"
                    "  ret = {"
                    "    'AccessToken': 'my token',"
                    "    'ExpiresIn': 3600,"
                    "    'GrantedScopes': args[4]['Scopes'],"
                    "  }\n"
                    "elif args[0] == 3:\n"
                    "  ret = {"
                    "    'Username': 'admin',"
                    "    'Password': 'rootme',"
                    "  }\n"
                    "else:\n"
                    "  ret = {}");
    OnlineAccounts::Manager manager("my-app");
    manager.waitForReady();

    /* Test OAuth 1.0 */
    OnlineAccounts::Account *account = manager.account(1);
    QVERIFY(account);
    QCOMPARE(account->authenticationMethod(),
             OnlineAccounts::AuthenticationMethodOAuth1);
    OnlineAccounts::OAuth1Data oauth1data;
    oauth1data.setInteractive(false);
    oauth1data.setConsumerKey("a key");
    QCOMPARE(oauth1data.consumerKey(), QByteArray("a key"));
    oauth1data.setConsumerSecret("a secret");
    QCOMPARE(oauth1data.consumerSecret(), QByteArray("a secret"));

    OnlineAccounts::OAuth1Reply oauth1reply(account->authenticate(oauth1data));
    QCOMPARE(oauth1reply.consumerKey(), QByteArray("a key"));
    QCOMPARE(oauth1reply.consumerSecret(), QByteArray("a secret"));
    QCOMPARE(oauth1reply.token(), QByteArray("a token"));
    QCOMPARE(oauth1reply.tokenSecret(), QByteArray("a token secret"));
    QCOMPARE(oauth1reply.signatureMethod(), QByteArray("PLAIN"));

    /* Test OAuth 2.0 */
    account = manager.account(2);
    QVERIFY(account);
    QCOMPARE(account->authenticationMethod(),
             OnlineAccounts::AuthenticationMethodOAuth2);
    OnlineAccounts::OAuth2Data oauth2data;
    oauth2data.invalidateCachedReply();
    QVERIFY(oauth2data.mustInvalidateCachedReply());
    oauth2data.setClientId("a client");
    QCOMPARE(oauth2data.clientId(), QByteArray("a client"));
    oauth2data.setClientSecret("a secret");
    QCOMPARE(oauth2data.clientSecret(), QByteArray("a secret"));
    QList<QByteArray> scopes =
        QList<QByteArray>() << "one" << "two" << "three";
    oauth2data.setScopes(scopes);
    QCOMPARE(oauth2data.scopes(), scopes);

    OnlineAccounts::OAuth2Reply oauth2reply(account->authenticate(oauth2data));
    QCOMPARE(oauth2reply.accessToken(), QByteArray("my token"));
    QCOMPARE(oauth2reply.expiresIn(), 3600);
    QCOMPARE(oauth2reply.grantedScopes(), scopes);

    /* Test Password */
    account = manager.account(3);
    QVERIFY(account);
    QCOMPARE(account->authenticationMethod(),
             OnlineAccounts::AuthenticationMethodPassword);
    OnlineAccounts::PasswordData pwdata;

    OnlineAccounts::PasswordReply pwreply(account->authenticate(pwdata));
    QCOMPARE(pwreply.username(), QByteArray("admin"));
    QCOMPARE(pwreply.password(), QByteArray("rootme"));

    /* Test the copy constructor */
    OnlineAccounts::OAuth2Data copy(oauth2data);
    QCOMPARE(copy.clientId(), QByteArray("a client"));
    /* Trigger the copy on write */
    copy.setClientId("new client");
    QCOMPARE(copy.clientId(), QByteArray("new client"));
    QCOMPARE(oauth2data.clientId(), QByteArray("a client"));
}

void MockTemplateTests::testAuthenticationErrors_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<int>("errorCode");
    QTest::addColumn<QString>("errorMessage");

    QTest::newRow("random dbus error") <<
        "raise dbus.exceptions.DBusException('not foobarized', name='org.foo.bar')" <<
        int(OnlineAccounts::Error::PermissionDenied) <<
        "not foobarized";

    QTest::newRow("no account") <<
        "raise dbus.exceptions.DBusException('Not there',"
        "name='" ONLINE_ACCOUNTS_ERROR_NO_ACCOUNT "')" <<
        int(OnlineAccounts::Error::NoAccount) <<
        "Not there";

    QTest::newRow("user canceled") <<
        "raise dbus.exceptions.DBusException('Sorry',"
        "name='" ONLINE_ACCOUNTS_ERROR_USER_CANCELED "')" <<
        int(OnlineAccounts::Error::UserCanceled) <<
        "Sorry";

    QTest::newRow("permission denied") <<
        "raise dbus.exceptions.DBusException('Nope',"
        "name='" ONLINE_ACCOUNTS_ERROR_PERMISSION_DENIED "')" <<
        int(OnlineAccounts::Error::PermissionDenied) <<
        "Nope";

    QTest::newRow("Interaction required") <<
        "raise dbus.exceptions.DBusException('Ask the user',"
        "name='" ONLINE_ACCOUNTS_ERROR_INTERACTION_REQUIRED "')" <<
        int(OnlineAccounts::Error::InteractionRequired) <<
        "Ask the user";
}

void MockTemplateTests::testAuthenticationErrors()
{
    QFETCH(QString, reply);
    QFETCH(int, errorCode);
    QFETCH(QString, errorMessage);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    "ret = [(1, {"
                    "  'displayName': 'Bob',"
                    "  'serviceId': 'MyService0',"
                    "  'authMethod': 2,"
                    "})]");
    addMockedMethod("Authenticate", "usbba{sv}", "a{sv}", reply);
    OnlineAccounts::Manager manager("my-app");
    manager.waitForReady();

    OnlineAccounts::Account *account = manager.account(1);
    QVERIFY(account);

    OnlineAccounts::OAuth2Data oauth2data;
    oauth2data.setClientId("a client");
    oauth2data.setClientSecret("a secret");
    oauth2data.setScopes(QList<QByteArray>() << "one" << "two");

    OnlineAccounts::OAuth2Reply r(account->authenticate(oauth2data));
    QVERIFY(r.hasError());
    QCOMPARE(int(r.error().code()), errorCode);
    QCOMPARE(r.error().text(), errorMessage);
}
#endif

QTEST_MAIN(MockTemplateTests)
#include "mock_template.moc"
