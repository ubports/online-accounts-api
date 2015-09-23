project(accountd)

set(ACCOUNTD accountd)

include_directories(.
    ${OnlineAccountsDaemon_SOURCE_DIR}/..
)

add_executable(${ACCOUNTD}
    inactivity_timer.cpp
    main.cpp
)
qt5_use_modules(${ACCOUNTD} DBus)
target_link_libraries(${ACCOUNTD}
    OnlineAccountsDaemon
)
set_target_properties(${ACCOUNTD} PROPERTIES AUTOMOC TRUE)

install(TARGETS ${ACCOUNTD}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(SERVICE_FILE com.ubuntu.OnlineAccounts.Manager.service)
configure_file(${SERVICE_FILE}.in ${SERVICE_FILE})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_FILE}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
