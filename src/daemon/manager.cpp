#include "manager.h"

#include <QVariant>

using namespace std;

Manager::Manager(QObject *parent) : QObject(parent) {
}

Manager::~Manager() {
}

QList<uint> Manager::GetAccounts(const QString &service_id) {
    return QList<uint>();
}

QVariantMap Manager::GetAccountInfo(const QString &service_id, uint account_id) {
    return QVariantMap();
}

QVariantMap Manager::Authenticate(const QString &service_id, uint account_id, bool interactive, bool invalidate) {
}

uint Manager::Register(const QString &service_id, QVariantMap &details, QVariantMap &credentials) {
}
