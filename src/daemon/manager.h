
#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <QList>
#include <QObject>
#include <QVariantMap>
#include <QDBusContext>

class Manager : public QObject, protected QDBusContext {
    Q_OBJECT
    struct Private;
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

private:
    bool checkAccess(const QString &service_id);
    QString getPeerSecurityContext();
    std::unique_ptr<Private> p;
};

#endif
