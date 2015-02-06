
#ifndef MANAGER_H
#define MANAGER_H

#include <QList>
#include <QObject>
#include <QVariantMap>

class Manager : public QObject {
    Q_OBJECT
public:
    explicit Manager(QObject *parent=nullptr);
    ~Manager();

public Q_SLOTS:
    QList<uint> GetAccounts(const QString &service_id);
    QVariantMap GetAccountInfo(const QString &service_id, uint account_id);
    QVariantMap Authenticate(const QString &service_id, uint account_id, bool interactive, bool invalidate);
    uint Register(const QString &service_id, QVariantMap &details, QVariantMap &credentials);

Q_SIGNALS:
    void AccountChanged(const QString &service_id, uint account_id, bool enabled);
    void CredentialsChanged(const QString &service_id, uint account_id);
};

#endif
