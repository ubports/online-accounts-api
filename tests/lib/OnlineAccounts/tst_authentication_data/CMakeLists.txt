set(TEST tst_authentication_data)
set(SOURCES
    ${OnlineAccounts_SOURCE_DIR}/authentication_data.cpp
    tst_authentication_data.cpp
)
add_executable(${TEST} ${SOURCES})
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccounts_SOURCE_DIR}
)
qt5_use_modules(${TEST} Core Test)
add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST} -xunitxml -o ${TEST}.xml)
add_dependencies(check ${TEST})
