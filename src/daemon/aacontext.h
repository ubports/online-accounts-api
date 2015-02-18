#ifndef AACONTEXT_H
#define AACONTEXT_H

#include <QDBusConnection>
#include <QDBusMessage>
#include <QHash>
#include <QString>

class AppArmorContext {
public:
    AppArmorContext();
    ~AppArmorContext();

    QString getPeerSecurityContext(const QDBusConnection &bus, const QDBusMessage &message);

private:
    QHash<QString,QString> m_contexts;
};

#endif
