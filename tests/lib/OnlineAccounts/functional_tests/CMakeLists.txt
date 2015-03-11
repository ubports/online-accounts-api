set(TEST functional_tests)
set(SOURCES
    functional_tests.cpp
)

pkg_check_modules(QTDBUSMOCK REQUIRED libqtdbusmock-1)
pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1)

add_executable(${TEST} ${SOURCES})
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsQt_SOURCE_DIR}/..
    ${QTDBUSMOCK_INCLUDE_DIRS}
    ${QTDBUSTEST_INCLUDE_DIRS}
)

target_link_libraries(${TEST}
    OnlineAccountsQt
    ${QTDBUSMOCK_LIBRARIES}
    ${QTDBUSTEST_LIBRARIES}
)

qt5_use_modules(${TEST} Core Test)
add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST} -xunitxml -o ${TEST}.xml)
add_dependencies(check ${TEST})
