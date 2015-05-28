#ifndef AACONTEXT_H
#define AACONTEXT_H

#include <QDBusConnection>
#include <QDBusMessage>
#include <QHash>
#include <QString>

class AppArmorContext {
public:
    ~AppArmorContext();

    static AppArmorContext *instance();

    QString getPeerSecurityContext(const QDBusConnection &bus, const QDBusMessage &message);

private:
    AppArmorContext();
    QHash<QString,QString> m_contexts;
};

#endif
