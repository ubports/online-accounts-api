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

#ifndef ONLINE_ACCOUNTS_DAEMON_AUTHENTICATOR_H
#define ONLINE_ACCOUNTS_DAEMON_AUTHENTICATOR_H

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace Accounts {
class AuthData;
}

namespace OnlineAccountsDaemon {

class AuthenticatorPrivate;
class Authenticator: public QObject
{
    Q_OBJECT

public:
    explicit Authenticator(QObject *parent = 0);
    ~Authenticator();

    void setInteractive(bool interactive);
    void invalidateCache();

    void authenticate(const Accounts::AuthData &authData,
                      const QVariantMap &parameters);

    bool isError() const { return !errorName().isEmpty(); }
    QVariantMap reply() const;
    QString errorName() const;
    QString errorMessage() const;

Q_SIGNALS:
    void finished();

private:
    Q_DECLARE_PRIVATE(Authenticator)
    AuthenticatorPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_AUTHENTICATOR_H
