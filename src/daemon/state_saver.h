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

#include <QHash>
#include <QObject>
#include <QString>

namespace OnlineAccountsDaemon {

struct AccountRef {
    int accountId;
    QString serviceName;
};

typedef QHash<QString,AccountRef> ClientAccountRefs;

class StateSaverPrivate;
class StateSaver: public QObject
{
    Q_OBJECT

public:
    explicit StateSaver(QObject *parent = 0);
    ~StateSaver();

    void save(const ClientAccountRefs &refs);
    ClientAccountRefs load();

private:
    Q_DECLARE_PRIVATE(StateSaver)
    StateSaverPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_DAEMON_STATE_SAVER_H
