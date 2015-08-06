/*
 * This file is part of OnlineAccountsModule
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
#include <QDBusArgument>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
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

class ModuleTest: public QObject
{
    Q_OBJECT

public:
    ModuleTest();

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
        QDBusArgument info;
        info.beginStructure();
        info << accountId << changes;
        info.endStructure();
        args << QVariant::fromValue(info);
        mocked().EmitSignal(ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                            "AccountChanged", "s(ua{sv})",
                            args);
    }

    QVariant get(const QAbstractListModel *model, int row, QString roleName)
    {
        QHash<int, QByteArray> roleNames = model->roleNames();
        int role = roleNames.key(roleName.toLatin1(), -1);
        return model->data(model->index(row), role);
    }

private Q_SLOTS:
    void initTestCase();
    void testModuleImport();
    void testModelProperties();
    void testModelRoles_data();
    void testModelRoles();
    void testModelFilters_data();
    void testModelFilters();
    void testModelChanges();
    void testModelRequestAccess_data();
    void testModelRequestAccess();
    void testAccountAuthentication_data();
    void testAccountAuthentication();

private:
    QtDBusTest::DBusTestRunner m_dbus;
    QtDBusMock::DBusMock m_mock;
};

ModuleTest::ModuleTest():
    QObject(),
    m_mock(m_dbus)
{
    m_mock.registerCustomMock(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                              ONLINE_ACCOUNTS_MANAGER_PATH,
                              ONLINE_ACCOUNTS_MANAGER_INTERFACE,
                              QDBusConnection::SessionBus);
    m_dbus.startServices();
}

void ModuleTest::initTestCase()
{
    qputenv("QML2_IMPORT_PATH", TEST_QML_IMPORT_PATH);
}

void ModuleTest::testModuleImport()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    delete object;
}

void ModuleTest::testModelProperties()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);

    QVERIFY(object->setProperty("applicationId", "foo"));
    QCOMPARE(object->property("applicationId").toString(), QString("foo"));

    QVERIFY(object->setProperty("serviceId", "bar"));
    QCOMPARE(object->property("serviceId").toString(), QString("bar"));

    delete object;
}

void ModuleTest::testModelRoles_data()
{
    QTest::addColumn<QString>("accountData");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<QString>("serviceId");
    QTest::addColumn<int>("authenticationMethod");

    QTest::newRow("empty") <<
        "{}" <<
        "" << "" << 0;
    QTest::newRow("complete") <<
        "{"
        " '" ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME "': 'Tom',"
        " '" ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID "': 'cool',"
        " '" ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD "': 2,"
        "}" <<
        "Tom" << "cool" << 2;
}

void ModuleTest::testModelRoles()
{
    QFETCH(QString, accountData);
    QFETCH(QString, displayName);
    QFETCH(QString, serviceId);
    QFETCH(int, authenticationMethod);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    QString("ret = [(1, %1)]").arg(accountData));

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel { applicationId: \"foo\" }",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QTRY_COMPARE(model->rowCount(), 1);
    QCOMPARE(model->data(model->index(0), Qt::DisplayRole).toString(),
             QString("%1 - %2").arg(displayName).arg(serviceId));
    QCOMPARE(get(model, 0, "displayName").toString(), displayName);
    QCOMPARE(get(model, 0, "valid").toBool(), true);
    QCOMPARE(get(model, 0, "accountId").toInt(), 1);
    QCOMPARE(get(model, 0, "serviceId").toString(), serviceId);
    QCOMPARE(get(model, 0, "authenticationMethod").toInt(),
             authenticationMethod);
    // until https://bugs.launchpad.net/bugs/1479768 is fixed
    QCOMPARE(get(model, 0, "settings").toMap(), QVariantMap());
    QObject *account = get(model, 0, "account").value<QObject*>();
    QVERIFY(account != 0);
    QCOMPARE(account->metaObject()->className(), "OnlineAccountsModule::Account");
    QCOMPARE(account,
             model->property("accountList").value<QList<QObject*> >().first());

    delete object;
}

void ModuleTest::testModelFilters_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<QString>("filters");
    QTest::addColumn<QList<int> >("expectedIds");
    QTest::addColumn<QStringList>("expectedDisplayNames");

    QTest::newRow("no accounts") <<
        "ret = []" <<
        "" <<
        QList<int>();

    QTest::newRow("one account, no service filter") <<
        "ret = [(1, {'displayName': 'Tom', 'serviceId': 'Foo' })]" <<
        "" <<
        (QList<int>() << 1);

    QTest::newRow("one account, with service filter") <<
        "ret = [(1, {'displayName': 'Tom', 'serviceId': 'Foo' })]" <<
        "serviceId: \"bar\"" <<
        QList<int>();
}

void ModuleTest::testModelFilters()
{
    QFETCH(QString, reply);
    QFETCH(QString, filters);
    QFETCH(QList<int>, expectedIds);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", reply);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {\n"
                      "  applicationId: \"foo\";" + filters.toUtf8() + "\n"
                      "}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QTRY_COMPARE(model->property("count").toInt(), expectedIds.count());

    QList<int> ids;
    QStringList displayNames;
    for (int i = 0; i < expectedIds.count(); i++) {
        ids.append(get(model, i, "accountId").toInt());
    }

    QCOMPARE(ids, expectedIds);

    delete object;
}

void ModuleTest::testModelChanges()
{
    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})", "ret = []");
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel { applicationId: \"foo\" }",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QSignalSpy countChanged(model, SIGNAL(countChanged()));
    QSignalSpy dataChanged(model, SIGNAL(dataChanged(const QModelIndex&,
                                                     const QModelIndex&)));

    QVariantMap changes;
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE,
                   uint(ONLINE_ACCOUNTS_INFO_CHANGE_ENABLED));
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME, "John");
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID, "coolService");
    emitAccountChanged("coolService", 5, changes);

    QVERIFY(countChanged.wait());
    QCOMPARE(dataChanged.count(), 0);
    QTRY_COMPARE(model->rowCount(), 1);
    QCOMPARE(model->property("accountList").value<QObjectList>().count(), 1);

    QCOMPARE(get(model, 0, "displayName").toString(), QString("John"));
    QObject *account =
        model->property("accountList").value<QObjectList>().first();
    QSignalSpy accountChanged(account, SIGNAL(accountChanged()));
    QSignalSpy validChanged(account, SIGNAL(validChanged()));

    QVERIFY(account);
    QVERIFY(account->property("valid").toBool());
    QCOMPARE(account->property("displayName").toString(), QString("John"));

    countChanged.clear();

    // Change the display name
    changes.clear();
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE,
                   uint(ONLINE_ACCOUNTS_INFO_CHANGE_UPDATED));
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME, "Bob");
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID, "coolService");
    emitAccountChanged("coolService", 5, changes);

    QVERIFY(dataChanged.wait());
    QCOMPARE(dataChanged.count(), 1);
    QCOMPARE(countChanged.count(), 0);
    QCOMPARE(accountChanged.count(), 1);
    QCOMPARE(validChanged.count(), 0);
    QCOMPARE(get(model, 0, "displayName").toString(), QString("Bob"));

    dataChanged.clear();
    accountChanged.clear();

    // disable the account
    changes.clear();
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_CHANGE_TYPE,
                   uint(ONLINE_ACCOUNTS_INFO_CHANGE_DISABLED));
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME, "Bob");
    changes.insert(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID, "coolService");
    emitAccountChanged("coolService", 5, changes);

    QVERIFY(countChanged.wait());
    QCOMPARE(countChanged.count(), 1);
    QCOMPARE(accountChanged.count(), 0);
    QCOMPARE(validChanged.count(), 1);
    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->property("accountList").value<QObjectList>().count(), 0);
    QCOMPARE(account->property("valid").toBool(), false);

    delete object;
}

void ModuleTest::testModelRequestAccess_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<QVariantMap>("expectedAccessReply");
    QTest::addColumn<QVariantMap>("expectedAuthenticationData");

    QVariantMap accessReply;
    QVariantMap authenticationData;
    accessReply["errorCode"] = 4;
    accessReply["errorText"] = "('org.foo.bar', 'not foobarized')";
    QTest::newRow("dbus error") <<
        "raise dbus.exceptions.DBusException('org.foo.bar', 'not foobarized')" <<
        accessReply <<
        authenticationData;

    accessReply.clear();
    accessReply["accountDisplayName"] = "Bob";
    authenticationData.clear();
    QTest::newRow("access granted") <<
        "ret = ((1, {'displayName': 'Bob'}),{'AccessToken':'GoOn'})" <<
        accessReply <<
        authenticationData;
}

void ModuleTest::testModelRequestAccess()
{
    QFETCH(QString, reply);
    QFETCH(QVariantMap, expectedAccessReply);
    QFETCH(QVariantMap, expectedAuthenticationData);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    "ret = [(1, {'displayName': 'Tom', 'serviceId': 'Foo' })]");
    addMockedMethod("RequestAccess", "sa{sv}", "(ua{sv})a{sv}", reply);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {\n"
                      "  applicationId: \"foo\"\n"
                      "}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QSignalSpy accessReplySpy(model,
                              SIGNAL(accessReply(const QVariantMap&,const QVariantMap&)));


    QString serviceId = "bar";
    QVariantMap params;
    bool ok = QMetaObject::invokeMethod(model, "requestAccess",
                                        Q_ARG(QString, serviceId),
                                        Q_ARG(QVariantMap, params));
    QVERIFY(ok);

    QVERIFY(accessReplySpy.wait());
    QVariantMap accessReply = accessReplySpy.at(0).at(0).toMap();
    QVariantMap authenticationData = accessReplySpy.at(0).at(1).toMap();

    if (expectedAccessReply.contains("accountDisplayName")) {
        QObject *account = accessReply["account"].value<QObject*>();
        QVERIFY(account != 0);
        QCOMPARE(account->metaObject()->className(),
                 "OnlineAccountsModule::Account");
        QCOMPARE(account->property("displayName").toString(),
                 expectedAccessReply["accountDisplayName"].toString());
    } else {
        QCOMPARE(accessReply, expectedAccessReply);
    }
    QCOMPARE(authenticationData, expectedAuthenticationData);

    delete object;
}

void ModuleTest::testAccountAuthentication_data()
{
    QTest::addColumn<int>("authMethod");
    QTest::addColumn<QString>("reply");
    QTest::addColumn<QVariantMap>("params");
    QTest::addColumn<QVariantMap>("expectedAuthenticationData");

    QVariantMap params;
    QVariantMap authenticationData;

    authenticationData["errorCode"] = 4;
    authenticationData["errorText"] = "not foobarized";
    QTest::newRow("random dbus error") <<
        ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH1 <<
        "raise dbus.exceptions.DBusException('not foobarized', name='org.foo.bar')" <<
        params <<
        authenticationData;

    authenticationData["errorCode"] = 1;
    authenticationData["errorText"] = "Not there";
    QTest::newRow("no account") <<
        ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH1 <<
        "raise dbus.exceptions.DBusException('Not there',"
        "name='" ONLINE_ACCOUNTS_ERROR_NO_ACCOUNT "')" <<
        params <<
        authenticationData;

    authenticationData["errorCode"] = 3;
    authenticationData["errorText"] = "Sorry";
    QTest::newRow("user canceled") <<
        ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH1 <<
        "raise dbus.exceptions.DBusException('Sorry',"
        "name='" ONLINE_ACCOUNTS_ERROR_USER_CANCELED "')" <<
        params <<
        authenticationData;

    params["ConsumerKey"] = "aaaa";
    params["ConsumerSecret"] = "bbb";
    authenticationData = params;
    authenticationData["Token"] = "a token";
    authenticationData["TokenSecret"] = "a token secret";
    authenticationData["SignatureMethod"] = "PLAIN";
    QTest::newRow("oauth 1.0") <<
        ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH1 <<
        "ret = {"
        "  'ConsumerKey': args[4]['ConsumerKey'],"
        "  'ConsumerSecret': args[4]['ConsumerSecret'],"
        "  'Token': 'a token',"
        "  'TokenSecret': 'a token secret',"
        "  'SignatureMethod': 'PLAIN',"
        "}" <<
        params <<
        authenticationData;

    params.clear();
    authenticationData.clear();
    params["ClientId"] = "aaaa";
    params["ClientSecret"] = "bbb";
    params["Scopes"] = (QStringList() << "one" << "two");
    authenticationData["AccessToken"] = "my token";
    authenticationData["ExpiresIn"] = 3600;
    authenticationData["GrantedScopes"] = (QStringList() << "one" << "two");
    QTest::newRow("oauth 2.0") <<
        ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH2 <<
        "ret = {"
        "  'AccessToken': 'my token',"
        "  'ExpiresIn': 3600,"
        "  'GrantedScopes': args[4]['Scopes'],"
        "}" <<
        params <<
        authenticationData;

    params.clear();
    authenticationData.clear();
    authenticationData["Username"] = "admin";
    authenticationData["Password"] = "rootme";
    QTest::newRow("password") <<
        ONLINE_ACCOUNTS_AUTH_METHOD_PASSWORD <<
        "ret = {"
        "  'Username': 'admin',"
        "  'Password': 'rootme',"
        "}" <<
        params <<
        authenticationData;
}

void ModuleTest::testAccountAuthentication()
{
    QFETCH(int, authMethod);
    QFETCH(QString, reply);
    QFETCH(QVariantMap, params);
    QFETCH(QVariantMap, expectedAuthenticationData);

    addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})",
                    QString("ret = [(1, {"
                            "  'displayName': 'Bob',"
                            "  'serviceId': 'MyService0',"
                            "  'authMethod': %1,"
                            "})]").arg(authMethod));
    addMockedMethod("Authenticate", "usbba{sv}", "a{sv}", reply);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {\n"
                      "  applicationId: \"foo\"\n"
                      "}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QTRY_COMPARE(model->rowCount(), 1);

    QObject *account =
        model->property("accountList").value<QObjectList>().first();
    QSignalSpy authenticationReply(account,
                              SIGNAL(authenticationReply(const QVariantMap&)));

    bool ok = QMetaObject::invokeMethod(account, "authenticate",
                                        Q_ARG(QVariantMap, params));
    QVERIFY(ok);

    QVERIFY(authenticationReply.wait());
    QVariantMap authenticationData = authenticationReply.at(0).at(0).toMap();

    QCOMPARE(authenticationData, expectedAuthenticationData);

    delete object;
}
/*
void ModuleTest::testAuthentication()
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

    account = manager.account(3);
    QVERIFY(account);
    QCOMPARE(account->authenticationMethod(),
             OnlineAccounts::AuthenticationMethodPassword);
    OnlineAccounts::PasswordData pwdata;

    OnlineAccounts::PasswordReply pwreply(account->authenticate(pwdata));
    QCOMPARE(pwreply.username(), QByteArray("admin"));
    QCOMPARE(pwreply.password(), QByteArray("rootme"));

    OnlineAccounts::OAuth2Data copy(oauth2data);
    QCOMPARE(copy.clientId(), QByteArray("a client"));
    copy.setClientId("new client");
    QCOMPARE(copy.clientId(), QByteArray("new client"));
    QCOMPARE(oauth2data.clientId(), QByteArray("a client"));
}

void ModuleTest::testAuthenticationErrors_data()
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

void ModuleTest::testAuthenticationErrors()
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
*/

QTEST_MAIN(ModuleTest)
#include "tst_qml_module.moc"
