/*
 * This file is part of OnlineAccountsDaemon
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

#include <QCoreApplication>
#include <QDBusConnection>
#include <QProcessEnvironment>
#include "inactivity_timer.h"
#include "OnlineAccountsDaemon/Manager"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();

    /* default daemonTimeout to 5 seconds */
    int daemonTimeout = 5;

    /* override daemonTimeout if OAU_DAEMON_TIMEOUT is set */
    if (environment.contains(QLatin1String("OAD_TIMEOUT"))) {
        bool isOk;
        int value = environment.value(
            QLatin1String("OAD_TIMEOUT")).toInt(&isOk);
        if (isOk)
            daemonTimeout = value;
    }

    auto manager = new OnlineAccountsDaemon::Manager();

    auto inactivityTimer =
        new OnlineAccountsDaemon::InactivityTimer(daemonTimeout * 1000);
    inactivityTimer->watchObject(manager);
    QObject::connect(inactivityTimer, SIGNAL(timeout()), &app, SLOT(quit()));

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject("/com/ubuntu/OnlineAccounts/Manager", manager);
    bus.registerService("com.ubuntu.OnlineAccounts.Manager");
    bus.connect(QString(),
                QStringLiteral("/org/freedesktop/DBus/Local"),
                QStringLiteral("org.freedesktop.DBus.Local"),
                QStringLiteral("Disconnected"),
                manager, SLOT(onDisconnected()));

    int ret = app.exec();

    bus.unregisterService("com.ubuntu.OnlineAccounts.Manager");
    bus.unregisterObject("/com/ubuntu/OnlineAccounts/Manager");

    delete inactivityTimer;
    delete manager;

    return ret;
}
