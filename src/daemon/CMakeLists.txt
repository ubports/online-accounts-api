project(OnlineAccountsDaemon)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR} ${APPARMOR_CFLAGS})

qt5_add_dbus_adaptor(adaptor_files manager.xml manager.h Manager)

add_executable(accountd
    inactivity_timer.cpp
    main.cpp
    manager.cpp
    aacontext.cpp
    ${adaptor_files})
qt5_use_modules(accountd DBus)
target_link_libraries(accountd ${APPARMOR_LDFLAGS})
set_target_properties(accountd PROPERTIES AUTOMOC TRUE)
