cmake_minimum_required(VERSION 3.0)
project(dbus-socketpair CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++11")

include(GNUInstallDirs)
include(FindPkgConfig)

find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
pkg_check_modules(APPARMOR libapparmor REQUIRED)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src/daemon)
add_subdirectory(src/lib/OnlineAccounts)

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_subdirectory(tests)
