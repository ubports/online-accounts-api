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

QTEST_MAIN(MockTemplateTests)
#include "mock_template.moc"
