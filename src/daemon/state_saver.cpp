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

#include "state_saver.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

class StateSaverPrivate {
public:
    StateSaverPrivate();

    QJsonValue accountRefToJson(const AccountRef &ref);
    AccountRef accountRefFromJson(const QJsonValue &value);

private:
    friend class StateSaver;
    QString m_cacheFile;
};

} // namespace

StateSaverPrivate::StateSaverPrivate()
{
    QString cachePath =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cacheFile = cachePath + "/client_account_refs.json";
}

QJsonValue StateSaverPrivate::accountRefToJson(const AccountRef &ref)
{
    QJsonObject jsonObject;
    jsonObject.insert(QStringLiteral("accountId"), ref.accountId);
    jsonObject.insert(QStringLiteral("serviceName"), ref.serviceName);
    return QJsonValue(jsonObject);
}

AccountRef StateSaverPrivate::accountRefFromJson(const QJsonValue &value)
{
    QJsonObject jsonObject = value.toObject();
    AccountRef ref;
    ref.accountId = jsonObject[QStringLiteral("accountId")].toInt();
    ref.serviceName = jsonObject[QStringLiteral("serviceName")].toString();
    return ref;
}

StateSaver::StateSaver(QObject *parent):
    QObject(parent),
    d_ptr(new StateSaverPrivate)
{
}

StateSaver::~StateSaver()
{
    delete d_ptr;
}

void StateSaver::save(const ClientAccountRefs &refs)
{
    Q_D(StateSaver);

    QJsonObject clients;
    for (ClientAccountRefs::const_iterator i = refs.constBegin();
         i != refs.constEnd(); i++) {
        clients.insert(i.key(), d->accountRefToJson(i.value()));
    }

    QFile file(d->m_cacheFile);
    if (Q_UNLIKELY(!file.open(QIODevice::WriteOnly | QIODevice::Text))) {
        qWarning() << "Couldn't save state to" << d->m_cacheFile;
        return;
    }

    QJsonDocument doc(clients);
    file.write(doc.toJson());
}

ClientAccountRefs StateSaver::load()
{
    Q_D(StateSaver);

    ClientAccountRefs refs;

    QFile file(d->m_cacheFile);
    if (Q_UNLIKELY(!file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        qWarning() << "Cannot open file" << d->m_cacheFile;
        return refs;
    }

    QByteArray contents = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(contents);
    if (doc.isEmpty() || !doc.isObject()) return refs;

    QJsonObject jsonObject = doc.object();
    for (QJsonObject::const_iterator i = jsonObject.constBegin();
         i != jsonObject.constEnd(); i++) {
        refs.insert(i.key(), d->accountRefFromJson(i.value()));
    }

    return refs;
}
