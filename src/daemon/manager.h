#ifndef MANAGER_H
#define MANAGER_H

#include <QDBusArgument>
#include <QDBusContext>
#include <QList>
#include <QObject>
#include <QVariantMap>

struct AccountInfo {
    uint accountId;
    QVariantMap details;

    AccountInfo(): accountId(0) {}
    AccountInfo(uint accountId, const QVariantMap &details):
        accountId(accountId), details(details) {}
};
Q_DECLARE_METATYPE(AccountInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                AccountInfo &info);

class ManagerPrivate;
class Manager: public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit Manager(QObject *parent = 0);
    ~Manager();

public Q_SLOTS:
    QList<AccountInfo> GetAccounts(const QVariantMap &filters);
    QVariantMap Authenticate(uint accountId, const QString &serviceId,
                             bool interactive, bool invalidate,
                             const QVariantMap &parameters);
    AccountInfo RequestAccess(const QString &serviceId,
                              const QVariantMap &parameters,
                              QVariantMap &credentials);

Q_SIGNALS:
    void AccountChanged(const QString &serviceId, AccountInfo accountInfo);

private:
    bool canAccess(const QString &serviceId);
    bool checkAccess(const QString &serviceId);
    QString getPeerSecurityContext();

    Q_DECLARE_PRIVATE(Manager)
    ManagerPrivate *d_ptr;
};

#endif
