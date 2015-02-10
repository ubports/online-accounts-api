
#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <QList>
#include <QObject>
#include <QVariantMap>
#include <QDBusArgument>
#include <QDBusContext>

struct AccountInfo {
    uint account_id = 0;
    QVariantMap details;

    AccountInfo() {}
    AccountInfo(uint account_id, const QVariantMap &details)
        : account_id(account_id), details(details) {}

};
Q_DECLARE_METATYPE(AccountInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, AccountInfo &info);

class Manager : public QObject, protected QDBusContext {
    Q_OBJECT
    struct Private;
public:
    explicit Manager(QObject *parent=nullptr);
    ~Manager();

public Q_SLOTS:
    QList<AccountInfo> GetAccounts(const QStringList &service_ids);
    AccountInfo GetAccountInfo(const QString &service_id, uint account_id);
    QVariantMap Authenticate(const QString &service_id, uint account_id, bool interactive, bool invalidate);
    AccountInfo Register(const QString &service_id, QVariantMap &credentials);

Q_SIGNALS:
    void AccountChanged(const QString &service_id, uint account_id, bool enabled);
    void CredentialsChanged(const QString &service_id, uint account_id);

private:
    bool canAccess(const QString &service_id);
    bool checkAccess(const QString &service_id);
    QString getPeerSecurityContext();
    std::unique_ptr<Private> p;
};

#endif
