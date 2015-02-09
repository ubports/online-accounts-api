#include "aacontext.h"
#include <stdexcept>
#include <sys/apparmor.h>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>

const int max_cache_size = 50;

AppArmorContext::AppArmorContext() {
}

AppArmorContext::~AppArmorContext() {
}

QString AppArmorContext::getPeerSecurityContext(const QDBusConnection &bus, const QDBusMessage &message) {
    if (!aa_is_enabled()) {
        return "unconfined";
    }

    QString peer_address = message.service();
    qWarning() << "Peer address is " << peer_address;

    try {
        return contexts.at(peer_address);
    } catch (const std::out_of_range &e) {
    }

    qWarning() << "Address wasn't in cache";
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus", "GetConnectionAppArmorSecurityContext");
    msg << peer_address;
    qWarning() << "Calling GetConnectionAppArmorSecurityContext";
    QDBusMessage reply = bus.call(msg, QDBus::Block);

    QString context;
    if (reply.type() == QDBusMessage::ReplyMessage) {
        context = reply.arguments().value(0).value<QString>();
    } else {
        qWarning() << "Could not determine AppArmor context: "
                   << reply.errorName() << ": " << reply.errorMessage();
    }

    // If the context cache has hit the maximum size, clear it
    if (contexts.size() >= max_cache_size) {
        contexts.clear();
    }
    contexts[peer_address] = context;
    return context;
}
