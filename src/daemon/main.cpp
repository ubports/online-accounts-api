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
#include "inactivity_timer.h"
#include "manager.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    auto manager = new OnlineAccountsDaemon::Manager();

    auto inactivityTimer = new OnlineAccountsDaemon::InactivityTimer(5000);
    inactivityTimer->watchObject(manager);
    QObject::connect(inactivityTimer, SIGNAL(timeout()), &app, SLOT(quit()));

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject("/com/ubuntu/OnlineAccounts/Manager", manager);
    bus.registerService("com.ubuntu.OnlineAccounts.Manager");

    int ret = app.exec();

    bus.unregisterService("com.ubuntu.OnlineAccounts.Manager");
    bus.unregisterObject("/com/ubuntu/OnlineAccounts/Manager");

    delete inactivityTimer;
    delete manager;

    return ret;
}
