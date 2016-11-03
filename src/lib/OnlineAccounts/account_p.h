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

#ifndef ONLINE_ACCOUNTS_ACCOUNT_P_H
#define ONLINE_ACCOUNTS_ACCOUNT_P_H

#include "account.h"
#include "account_info.h"

namespace OnlineAccounts {

class AccountPrivate
{
    Q_DECLARE_PUBLIC(Account)

public:
    AccountPrivate(Manager *manager, const AccountInfo &info);
    ~AccountPrivate();

    void setInvalid();
    void update(const AccountInfo &info);

    Service service() const;

private:
    Manager *m_manager;
    AccountInfo m_info;
    bool m_isValid;
    mutable Account *q_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_ACCOUNT_P_H
