/*
 * This file is part of libOnlineAccounts
 *
 * Copyright (C) 2016 Canonical Ltd.
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

#include "service.h"

#include <QVariantMap>
#include "OnlineAccountsDaemon/dbus_constants.h"

using namespace OnlineAccounts;

Service::ServiceData::ServiceData(const QVariantMap &map):
    m_id(map.value(ONLINE_ACCOUNTS_INFO_KEY_SERVICE_ID).toString()),
    m_displayName(map.value(ONLINE_ACCOUNTS_INFO_KEY_DISPLAY_NAME).toString()),
    m_translations(map.value(ONLINE_ACCOUNTS_INFO_KEY_TRANSLATIONS).toString())
{
}

Service::Service():
    d(new ServiceData(QVariantMap()))
{
}

Service::Service(const QVariantMap &map):
    d(new ServiceData(map))
{
}
