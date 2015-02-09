#ifndef AACONTEXT_H
#define AACONTEXT_H

#include <map>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QString>

class AppArmorContext {
public:
    AppArmorContext();
    ~AppArmorContext();

    QString getPeerSecurityContext(const QDBusConnection &bus, const QDBusMessage &message);

private:
    std::map<QString,QString> contexts;
};

#endif
