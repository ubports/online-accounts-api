project(OnlineAccountsDaemon)

pkg_check_modules(ACCOUNTSQT accounts-qt5 REQUIRED)
pkg_check_modules(OACLIENT OnlineAccountsClient REQUIRED)
pkg_check_modules(SIGNONQT libsignon-qt5 REQUIRED)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(
    ${ACCOUNTSQT_CFLAGS}
    ${APPARMOR_CFLAGS}
    ${OACLIENT_CFLAGS}
    ${SIGNONQT_CFLAGS}
)

add_executable(accountd
    access_request.cpp
    async_operation.cpp
    authentication_request.cpp
    authenticator.cpp
    client_registry.cpp
    inactivity_timer.cpp
    main.cpp
    manager.cpp
    manager_adaptor.cpp
    state_saver.cpp
)
qt5_use_modules(accountd DBus)
target_link_libraries(accountd
    ${ACCOUNTSQT_LDFLAGS}
    ${APPARMOR_LDFLAGS}
    ${OACLIENT_LDFLAGS}
    ${SIGNONQT_LDFLAGS}
)
set_target_properties(accountd PROPERTIES AUTOMOC TRUE)
