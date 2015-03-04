project(OnlineAccounts)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

add_library(OnlineAccounts SHARED
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
qt5_use_modules(OnlineAccounts DBus)
set_target_properties(OnlineAccounts PROPERTIES AUTOMOC TRUE)
