project(OnlineAccountsDaemon)

pkg_check_modules(ACCOUNTSQT accounts-qt5 REQUIRED)
pkg_check_modules(OACLIENT OnlineAccountsClient REQUIRED)
pkg_check_modules(SIGNONQT libsignon-qt5 REQUIRED)

set(ACCOUNTD accountd)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(
    ${ACCOUNTSQT_CFLAGS}
    ${APPARMOR_CFLAGS}
    ${OACLIENT_CFLAGS}
    ${SIGNONQT_CFLAGS}
)

add_executable(${ACCOUNTD}
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
qt5_use_modules(${ACCOUNTD} DBus)
target_link_libraries(${ACCOUNTD}
    ${ACCOUNTSQT_LDFLAGS}
    ${APPARMOR_LDFLAGS}
    ${OACLIENT_LDFLAGS}
    ${SIGNONQT_LDFLAGS}
)
set_target_properties(${ACCOUNTD} PROPERTIES AUTOMOC TRUE)

install(TARGETS ${ACCOUNTD}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(SERVICE_FILE com.ubuntu.OnlineAccounts.Manager.service)
configure_file(${SERVICE_FILE}.in ${SERVICE_FILE})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_FILE}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

# Development files

set(ACCOUNTD_LIB OnlineAccountsDaemon)
configure_file(${ACCOUNTD_LIB}.pc.in ${ACCOUNTD_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${ACCOUNTD_LIB}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES "com.ubuntu.OnlineAccounts.Manager.xml"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces
)

install(FILES "dbus_constants.h" DESTINATION include/${ACCOUNTD_LIB})
