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

private Q_SLOTS:
    void testManagerReady_data();
    void testManagerReady();
    void testManagerAvailableAccounts_data();
    void testManagerAvailableAccounts();

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

QTEST_MAIN(FunctionalTests)
#include "functional_tests.moc"
