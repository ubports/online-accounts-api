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
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

using namespace OnlineAccountsDaemon;

namespace OnlineAccountsDaemon {

class StateSaverPrivate {
public:
    StateSaverPrivate();
    ~StateSaverPrivate();

    static QList<AccountInfo> accountsFromJson(const QJsonValue &value);
    static QJsonValue accountsToJson(const QList<AccountInfo> &accounts);

    static QList<Client> clientsFromJson(const QJsonValue &value);
    static QJsonValue clientsToJson(const QList<Client> &clients);

    void load();
    void save();

private:
    friend class StateSaver;
    QString m_cacheFile;
    QList<Client> m_clients;
    QList<AccountInfo> m_accounts;
};

} // namespace

StateSaverPrivate::StateSaverPrivate()
{
    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    m_cacheFile = cacheDir.filePath("client_account_refs.json");

    load();
}

StateSaverPrivate::~StateSaverPrivate()
{
    save();
}

QList<AccountInfo>
StateSaverPrivate::accountsFromJson(const QJsonValue &value)
{
    QList<AccountInfo> accounts;
    QJsonArray jsonAccounts = value.toArray();

    Q_FOREACH(const QJsonValue &jsonAccount, jsonAccounts) {
        QJsonObject jsonObject = jsonAccount.toObject();
        int accountId = jsonObject.value("accountId").toInt();
        QVariantMap details =
            jsonObject.value("details").toObject().toVariantMap();
        accounts.append(AccountInfo(uint(accountId), details));
    }
    return accounts;
}

QJsonValue
StateSaverPrivate::accountsToJson(const QList<AccountInfo> &accounts)
{
    Q_UNUSED(accounts);
    return QJsonValue(); // TODO
}

QList<Client> StateSaverPrivate::clientsFromJson(const QJsonValue &value)
{
    QList<Client> clients;
    Q_FOREACH(const QJsonValue &jsonClient, value.toArray()) {
        QJsonObject jsonObject = jsonClient.toObject();
        QString busName = jsonObject.value("busName").toString();
        QString applicationId = jsonObject.value("applicationId").toString();
        clients.append(Client(busName, applicationId));
    }
    return clients;
}

QJsonValue StateSaverPrivate::clientsToJson(const QList<Client> &clients)
{
    QJsonArray clientArray;
    Q_FOREACH(const Client &client, clients) {
        QJsonObject clientObject;
        clientObject.insert("busName", client.first);
        clientObject.insert("applicationId", client.second);
        clientArray.append(clientObject);
    }
    return QJsonValue(clientArray);
}

void StateSaverPrivate::save()
{
    QFile file(m_cacheFile);
    if (Q_UNLIKELY(!file.open(QIODevice::WriteOnly | QIODevice::Text))) {
        qWarning() << "Couldn't save state to" << m_cacheFile;
        return;
    }

    QJsonObject jsonObject;
    jsonObject.insert("accounts", accountsToJson(m_accounts));
    jsonObject.insert("clients", clientsToJson(m_clients));

    QJsonDocument doc(jsonObject);
    file.write(doc.toJson());
}

void StateSaverPrivate::load()
{
    QFile file(m_cacheFile);
    if (Q_UNLIKELY(!file.open(QIODevice::ReadOnly | QIODevice::Text))) {
        qWarning() << "Cannot open file" << m_cacheFile;
        return;
    }

    QByteArray contents = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(contents);
    if (doc.isEmpty() || !doc.isObject()) return;

    QJsonObject jsonObject = doc.object();
    m_accounts = accountsFromJson(jsonObject.value("accounts"));
    m_clients = clientsFromJson(jsonObject.value("clients"));
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

void StateSaver::setAccounts(const QList<AccountInfo> &accounts)
{
    Q_D(StateSaver);
    d->m_accounts = accounts;
}

QList<AccountInfo> StateSaver::accounts() const
{
    Q_D(const StateSaver);
    return d->m_accounts;
}

void StateSaver::setClients(const QList<Client> &clients)
{
    Q_D(StateSaver);
    d->m_clients = clients;
}

QList<Client> StateSaver::clients() const
{
    Q_D(const StateSaver);
    return d->m_clients;
}
