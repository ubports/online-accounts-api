project(OnlineAccountsQt)

set(CLIENT_LIB OnlineAccountsQt)
include_directories(.
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsDaemon_SOURCE_DIR}/..
)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_library(${CLIENT_LIB} SHARED
    account.cpp
    account_info.cpp
    authentication_data.cpp
    authentication_reply.cpp
    dbus_interface.cpp
    error.cpp
    manager.cpp
    pending_call.cpp
    request_access_reply.cpp
    service.cpp
)
set_target_properties(${CLIENT_LIB} PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

qt5_use_modules(${CLIENT_LIB} DBus)
set_target_properties(${CLIENT_LIB} PROPERTIES AUTOMOC TRUE)

# Library install

install(TARGETS ${CLIENT_LIB} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Development files

configure_file(${CLIENT_LIB}.pc.in ${CLIENT_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CLIENT_LIB}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES
    Account account.h
    AuthenticationData authentication_data.h
    AuthenticationReply
    Error error.h
    global.h
    Manager manager.h
    OAuth1Data
    OAuth2Data
    PasswordData
    PendingCall pending_call.h
    PendingCallWatcher
    Service service.h
    DESTINATION include/${CLIENT_LIB}/OnlineAccounts
)
