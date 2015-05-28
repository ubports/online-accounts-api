#include "aacontext.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <sys/apparmor.h>

const int maxCacheSize = 50;
static AppArmorContext *static_instance = 0;

AppArmorContext::AppArmorContext()
{
}

AppArmorContext::~AppArmorContext()
{
}

AppArmorContext *AppArmorContext::instance()
{
    if (!static_instance) {
        static_instance = new AppArmorContext();
    }
    return static_instance;
}

QString AppArmorContext::getPeerSecurityContext(const QDBusConnection &bus,
                                                const QDBusMessage &message)
{
    if (!aa_is_enabled()) {
        return "unconfined";
    }

    QString peer_address = message.service();
    QString &context = m_contexts[peer_address];
    /* If the peer_address was now known, it's now been added as an empty
     * QString. */
    if (!context.isEmpty()) {
        return context;
    }

    QDBusMessage msg =
        QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                       "/org/freedesktop/DBus",
                                       "org.freedesktop.DBus",
                                       "GetConnectionAppArmorSecurityContext");
    msg << peer_address;
    QDBusMessage reply = bus.call(msg, QDBus::Block);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        context = reply.arguments().value(0).value<QString>();
    } else {
        qWarning() << "Could not determine AppArmor context: " <<
            reply.errorName() << ": " << reply.errorMessage();
    }

    // If the context cache has hit the maximum size, clear it
    if (m_contexts.size() >= maxCacheSize) {
        m_contexts.clear();
    }
    return context;
}
