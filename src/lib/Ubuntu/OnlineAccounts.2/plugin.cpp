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

#include "plugin.h"

#include "account.h"
#include "account_model.h"

#include <QDebug>
#include <QQmlComponent>

using namespace OnlineAccountsModule;

void Plugin::registerTypes(const char* uri)
{
    qDebug() << Q_FUNC_INFO << uri;

    qmlRegisterType<AccountModel>(uri, 2, 0, "AccountModel");
    qmlRegisterUncreatableType<Account>(uri, 2, 0, "Account",
                                        "Cannot be created from QML");
    qmlRegisterUncreatableType<OnlineAccounts::Service>(uri, 2, 0, "Service",
                                                        "Cannot be created from QML");
}
