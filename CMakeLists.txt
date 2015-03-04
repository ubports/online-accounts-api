cmake_minimum_required(VERSION 3.0)
project(OnlineAccountsApi CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++11")

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_lower) # Build types should always be lowercase but sometimes they are not.

include(GNUInstallDirs)
include(FindPkgConfig)

find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
pkg_check_modules(APPARMOR libapparmor REQUIRED)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src/daemon)
add_subdirectory(src/lib/OnlineAccounts)

if(cmake_build_type_lower MATCHES coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage" )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage" )
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage" )
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage" )

    # We add -g when building with coverage so valgrind reports line numbers.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g" )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" )
endif()

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_subdirectory(tests)

include(EnableCoverageReport)
if(cmake_build_type_lower MATCHES coverage)
    ENABLE_COVERAGE_REPORT(TARGETS OnlineAccounts FILTER /usr/include ${CMAKE_SOURCE_DIR}/tests/* ${CMAKE_BINARY_DIR}/*)
endif()

