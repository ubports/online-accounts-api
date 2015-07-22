set(TEST tst_authentication_data)
set(SOURCES
    ${OnlineAccountsQt_SOURCE_DIR}/authentication_data.cpp
    tst_authentication_data.cpp
)
add_executable(${TEST} ${SOURCES})
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsQt_SOURCE_DIR}
    ${OnlineAccountsDaemon_SOURCE_DIR}/..
)
qt5_use_modules(${TEST} Core DBus Test)
add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
add_dependencies(check ${TEST})
