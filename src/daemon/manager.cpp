#include "manager.h"
#include <QDebug>
#include <QDBusMessage>
#include "aacontext.h"

using namespace std;

static const char FORBIDDEN_ERROR[] = "com.ubuntu.OnlineAccounts.Error.Forbidden";

QDBusArgument &operator<<(QDBusArgument &argument, const AccountInfo &info) {
    argument.beginStructure();
    argument << info.account_id << info.details;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AccountInfo &info) {
    argument.beginStructure();
    argument >> info.account_id >> info.details;
    argument.endStructure();
    return argument;
}


struct Manager::Private {
    AppArmorContext apparmor;
};

Manager::Manager(QObject *parent)
    : QObject(parent), p(new Private) {
}

Manager::~Manager() {
}

bool Manager::canAccess(const QString &service_id) {
    QString context = p->apparmor.getPeerSecurityContext(connection(), message());
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
    pos = service_id.indexOf('_');
    if (pos < 0) {
        return false;
    }
    return service_id.left(pos) == pkgname;
}

bool Manager::checkAccess(const QString &service_id) {
    bool has_access = canAccess(service_id);
    if (!has_access) {
        sendErrorReply(FORBIDDEN_ERROR, QString("Access to service ID %1 forbidden").arg(service_id));
    }
    return has_access;
}

QList<AccountInfo> Manager::GetAccounts(const QStringList &service_ids) {
    for (const auto &service_id : service_ids) {
        if (!checkAccess(service_id)) {
            return QList<AccountInfo>();
        }
    }

    return QList<AccountInfo>({AccountInfo(0, QVariantMap())});
}

AccountInfo Manager::GetAccountInfo(const QString &service_id, uint account_id) {
    if (!checkAccess(service_id)) {
        return AccountInfo();
    }

    return AccountInfo(account_id, QVariantMap());
}

QVariantMap Manager::Authenticate(const QString &service_id, uint account_id, bool interactive, bool invalidate) {
    if (!checkAccess(service_id)) {
        return QVariantMap();
    }

    return QVariantMap();
}

AccountInfo Manager::Register(const QString &service_id, QVariantMap &credentials) {
    if (!checkAccess(service_id)) {
        return AccountInfo();
    }

    return AccountInfo();
}
