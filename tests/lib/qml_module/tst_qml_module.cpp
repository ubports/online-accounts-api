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

#include "OnlineAccountsDaemon/dbus_constants.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QRegularExpression>
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

    void addGetAccountsMethod(const QString &code) {
        addMockedMethod("GetAccounts", "a{sv}", "a(ua{sv})aa{sv}", code);
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
    void testServices_data();
    void testServices();
    void testModelRoles_data();
    void testModelRoles();
    void testModelFilters_data();
    void testModelFilters();
    void testModelChanges();
    void testModelRequestAccess_data();
    void testModelRequestAccess();
    void testAccountAuthentication_data();
    void testAccountAuthentication();
    void testInitialization_data();
    void testInitialization();

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

    QCOMPARE(object->property("ready").toBool(), false);

    delete object;
}

void ModuleTest::testServices_data()
{
    QTest::addColumn<QString>("reply");
    QTest::addColumn<QList<QVariantMap>>("expectedServices");

    QTest::newRow("no services") <<
        "[]" <<
        QList<QVariantMap> {};

    QTest::newRow("one service") <<
        "[{"
        "'" ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID "': 'app_coolshare',"
        "'" ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME "': 'Cool Share',"
        "'" ONLINE_ACCOUNTS_INFO_KEY_TRANSLATIONS "': 'this package',"
        "}]" <<
        QList<QVariantMap> {
            {
                { "serviceId", "app_coolshare" },
                { "displayName", "Cool Share" },
                { "translations", "this package"},
            },
        };
}

void ModuleTest::testServices()
{
    QFETCH(QString, reply);
    QFETCH(QList<QVariantMap>, expectedServices);

    addGetAccountsMethod(QString("ret = ([], %1)").arg(reply));

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {\n"
                      "  applicationId: \"foo\"\n"
                      "  function getServices() {\n"
                      "    console.log(\"Servicees: \" + JSON.stringify(serviceList));\n"
                      "    console.log(\"Services: \" + serviceList);\n"
                      "    var ret = [];\n"
#if 1
                      "    for (var i = 0; i < serviceList.length; i++) {\n"
                      "      console.log(\"Service: \" + JSON.stringify(serviceList[i]));\n"
                      "      ret.append(serviceList[i]);\n"
                      "    }\n"
#endif
                      "    return ret;\n"
                      "  }\n"
                      "}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QSignalSpy ready(object, SIGNAL(isReadyChanged()));
    ready.wait();

    QVariant serviceList;
    bool ok = QMetaObject::invokeMethod(object, "getServices",
                                        Q_RETURN_ARG(QVariant, serviceList));
    QVERIFY(ok);

    delete object;
}

void ModuleTest::testModelRoles_data()
{
    QTest::addColumn<QString>("accountData");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<QString>("serviceId");
    QTest::addColumn<int>("authenticationMethod");
    QTest::addColumn<QVariantMap>("settings");

    QVariantMap settings;
    QTest::newRow("empty") <<
        "{}" <<
        "" << "" << 0 << settings;

    settings.insert("Server", "www.example.com");
    settings.insert("Port", "9900");
    QTest::newRow("complete") <<
        "{"
        " '" ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME "': 'Tom',"
        " '" ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID "': 'cool',"
        " '" ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD "': 2,"
        " '" ONLINE_ACCOUNTS_INFO_KEY_SETTINGS "Server': 'www.example.com',"
        " '" ONLINE_ACCOUNTS_INFO_KEY_SETTINGS "Port': 9900,"
        "}" <<
        "Tom" << "cool" << 2 << settings;
}

void ModuleTest::testModelRoles()
{
    QFETCH(QString, accountData);
    QFETCH(QString, displayName);
    QFETCH(QString, serviceId);
    QFETCH(int, authenticationMethod);
    QFETCH(QVariantMap, settings);

    addGetAccountsMethod(QString("ret = ([(1, %1)], [])").arg(accountData));

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel { applicationId: \"foo\" }",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QCOMPARE(object->property("ready").toBool(), false);
    QSignalSpy ready(object, SIGNAL(isReadyChanged()));

    ready.wait();
    QCOMPARE(object->property("ready").toBool(), true);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->data(model->index(0), Qt::DisplayRole).toString(),
             QString("%1 - %2").arg(displayName).arg(serviceId));
    QCOMPARE(get(model, 0, "displayName").toString(), displayName);
    QCOMPARE(get(model, 0, "valid").toBool(), true);
    QCOMPARE(get(model, 0, "accountId").toInt(), 1);
    QCOMPARE(get(model, 0, "serviceId").toString(), serviceId);
    QCOMPARE(get(model, 0, "authenticationMethod").toInt(),
             authenticationMethod);
    // until https://bugs.launchpad.net/bugs/1479768 is fixed
    QCOMPARE(get(model, 0, "settings").toMap(), settings);
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
        "[]" <<
        "" <<
        QList<int>();

    QTest::newRow("one account, no service filter") <<
        "[(1, {'displayName': 'Tom', 'serviceId': 'Foo' })]" <<
        "" <<
        (QList<int>() << 1);

    QTest::newRow("one account, with service filter") <<
        "[(1, {'displayName': 'Tom', 'serviceId': 'Foo' })]" <<
        "serviceId: \"bar\"" <<
        QList<int>();
}

void ModuleTest::testModelFilters()
{
    QFETCH(QString, reply);
    QFETCH(QString, filters);
    QFETCH(QList<int>, expectedIds);

    addGetAccountsMethod(QString("ret = (%1, [])").arg(reply));

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
    addGetAccountsMethod("ret = ([], [])");
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel { applicationId: \"foo\" }",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QTRY_COMPARE(object->property("ready").toBool(), true);

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
    QCOMPARE(countChanged.count(), 1);
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
    authenticationData.insert("AccessToken", "GoOn");
    QTest::newRow("access granted") <<
        "ret = ((1, {'displayName': 'Bob', 'serviceId': 'bar'}),{'AccessToken':'GoOn'})" <<
        accessReply <<
        authenticationData;
}

void ModuleTest::testModelRequestAccess()
{
    QFETCH(QString, reply);
    QFETCH(QVariantMap, expectedAccessReply);
    QFETCH(QVariantMap, expectedAuthenticationData);

    addGetAccountsMethod("ret = ([(1, {'displayName': 'Tom', 'serviceId': 'Foo' })], [])");
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

    addGetAccountsMethod(QString("ret = ([(1, {"
                                 "  'displayName': 'Bob',"
                                 "  'serviceId': 'MyService0',"
                                 "  'authMethod': %1,"
                                 "})], [])").arg(authMethod));
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

void ModuleTest::testInitialization_data()
{
    QTest::addColumn<QString>("appId");
    QTest::addColumn<bool>("errorExpected");

    QTest::newRow("empty APP_ID") <<
        "" <<
        true;

    QTest::newRow("invalid APP_ID") <<
        "" <<
        true;

    QTest::newRow("valid APP_ID") <<
        "my.package_app_0.2" <<
        false;
}

void ModuleTest::testInitialization()
{
    QFETCH(QString, appId);
    QFETCH(bool, errorExpected);

    qputenv("APP_ID", appId.toUtf8());

    if (errorExpected) {
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression("Ubuntu.OnlineAccounts:.*"));
    }

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 2.0\n"
                      "AccountModel {}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);

    if (!errorExpected) {
        /* We just want to check that invoking this method won't cause a crash */
        QString serviceId = "bar";
        QVariantMap params;
        bool ok = QMetaObject::invokeMethod(object, "requestAccess",
                                            Q_ARG(QString, serviceId),
                                            Q_ARG(QVariantMap, params));
        QVERIFY(ok);
    }
    delete object;
}

QTEST_MAIN(ModuleTest)
#include "tst_qml_module.moc"
