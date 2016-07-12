/*
 * This file is part of libOnlineAccounts
 *
 * Copyright (C) 2015-2016 Canonical Ltd.
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
#include "OnlineAccounts/account_info.h"
#include "OnlineAccountsDaemon/dbus_constants.h"
#include <QDBusConnection>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <libqtdbusmock/DBusMock.h>

namespace QTest {
template<>
char *toString(const QVariantMap &map)
{
    QJsonDocument doc(QJsonObject::fromVariantMap(map));
    return qstrdup(doc.toJson(QJsonDocument::Compact).data());
}
} // QTest namespace

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
    void testMultipleServices();
    void testPendingCallWatcher();
    void testAuthentication();
    void testAuthenticationErrors_data();
    void testAuthenticationErrors();

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
        "ret = ((1, {'displayName': 'Bob', 'serviceId': 'my-service'}),{'AccessToken':'GoOn'})" <<
        true <<
        int(0) <<
        "Bob" <<
        "GoOn";
    QTest::newRow("access granted, no wait") <<
        "ret = ((1, {'displayName': 'Bob', 'serviceId': 'my-service'}),{'AccessToken':'GoOn'})" <<
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

    QVariantMap accountSettings;
    Q_FOREACH(const QString &key, account->keys()) {
        accountSettings.insert(key, account->setting(key));
    }
    QCOMPARE(accountSettings, settings);

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

void FunctionalTests::testMultipleServices()
{
    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    "ret = ["
                    "(1, {"
                    "  'displayName': 'One',"
                    "  'serviceId': 'service2',"
                    "  'authMethod': 1,"
                    "}),"
                    "(2, {"
                    "  'displayName': 'Two',"
                    "  'serviceId': 'service1',"
                    "  'authMethod': 1,"
                    "}),"
                    "(2, {"
                    "  'displayName': 'Two',"
                    "  'serviceId': 'service2',"
                    "  'authMethod': 1,"
                    "}),"
                    "(4, {"
                    "  'displayName': 'Three',"
                    "  'serviceId': 'service1',"
                    "  'authMethod': 1,"
                    "}),"
                    "]");
    OnlineAccounts::Manager manager("my-app");
    manager.waitForReady();

    // All account services are available, including both from account #2
    auto accounts = manager.availableAccounts();
    QCOMPARE(accounts.count(), 4);

    // Picks the first known service for the given account ID.
    OnlineAccounts::Account *account = manager.account(2);
    QVERIFY(account);
    QVERIFY(account->isValid());
    QCOMPARE(account->id(), OnlineAccounts::AccountId(2));
    QCOMPARE(account->displayName(), QString("Two"));
    QCOMPARE(account->serviceId(), QString("service1"));

    account = manager.account(2, "service2");
    QVERIFY(account);
    QCOMPARE(account->id(), OnlineAccounts::AccountId(2));
    QCOMPARE(account->serviceId(), QString("service2"));

    account = manager.account(2, "service3");
    QVERIFY(account == nullptr);

    account = manager.account(3);
    QVERIFY(account == nullptr);
}

void FunctionalTests::testAuthentication()
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
                    "(4, {"
                    "  'displayName': 'Jim',"
                    "  'serviceId': 'MyService3',"
                    "  'authMethod': 4,"
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
                    "elif args[0] == 4:\n"
                    "  ret = {"
                    "    'Response': 'pong',"
                    "    'ChosenMechanism': 'tennis',"
                    "    'state': 1,"
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
    /* Compare the whole data dictionary */
    QVariantMap expectedData;
    expectedData.insert(ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_KEY, "a key");
    expectedData.insert(ONLINE_ACCOUNTS_AUTH_KEY_CONSUMER_SECRET, "a secret");
    expectedData.insert(ONLINE_ACCOUNTS_AUTH_KEY_TOKEN, "a token");
    expectedData.insert(ONLINE_ACCOUNTS_AUTH_KEY_TOKEN_SECRET, "a token secret");
    expectedData.insert(ONLINE_ACCOUNTS_AUTH_KEY_SIGNATURE_METHOD, "PLAIN");
    QCOMPARE(oauth1reply.data(), expectedData);

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

    /* Test SASL */
    account = manager.account(4);
    QVERIFY(account);
    QCOMPARE(account->authenticationMethod(),
             OnlineAccounts::AuthenticationMethodSasl);
    OnlineAccounts::SaslData sasldata;

    OnlineAccounts::SaslReply saslreply(account->authenticate(sasldata));
    QCOMPARE(saslreply.chosenMechanism(), QString("tennis"));
    QCOMPARE(saslreply.response(), QByteArray("pong"));
    QCOMPARE(saslreply.state(), OnlineAccounts::SaslReply::Continue);

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

void FunctionalTests::testAuthenticationErrors_data()
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

void FunctionalTests::testAuthenticationErrors()
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

QTEST_MAIN(FunctionalTests)
#include "functional_tests.moc"
