
#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <QList>
#include <QObject>
#include <QVariantMap>
#include <QDBusContext>

struct AccountData {
    quint32 accountId;
    QVariantMap details;
};

class Manager : public QObject, protected QDBusContext {
    Q_OBJECT
    struct Private;
public:
    enum ChangeType {
      Enabled = 0,
      Disabled,
      Deleted,
      Changed,
    };

    explicit Manager(QObject *parent=nullptr);
    ~Manager();

public Q_SLOTS:
    QList<AccountData> GetAccounts(const QVariantMap &filters);
    QVariantMap Authenticate(quint32 accountId, const QString &serviceId,
                             bool interactive, bool invalidate,
                             const QVariantMap &parameters);
    AccountData RequestAccess(const QString &applicationId,
                              const QString &serviceId);

Q_SIGNALS:
    void AccountChanged(uint account_id, ChangeType changeType);

private:
    bool canAccess(const QString &service_id);
    bool checkAccess(const QString &service_id);
    QString getPeerSecurityContext();
    std::unique_ptr<Private> p;
};

#endif
