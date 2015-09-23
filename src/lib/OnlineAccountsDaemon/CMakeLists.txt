project(OnlineAccountsDaemon)

pkg_check_modules(ACCOUNTSQT accounts-qt5 REQUIRED)
pkg_check_modules(OACLIENT OnlineAccountsClient REQUIRED)
pkg_check_modules(SIGNONQT libsignon-qt5 REQUIRED)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(ACCOUNTD_LIB OnlineAccountsDaemon)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(
    ${ACCOUNTSQT_CFLAGS}
    ${APPARMOR_CFLAGS}
    ${OACLIENT_CFLAGS}
    ${SIGNONQT_CFLAGS}
)

add_library(${ACCOUNTD_LIB} SHARED
    access_request.cpp
    async_operation.cpp
    authentication_request.cpp
    authenticator.cpp
    client_registry.cpp
    manager.cpp
    manager_adaptor.cpp
    state_saver.cpp
)
#set_target_properties(${ACCOUNTD_LIB} PROPERTIES
#    VERSION 1.0.0
#    SOVERSION 1
#)
qt5_use_modules(${ACCOUNTD_LIB} DBus)
target_link_libraries(${ACCOUNTD_LIB}
    ${ACCOUNTSQT_LDFLAGS}
    ${APPARMOR_LDFLAGS}
    ${OACLIENT_LDFLAGS}
    ${SIGNONQT_LDFLAGS}
)
set_target_properties(${ACCOUNTD_LIB} PROPERTIES AUTOMOC TRUE)

# Install

install(TARGETS ${ACCOUNTD_LIB}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Development files

configure_file(${ACCOUNTD_LIB}.pc.in ${ACCOUNTD_LIB}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${ACCOUNTD_LIB}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES "com.ubuntu.OnlineAccounts.Manager.xml"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces
)

install(FILES "dbus_constants.h" DESTINATION include/${ACCOUNTD_LIB})
