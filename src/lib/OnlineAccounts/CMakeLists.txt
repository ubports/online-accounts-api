project(OnlineAccountsQt)

include_directories(.
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsDaemon_SOURCE_DIR}/..
)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

add_library(OnlineAccountsQt SHARED
    account.cpp
    account_info.cpp
    authentication_data.cpp
    authentication_reply.cpp
    dbus_interface.cpp
    error.cpp
    manager.cpp
    pending_call.cpp
    request_access_reply.cpp
)
qt5_use_modules(OnlineAccountsQt DBus)
set_target_properties(OnlineAccountsQt PROPERTIES AUTOMOC TRUE)
