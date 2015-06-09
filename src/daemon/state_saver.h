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

#ifndef ONLINE_ACCOUNTS_DAEMON_STATE_SAVER_H
#define ONLINE_ACCOUNTS_DAEMON_STATE_SAVER_H

#include <QObject>
#include <QStringList>
#include "account_info.h"

namespace OnlineAccountsDaemon {

typedef QPair<QString,QString> Client;

class StateSaverPrivate;
class StateSaver: public QObject
{
    Q_OBJECT

public:
    explicit StateSaver(QObject *parent = 0);
    ~StateSaver();

    void setAccounts(const QList<AccountInfo> &accounts);
    QList<AccountInfo> accounts() const;

    void setClients(const QList<Client> &clients);
    QList<Client> clients() const;

private:
    Q_DECLARE_PRIVATE(StateSaver)
    StateSaverPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_STATE_SAVER_H
