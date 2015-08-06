/*
 * This file is part of OnlineAccountsModule
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

#ifndef ONLINE_ACCOUNTS_MODULE_ACCOUNT_MODEL_H
#define ONLINE_ACCOUNTS_MODULE_ACCOUNT_MODEL_H

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QString>
#include <QVariant>

namespace OnlineAccountsModule {

class Account;

class AccountModelPrivate;
class AccountModel: public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString applicationId READ applicationId \
               WRITE setApplicationId NOTIFY applicationIdChanged)
    Q_PROPERTY(QString serviceId READ serviceId \
               WRITE setServiceId NOTIFY serviceIdChanged)
    Q_PROPERTY(QList<QObject*> accountList READ accountList \
               NOTIFY accountListChanged)

public:
    enum Roles {
        DisplayNameRole = Qt::UserRole + 1,
        ValidRole,
        AccountIdRole,
        ServiceIdRole,
        AuthenticationMethodRole,
        SettingsRole,
        AccountRole,
    };

    explicit AccountModel(QObject *parent = 0);
    ~AccountModel();

    void setApplicationId(const QString &applicationId);
    QString applicationId() const;

    void setServiceId(const QString &serviceId);
    QString serviceId() const;

    QList<QObject*> accountList() const;

    Q_INVOKABLE void requestAccess(const QString &service,
                                   const QVariantMap &params);

    // reimplemented virtual methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void countChanged();
    void applicationIdChanged();
    void serviceIdChanged();
    void accountListChanged();
    void accessReply(const QVariantMap &reply,
                     const QVariantMap &authenticationData);

private:
    Q_DECLARE_PRIVATE(AccountModel)
    AccountModelPrivate *d_ptr;
};

} // namespace

#endif // ONLINE_ACCOUNTS_MODULE_ACCOUNT_MODEL_H
