#include "manager.h"
#include <iostream>
#include <QDBusMessage>
#include "aacontext.h"

using namespace std;

struct Manager::Private {
    AppArmorContext apparmor;
};

Manager::Manager(QObject *parent)
    : QObject(parent), p(new Private) {
}

Manager::~Manager() {
}

bool Manager::checkAccess(const QString &service_id) {
}

QList<uint> Manager::GetAccounts(const QString &service_id) {
    std::cout << "Peer = " << message().service().toStdString() << std::endl;
    QString context = p->apparmor.getPeerSecurityContext(connection(), message());
    std::cout << "GetAccounts called with peer context " << context.toStdString() << std::endl;
    return QList<uint>();
}

QVariantMap Manager::GetAccountInfo(const QString &service_id, uint account_id) {
    return QVariantMap();
}

QVariantMap Manager::Authenticate(const QString &service_id, uint account_id, bool interactive, bool invalidate) {
    return QVariantMap();
}

uint Manager::Register(const QString &service_id, QVariantMap &details, QVariantMap &credentials) {
    return 0;
}
