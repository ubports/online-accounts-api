set(TEST tst_qml_module)
set(SOURCES
    tst_qml_module.cpp
)

pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1)
pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1)

add_executable(${TEST} ${SOURCES})
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsDaemon_SOURCE_DIR}/..
    ${QTDBUSMOCK_INCLUDE_DIRS}
    ${QTDBUSTEST_INCLUDE_DIRS}
)

target_link_libraries(${TEST}
    ${QTDBUSMOCK_LIBRARIES}
    ${QTDBUSTEST_LIBRARIES}
)

add_definitions(
    -DTEST_QML_IMPORT_PATH="${OnlineAccountsQML_BINARY_DIR}/../.."
)

qt5_use_modules(${TEST} Core DBus Qml Test)
add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
add_dependencies(check ${TEST})
