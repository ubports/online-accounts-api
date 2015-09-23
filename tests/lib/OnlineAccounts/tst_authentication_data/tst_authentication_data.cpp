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

#include <QObject>
#include <QTest>
#include "authentication_data.h"

using namespace OnlineAccounts;

class AuthenticationDataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testData();
};

void AuthenticationDataTest::testData()
{
    OAuth2Data oauth2;

    oauth2.setClientId("client");
    QCOMPARE(oauth2.clientId(), QByteArray("client"));

    oauth2.setInteractive(false);
    QCOMPARE(oauth2.interactive(), false);

    OAuth2Data copy = oauth2;
    QCOMPARE(copy.clientId(), QByteArray("client"));
    QCOMPARE(copy.interactive(), false);

    oauth2.setClientId("client-changed");
    QCOMPARE(oauth2.clientId(), QByteArray("client-changed"));
    QCOMPARE(copy.clientId(), QByteArray("client"));

    /* As dictionary */
    QVariantMap expectedParameters;
    expectedParameters.insert("ClientId", "client-changed");
    QCOMPARE(oauth2.parameters(), expectedParameters);

    QVariantMap parameters;
    parameters.insert("UnknownKey", "some value");
    parameters.insert("Hello", "World");
    oauth2.setParameters(parameters);
    QCOMPARE(oauth2.parameters(), parameters);
}

QTEST_MAIN(AuthenticationDataTest)
#include "tst_authentication_data.moc"
