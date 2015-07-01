set(TEST tst_daemon)
set(SOURCES
    daemon_interface.cpp
    functional_tests.cpp
)

pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1)
pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1)

add_executable(${TEST} ${SOURCES})
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsQt_SOURCE_DIR}/..
    ${OnlineAccountsDaemon_SOURCE_DIR}/..
    ${QTDBUSMOCK_INCLUDE_DIRS}
    ${QTDBUSTEST_INCLUDE_DIRS}
)
add_definitions(
    -DTEST_DBUS_CONFIG_FILE="${CMAKE_CURRENT_BINARY_DIR}/data/testsession.conf"
    -DSIGNOND_MOCK_TEMPLATE="${CMAKE_CURRENT_SOURCE_DIR}/signond.py"
    -DONLINE_ACCOUNTS_SERVICE_MOCK_TEMPLATE="${CMAKE_CURRENT_SOURCE_DIR}/online_accounts-service.py"
)   

configure_file(data/testsession.conf.in data/testsession.conf)
configure_file(data/com.ubuntu.OnlineAccounts.Manager.service.in
    data/com.ubuntu.OnlineAccounts.Manager.service
)

target_link_libraries(${TEST}
    OnlineAccountsQt
    ${QTDBUSMOCK_LIBRARIES}
    ${QTDBUSTEST_LIBRARIES}
)

qt5_use_modules(${TEST} Core DBus Test)

set_target_properties(${TEST} PROPERTIES AUTOMOC TRUE)
add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST} -xunitxml -o ${TEST}.xml)
add_dependencies(check ${TEST})
