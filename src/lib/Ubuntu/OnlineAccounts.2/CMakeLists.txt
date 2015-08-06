project(OnlineAccountsQML)

set(PLUGIN OnlineAccountsQML)
include_directories(.
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OnlineAccountsQt_SOURCE_DIR}/..
)

set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined")
add_library(${PLUGIN} MODULE
    account.cpp
    account_model.cpp
    authentication_data.cpp
    plugin.cpp
)

target_link_libraries(${PLUGIN}
    OnlineAccountsQt
)

qt5_use_modules(${PLUGIN} Core Qml)
set_target_properties(${PLUGIN} PROPERTIES AUTOMOC TRUE)

file(GLOB QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qml qmldir *.js)

# Module install

set(QT_INSTALL_QML "${CMAKE_INSTALL_LIBDIR}/qt5/qml")
set(PLUGIN_IMPORTS_DIR "${QT_INSTALL_QML}/Ubuntu/OnlineAccounts.2")
install(TARGETS ${PLUGIN} DESTINATION ${PLUGIN_IMPORTS_DIR})
install(FILES ${QML_FILES} DESTINATION ${PLUGIN_IMPORTS_DIR})

if(NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  # copy qml files over to build dir to be able to import them uninstalled
  foreach(_file ${QML_FILES})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${CMAKE_CURRENT_BINARY_DIR}/${_file})
  endforeach(_file)
  add_custom_target(copy_files_to_build_dir DEPENDS ${QML_FILES})
  add_dependencies(${PLUGIN} copy_files_to_build_dir)
endif()

