project(OnlineAccountsDaemon)

pkg_check_modules(ACCOUNTSQT accounts-qt5 REQUIRED)
pkg_check_modules(SIGNONQT libsignon-qt5 REQUIRED)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(
    ${ACCOUNTSQT_CFLAGS}
    ${APPARMOR_CFLAGS}
    ${SIGNONQT_CFLAGS}
)

qt5_add_dbus_adaptor(adaptor_files manager.xml manager.h Manager)

add_executable(accountd
    inactivity_timer.cpp
    main.cpp
    manager.cpp
    aacontext.cpp
    ${adaptor_files})
qt5_use_modules(accountd DBus)
target_link_libraries(accountd
    ${ACCOUNTSQT_LDFLAGS}
    ${APPARMOR_LDFLAGS}
    ${SIGNONQT_LDLAGS}
)
set_target_properties(accountd PROPERTIES AUTOMOC TRUE)
