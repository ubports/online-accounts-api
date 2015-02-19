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

#ifndef ONLINE_ACCOUNTS_ERROR_H
#define ONLINE_ACCOUNTS_ERROR_H

#include <QString>

#include "global.h"

namespace OnlineAccounts {

class ONLINE_ACCOUNTS_EXPORT Error
{
public:
    enum Code {
        NoError = 0,
        NoAccount, /* Account got removed or disabled */
        WrongType, /* For instance, using a PasswordReply to handle an OAuth
                      reply */
        UserCanceled, /* The user dismissed the authentication prompt */
        PermissionDenied,
        InteractionRequired,
    };

    Error(): m_code(NoError) {}
    Error(Code code, const QString &text): m_code(code), m_text(text) {}

    Code code() const { return m_code; }
    QString text() const { return m_text; }

private:
    Code m_code;
    QString m_text;
};

} // namespace

#endif // ONLINE_ACCOUNTS_ERROR_H
