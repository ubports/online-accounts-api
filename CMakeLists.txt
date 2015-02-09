cmake_minimum_required(VERSION 3.0)
project(dbus-socketpair CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++11")

include(GNUInstallDirs)
include(FindPkgConfig)

find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
pkg_check_modules(APPARMOR libapparmor REQUIRED)

add_subdirectory(src/daemon)
