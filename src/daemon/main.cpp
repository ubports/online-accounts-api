#include "manager.h"
#include "manageradaptor.h"

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    auto server = new Manager();
    new ManagerAdaptor(server);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerService("com.ubuntu.OnlineAccounts.Manager");
    bus.registerObject("/com/ubuntu/OnlineAccounts/Manager", server);
    return app.exec();
}
