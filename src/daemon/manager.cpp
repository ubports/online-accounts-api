#include <QDBusMessage>
#include <QDebug>
#include "aacontext.h"
#include "manager.h"

static const char FORBIDDEN_ERROR[] = "com.ubuntu.OnlineAccounts.Error.Forbidden";

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info)
{
    argument.beginStructure();
    argument << info.accountId << info.details;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                AccountInfo &info)
{
    argument.beginStructure();
    argument >> info.accountId >> info.details;
    argument.endStructure();
    return argument;
}

class ManagerPrivate {
public:
    AppArmorContext m_apparmor;
};

Manager::Manager(QObject *parent):
    QObject(parent),
    d_ptr(new ManagerPrivate)
{
}

Manager::~Manager()
{
}

bool Manager::canAccess(const QString &serviceId)
{
    Q_D(Manager);

    QString context = d->m_apparmor.getPeerSecurityContext(connection(),
                                                           message());
    // Could not determine peer's AppArmor context, so deny access
    if (context.isEmpty()) {
        return false;
    }
    // Unconfined processes can access anything
    if (context == "unconfined") {
        return true;
    }

    // Try to extract the click package name from the AppArmor context.
    int pos = context.indexOf('_');
    if (pos < 0) {
        qWarning() << "AppArmor context doesn't contain package ID: " << context;
        return false;
    }
    QString pkgname = context.left(pos);

    // Do the same on the service ID: we are only dealing with
    // confined apps at this point, so only $pkgname prefixed
    // services are accessible.
    pos = serviceId.indexOf('_');
    if (pos < 0) {
        return false;
    }
    return serviceId.left(pos) == pkgname;
}

bool Manager::checkAccess(const QString &serviceId)
{
    bool hasAccess = canAccess(serviceId);
    if (!hasAccess) {
        sendErrorReply(FORBIDDEN_ERROR,
                       QString("Access to service ID %1 forbidden").arg(serviceId));
    }
    return hasAccess;
}

QList<AccountInfo> Manager::GetAccounts(const QStringList &serviceIds)
{
    Q_FOREACH(const QString &serviceId, serviceIds) {
        if (!checkAccess(serviceId)) {
            return QList<AccountInfo>();
        }
    }

    return QList<AccountInfo>({AccountInfo(0, QVariantMap())});
}

AccountInfo Manager::GetAccountInfo(const QString &serviceId, uint accountId)
{
    Q_UNUSED(accountId);

    if (!checkAccess(serviceId)) {
        return AccountInfo();
    }

    return AccountInfo(accountId, QVariantMap());
}

QVariantMap Manager::Authenticate(const QString &serviceId, uint accountId,
                                  bool interactive, bool invalidate)
{
    Q_UNUSED(accountId);
    Q_UNUSED(interactive);
    Q_UNUSED(invalidate);

    if (!checkAccess(serviceId)) {
        return QVariantMap();
    }

    return QVariantMap();
}

AccountInfo Manager::Register(const QString &serviceId, QVariantMap &credentials)
{
    Q_UNUSED(credentials);

    if (!checkAccess(serviceId)) {
        return AccountInfo();
    }

    return AccountInfo();
}
