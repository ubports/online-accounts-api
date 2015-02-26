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

#include "account_p.h"

using namespace OnlineAccounts;

AccountPrivate::AccountPrivate(Manager *manager, const AccountInfo &info):
    m_manager(manager),
    m_info(info),
    q_ptr(0)
{
}

AccountPrivate::~AccountPrivate()
{
}

Account::Account(AccountPrivate *priv, QObject *parent):
    QObject(parent),
    d_ptr(priv)
{
    priv->q_ptr = this;
}

Account::~Account()
{
    delete d_ptr;
    d_ptr = 0;
}
