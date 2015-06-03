project(OnlineAccountsDaemon)

pkg_check_modules(ACCOUNTSQT accounts-qt5 REQUIRED)
pkg_check_modules(SIGNONQT libsignon-qt5 REQUIRED)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(
    ${ACCOUNTSQT_CFLAGS}
    ${APPARMOR_CFLAGS}
    ${SIGNONQT_CFLAGS}
)

add_executable(accountd
    client_registry.cpp
    inactivity_timer.cpp
    main.cpp
    manager.cpp
    manager_adaptor.cpp
    state_saver.cpp
)
qt5_use_modules(accountd DBus)
target_link_libraries(accountd
    ${ACCOUNTSQT_LDFLAGS}
    ${APPARMOR_LDFLAGS}
    ${SIGNONQT_LDLAGS}
)
set_target_properties(accountd PROPERTIES AUTOMOC TRUE)
