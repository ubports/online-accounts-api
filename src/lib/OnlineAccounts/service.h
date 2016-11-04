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

#ifndef ONLINE_ACCOUNTS_SERVICE_H
#define ONLINE_ACCOUNTS_SERVICE_H

#include <QObject>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QUrl>
#include <QVariantMap>

#include "global.h"

namespace OnlineAccounts {

class Manager;
class ManagerPrivate;

class ONLINE_ACCOUNTS_EXPORT Service
{
    Q_GADGET
    Q_PROPERTY(QString serviceId READ id CONSTANT)
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(QUrl iconSource READ iconSource CONSTANT)

private:
    class ServiceData: public QSharedData {
        ServiceData(const QVariantMap &map);
        friend class ManagerPrivate;
        friend class Service;
        QString m_id;
        QString m_displayName;
        QUrl m_iconSource;
    };

public:
    Service();
    ~Service() {}

    Service(const Service &other): d(other.d) {}

    bool isValid() const { return !d->m_id.isEmpty(); }
    QString id() const { return d->m_id; }
    QString displayName() const { return d->m_displayName; }
    QUrl iconSource() const { return d->m_iconSource; }

protected:
    Service(const QVariantMap &map);

private:
    friend class Manager;
    friend class ManagerPrivate;
    Service(ServiceData *d): d(d) {};
    QSharedDataPointer<ServiceData> d;
};

} // namespace

Q_DECLARE_METATYPE(OnlineAccounts::Service)

#endif // ONLINE_ACCOUNTS_SERVICE_H
