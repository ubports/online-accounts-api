#include "manager.h"

#include <QDBusMessage>
#include <QDebug>
#include "aacontext.h"

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
    delete d_ptr;
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

QList<AccountInfo> Manager::GetAccounts(const QVariantMap &filters)
{
    Q_UNUSED(filters);

    return QList<AccountInfo>();
}

QVariantMap Manager::Authenticate(uint accountId, const QString &serviceId,
                                  bool interactive, bool invalidate,
                                  const QVariantMap &parameters)
{
    Q_UNUSED(accountId);
    Q_UNUSED(interactive);
    Q_UNUSED(invalidate);
    Q_UNUSED(parameters);

    if (!checkAccess(serviceId)) {
        return QVariantMap();
    }

    return QVariantMap();
}

AccountInfo Manager::RequestAccess(const QString &serviceId,
                                   const QVariantMap &parameters,
                                   QVariantMap &credentials)
{
    Q_UNUSED(parameters);

    if (!checkAccess(serviceId)) {
        return AccountInfo();
    }

    credentials = QVariantMap();
    return AccountInfo();
}
