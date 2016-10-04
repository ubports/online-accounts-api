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

#include "OnlineAccountsDaemon/dbus_constants.h"
#include "daemon_interface.h"
#include "fake_dbus_apparmor.h"
#include "fake_online_accounts_service.h"
#include "fake_signond.h"
#include <Accounts/Account>
#include <Accounts/Manager>
#include <Accounts/Service>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QSignalSpy>
#include <QTest>
#include <libqtdbusmock/DBusMock.h>
#include <sys/types.h>
#include <unistd.h>

namespace QTest {
template<>
char *toString(const QSet<int> &set)
{
    QByteArray ba = "QSet<int>(";
    QStringList list;
    Q_FOREACH(int i, set) {
        list.append(QString::number(i));
    }
    ba += list.join(", ");
    ba += ")";
    return qstrdup(ba.data());
}

template<>
char *toString(const QVariantMap &map)
{
    QJsonDocument doc(QJsonObject::fromVariantMap(map));
    return qstrdup(doc.toJson(QJsonDocument::Compact).data());
}

} // QTest namespace

class TestProcess: public QProcess
{
    Q_OBJECT

public:
    TestProcess(QObject *parent = 0):
        QProcess(parent),
        m_replyExpected(false)
    {
        setProgram(QStringLiteral(TEST_PROCESS));
        setReadChannel(QProcess::StandardOutput);
        setProcessChannelMode(QProcess::ForwardedErrorChannel);
        start();
        QVERIFY(waitForStarted());
        QVERIFY(waitForReadyRead());
        m_uniqueName = QString::fromUtf8(readLine()).trimmed();

        QObject::connect(this, SIGNAL(readyReadStandardOutput()),
                         this, SLOT(onReadyRead()));
    }

    ~TestProcess() { quit(); }

    QString uniqueName() const { return m_uniqueName; }

    void quit() { write("\n"); waitForFinished(); }

    QList<AccountInfo> getAccounts(const QVariantMap &filters) {
        QJsonDocument doc(QJsonObject::fromVariantMap(filters));
        m_replyExpected = true;
        write("GetAccounts -f ");
        write(doc.toJson(QJsonDocument::Compact) + '\n');

        waitForReadyRead();
        doc = QJsonDocument::fromJson(readLine());
        m_replyExpected = false;
        QList<AccountInfo> accountInfos;
        Q_FOREACH(const QJsonValue &v, doc.array()) {
            QJsonArray a = v.toArray();
            accountInfos.append(AccountInfo(a.at(0).toInt(),
                                            a.at(1).toObject().toVariantMap()));
        }
        return accountInfos;
    }

private Q_SLOTS:
    void onReadyRead() {
        if (m_replyExpected) return;
        QByteArray line = readLine().trimmed();
        QList<QByteArray> parts = line.split(' ');
        if (parts[0] != "AccountChanged") return;

        QByteArray changes = parts.mid(2).join(' ');
        QJsonDocument doc = QJsonDocument::fromJson(changes);
        QJsonArray a = doc.array();

        Q_EMIT accountChanged(QString::fromUtf8(parts[1]),
                              AccountInfo(a.at(0).toInt(),
                                          a.at(1).toObject().toVariantMap()));
    }

Q_SIGNALS:
    void accountChanged(QString serviceId, AccountInfo account);

private:
    QString m_uniqueName;
    bool m_replyExpected;
};

class DBusService: public QtDBusTest::DBusTestRunner
{
public:
    DBusService();

    FakeDBusApparmor &dbusApparmor() { return m_dbusApparmor; }
    FakeOnlineAccountsService &onlineAccounts() { return m_onlineAccounts; }
    FakeSignond &signond() { return m_signond; }

private:
    QtDBusMock::DBusMock m_mock;
    FakeDBusApparmor m_dbusApparmor;
    FakeOnlineAccountsService m_onlineAccounts;
    FakeSignond m_signond;
};

DBusService::DBusService():
    QtDBusTest::DBusTestRunner(TEST_DBUS_CONFIG_FILE),
    m_mock(*this),
    m_dbusApparmor(&m_mock),
    m_onlineAccounts(&m_mock),
    m_signond(&m_mock)
{
}

class FunctionalTests: public QObject
{
    Q_OBJECT

    struct EnvSetup {
        EnvSetup();
    };

public:
    FunctionalTests();

private Q_SLOTS:
    void init();
    void cleanup();
    void testGetAccountsFiltering_data();
    void testGetAccountsFiltering();
    void testAuthenticate_data();
    void testAuthenticate();
    void testRequestAccess_data();
    void testRequestAccess();
    void testAccountChanges();
    void testLifetime();

private:
    void clearDb();

private:
    EnvSetup m_env;
    DBusService *m_dbus;
    int m_firstAccountId;
    int m_account3CredentialsId;
};

FunctionalTests::EnvSetup::EnvSetup() {
    qputenv("ACCOUNTS", "/tmp/");
    qputenv("AG_APPLICATIONS", TEST_DATA_DIR);
    qputenv("AG_SERVICES", TEST_DATA_DIR);
    qputenv("AG_SERVICE_TYPES", TEST_DATA_DIR);
    qputenv("AG_PROVIDERS", TEST_DATA_DIR);
    qputenv("XDG_DATA_HOME", TEST_DATA_DIR);

    qputenv("SSO_USE_PEER_BUS", "0");

    qputenv("OAD_TIMEOUT", "30");
    qputenv("OAD_TESTING", "1");
}

FunctionalTests::FunctionalTests():
    QObject(),
    m_dbus(0),
    m_account3CredentialsId(35)
{
    clearDb();

    /* Populate the accounts DB */
    Accounts::Manager *manager = new Accounts::Manager(Accounts::Manager::DisableNotifications, this);
    Accounts::Service coolMail = manager->service("coolmail");
    Accounts::Service coolShare = manager->service("com.ubuntu.tests_coolshare");
    Accounts::Service oauth1auth = manager->service("oauth1auth");
    Accounts::Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);
    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount 1");
    // Do not create this identity, we want it to be non-existing
    account1->setCredentialsId(249);
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->syncAndBlock();
    m_firstAccountId = account1->id() - 1;

    Accounts::Account *account2 = manager->createAccount("cool");
    QVERIFY(account2 != 0);
    account2->setEnabled(true);
    account2->setDisplayName("CoolAccount 2");
    account2->selectService(coolMail);
    account2->setEnabled(false);
    account2->selectService(coolShare);
    account2->setEnabled(true);
    account2->syncAndBlock();

    Accounts::Account *account3 = manager->createAccount("cool");
    QVERIFY(account3 != 0);
    account3->setEnabled(true);
    account3->setDisplayName("CoolAccount 3");
    account3->setValue("color", "red");
    account3->setValue("size", "big");
    account3->setCredentialsId(m_account3CredentialsId);
    account3->selectService(coolMail);
    account3->setEnabled(true);
    account3->selectService(oauth1auth);
    account3->setEnabled(true);
    account3->syncAndBlock();

    delete manager;
}

void FunctionalTests::clearDb()
{
    QDir dbroot(QString::fromLatin1(qgetenv("ACCOUNTS")));
    dbroot.remove("accounts.db");
}

void FunctionalTests::init()
{
    m_dbus = new DBusService();
    m_dbus->startServices();

    /* Uncomment next line to debug DBus calls */
    //QProcess::startDetached("/usr/bin/dbus-monitor");
}

void FunctionalTests::cleanup()
{
    delete m_dbus;
}

void FunctionalTests::testGetAccountsFiltering_data()
{
    QTest::addColumn<QVariantMap>("filters");
    QTest::addColumn<QString>("securityContext");
    QTest::addColumn<QList<int> >("expectedAccountIds");

    QVariantMap filters;
    QTest::newRow("empty filters") <<
        filters <<
        "unconfined" <<
        (QList<int>() << 1 << 2 << 3);

    QTest::newRow("empty filters, confined") <<
        filters <<
        "com.ubuntu.tests_application_0.2" <<
        (QList<int>() << 2);

    filters[ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID] = "coolmail";
    QTest::newRow("by service ID") <<
        filters <<
        "unconfined" <<
        (QList<int>() << 1 << 3);
}

void FunctionalTests::testGetAccountsFiltering()
{
    QFETCH(QVariantMap, filters);
    QFETCH(QString, securityContext);
    QFETCH(QList<int>, expectedAccountIds);

    DaemonInterface *daemon = new DaemonInterface(m_dbus->sessionConnection());

    TestProcess testProcess;
    QVariantMap credentials {
        { "LinuxSecurityLabel", securityContext.toUtf8() },
    };
    m_dbus->dbusApparmor().setCredentials(testProcess.uniqueName(), credentials);

    QList<AccountInfo> accountInfos = testProcess.getAccounts(filters);
    QList<int> accountIds;
    Q_FOREACH(const AccountInfo &info, accountInfos) {
        accountIds.append(info.id() + m_firstAccountId);
    }
    QCOMPARE(accountIds.toSet(), expectedAccountIds.toSet());

    delete daemon;
}

void FunctionalTests::testAuthenticate_data()
{
    QTest::addColumn<int>("accountId");
    QTest::addColumn<QString>("serviceId");
    QTest::addColumn<bool>("interactive");
    QTest::addColumn<bool>("invalidate");
    QTest::addColumn<QVariantMap>("authParams");
    QTest::addColumn<QVariantMap>("expectedCredentials");
    QTest::addColumn<QString>("errorName");

    QVariantMap authParams;
    QVariantMap credentials;
    QTest::newRow("invalid account ID") <<
        12412341 <<
        "coolmail" <<
        false << false <<
        authParams <<
        credentials <<
        ONLINE_ACCOUNTS_ERROR_PERMISSION_DENIED;

    authParams["errorName"] =
        "com.google.code.AccountsSSO.SingleSignOn.Error.Network";
    QTest::newRow("Authentication error") <<
        3 <<
        "coolmail" <<
        false << false <<
        authParams <<
        credentials <<
        "com.ubuntu.OnlineAccounts.Error.Network";
    authParams.clear();

    authParams["one"] = 1;
    credentials["one"] = 1;
    credentials["host"] = "coolmail.ex";
    credentials["UiPolicy"] = 2;
    QTest::newRow("no interactive, no invalidate") <<
        3 <<
        "coolmail" <<
        false << false <<
        authParams <<
        credentials <<
        QString();

    credentials["UiPolicy"] = 0;
    QTest::newRow("interactive, no invalidate") <<
        3 <<
        "coolmail" <<
        true << false <<
        authParams <<
        credentials <<
        QString();

    credentials["ForceTokenRefresh"] = true;
    QTest::newRow("interactive, invalidate") <<
        3 <<
        "coolmail" <<
        true << true <<
        authParams <<
        credentials <<
        QString();

    authParams.clear();
    credentials.clear();
    credentials["UiPolicy"] = 0;
    credentials["ConsumerKey"] = "c0nsum3rk3y";
    credentials["ConsumerSecret"] = "c0nsum3rs3cr3t";
    QTest::newRow("OAuth1 client data") <<
        3 <<
        "oauth1auth" <<
        true << false <<
        authParams <<
        credentials <<
        QString();

    authParams.clear();
    authParams["ConsumerKey"] = "overridden";
    credentials.clear();
    credentials["UiPolicy"] = 0;
    credentials["ConsumerKey"] = "overridden";
    credentials["ConsumerSecret"] = "c0nsum3rs3cr3t";
    QTest::newRow("OAuth1 client data, overridden") <<
        3 <<
        "oauth1auth" <<
        true << false <<
        authParams <<
        credentials <<
        QString();
}

void FunctionalTests::testAuthenticate()
{
    QFETCH(int, accountId);
    QFETCH(QString, serviceId);
    QFETCH(bool, interactive);
    QFETCH(bool, invalidate);
    QFETCH(QVariantMap, authParams);
    QFETCH(QVariantMap, expectedCredentials);
    QFETCH(QString, errorName);

    m_dbus->signond().addIdentity(m_account3CredentialsId, QVariantMap());

    DaemonInterface *daemon = new DaemonInterface(m_dbus->sessionConnection());

    QDBusPendingReply<QVariantMap> reply =
        daemon->authenticate(m_firstAccountId + accountId, serviceId,
                             interactive, invalidate, authParams);
    reply.waitForFinished();

    if (errorName.isEmpty()) {
        QVERIFY2(!reply.isError(), reply.error().message().toUtf8().constData());
        QVariantMap credentials = reply.argumentAt<0>();
        // Add the requestor PID
        expectedCredentials["requestorPid"] = getpid();
        QCOMPARE(credentials, expectedCredentials);
    } else {
        QVERIFY(reply.isError());
        QCOMPARE(reply.error().name(), errorName);
    }

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

    accessReply["accountId"] = m_firstAccountId + 3;
    accountInfo[ONLINE_ACCOUNTS_INFO_KEY_AUTH_METHOD] =
        ONLINE_ACCOUNTS_AUTH_METHOD_OAUTH2;
    accountInfo[ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME] = "CoolAccount 3";
    accountInfo[ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID] = "coolmail";
    accountInfo["settings/auth/mechanism"] = "user_agent";
    accountInfo["settings/auth/method"] = "oauth2";
    accountInfo["settings/auth/oauth2/user_agent/host"] = "coolmail.ex";
    accountInfo["settings/auto-explode-after"] = 10;
    accountInfo["settings/color"] = "green";
    accountInfo["settings/size"] = "big";
    credentials["host"] = "coolmail.ex";
    QTest::newRow("no auth params") <<
        "coolmail" <<
        authParams <<
        accessReply <<
        m_firstAccountId + 3 <<
        accountInfo <<
        credentials <<
        "";

    authParams["one"] = 1;
    credentials["one"] = 1;
    QTest::newRow("with auth params") <<
        "coolmail" <<
        authParams <<
        accessReply <<
        m_firstAccountId + 3 <<
        accountInfo <<
        credentials <<
        "";
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

    m_dbus->onlineAccounts().setRequestAccessReply(accessReply);
    m_dbus->signond().addIdentity(m_account3CredentialsId, QVariantMap());

    DaemonInterface *daemon = new DaemonInterface(m_dbus->sessionConnection());

    QDBusPendingReply<AccountInfo,QVariantMap> reply =
        daemon->requestAccess(serviceId, authParams);
    reply.waitForFinished();

    if (errorName.isEmpty()) {
        QVERIFY2(!reply.isError(), reply.error().message().toUtf8().constData());
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

void FunctionalTests::testAccountChanges()
{
    DaemonInterface *daemon = new DaemonInterface(m_dbus->sessionConnection());

    /* First, we make a call to the service so that it knows about our client
     * and will later notify it about changes.
     */
    QVariantMap filters;
    filters["applicationId"] = "com.ubuntu.tests_application";
    TestProcess testProcess;
    QSignalSpy accountChanged(&testProcess,
                              SIGNAL(accountChanged(QString,AccountInfo)));
    QList<AccountInfo> accountInfos = testProcess.getAccounts(filters);
    QList<int> initialAccountIds;
    Q_FOREACH(const AccountInfo &info, accountInfos) {
        initialAccountIds.append(info.id());
    }

    /* Create a new account */
    Accounts::Manager *manager = new Accounts::Manager(this);
    Accounts::Service coolShare = manager->service("com.ubuntu.tests_coolshare");
    Accounts::Account *account = manager->createAccount("cool");
    QVERIFY(account != 0);
    account->setEnabled(true);
    account->setDisplayName("New account");
    account->selectService(coolShare);
    account->setEnabled(true);
    account->syncAndBlock();

    QTRY_COMPARE(accountChanged.count(), 1);
    QString serviceId = accountChanged.at(0).at(0).toString();
    AccountInfo accountInfo = accountChanged.at(0).at(1).value<AccountInfo>();

    QCOMPARE(serviceId, coolShare.name());
    QCOMPARE(accountInfo.id(), account->id());
    QVariantMap expectedAccountInfo;
    expectedAccountInfo["authMethod"] = ONLINE_ACCOUNTS_AUTH_METHOD_UNKNOWN;
    expectedAccountInfo["changeType"] = ONLINE_ACCOUNTS_INFO_CHANGE_ENABLED;
    expectedAccountInfo["displayName"] = "New account";
    expectedAccountInfo["serviceId"] = "com.ubuntu.tests_coolshare";
    QCOMPARE(accountInfo.data(), expectedAccountInfo);

    /* Change a setting */
    accountChanged.clear();
    account->setValue("color", "blue");
    account->syncAndBlock();

    QTRY_COMPARE(accountChanged.count(), 1);
    serviceId = accountChanged.at(0).at(0).toString();
    accountInfo = accountChanged.at(0).at(1).value<AccountInfo>();

    QCOMPARE(serviceId, coolShare.name());
    QCOMPARE(accountInfo.id(), account->id());
    expectedAccountInfo["authMethod"] = ONLINE_ACCOUNTS_AUTH_METHOD_UNKNOWN;
    expectedAccountInfo["changeType"] = ONLINE_ACCOUNTS_INFO_CHANGE_UPDATED;
    expectedAccountInfo["settings/color"] = "blue";
    expectedAccountInfo["displayName"] = "New account";
    expectedAccountInfo["serviceId"] = "com.ubuntu.tests_coolshare";
    QCOMPARE(accountInfo.data(), expectedAccountInfo);

    /* Delete the account */
    accountChanged.clear();
    account->remove();
    account->syncAndBlock();

    QTRY_COMPARE(accountChanged.count(), 1);
    serviceId = accountChanged.at(0).at(0).toString();
    accountInfo = accountChanged.at(0).at(1).value<AccountInfo>();

    QCOMPARE(serviceId, coolShare.name());
    QCOMPARE(accountInfo.id(), account->id());
    expectedAccountInfo["authMethod"] = ONLINE_ACCOUNTS_AUTH_METHOD_UNKNOWN;
    expectedAccountInfo["changeType"] = ONLINE_ACCOUNTS_INFO_CHANGE_DISABLED;
    expectedAccountInfo["displayName"] = "New account";
    expectedAccountInfo["serviceId"] = "com.ubuntu.tests_coolshare";
    QCOMPARE(accountInfo.data(), expectedAccountInfo);

    delete manager;
    delete daemon;
}

void FunctionalTests::testLifetime()
{
    /* Destroy the D-Bus daemon, and create one with the OAD_TIMEOUT variable
     * set to a lower value, to make this test meaningful */
    delete m_dbus;

    qputenv("OAD_TIMEOUT", "2");

    m_dbus = new DBusService();
    m_dbus->startServices();

    /* Make a dbus call, and have signond reply after 3 seconds; make sure that
     * the online accounts daemon doesn't time out. */
    m_dbus->signond().addIdentity(m_account3CredentialsId, QVariantMap());

    DaemonInterface *daemon = new DaemonInterface(m_dbus->sessionConnection());

    QDBusServiceWatcher watcher(ONLINE_ACCOUNTS_MANAGER_SERVICE_NAME,
                                m_dbus->sessionConnection(),
                                QDBusServiceWatcher::WatchForUnregistration);
    QSignalSpy unregistered(&watcher,
                            SIGNAL(serviceUnregistered(const QString &)));

    QVariantMap authParams;
    authParams["delay"] = 3;
    QDBusPendingReply<QVariantMap> reply =
        daemon->authenticate(m_firstAccountId + 3, "coolmail",
                             false, false, authParams);
    reply.waitForFinished();

    QVERIFY2(!reply.isError(), reply.error().message().toUtf8().constData());
    QVariantMap expectedCredentials(authParams);
    expectedCredentials["UiPolicy"] = 2;
    expectedCredentials["host"] = "coolmail.ex";
    expectedCredentials["requestorPid"] = getpid();
    QVariantMap credentials = reply.argumentAt<0>();
    QCOMPARE(credentials, expectedCredentials);

    QCOMPARE(unregistered.count(), 0);

    delete daemon;

    /* We expect the OA service to exit within a couple of seconds */
    unregistered.wait();
}

QTEST_MAIN(FunctionalTests)
#include "functional_tests.moc"
