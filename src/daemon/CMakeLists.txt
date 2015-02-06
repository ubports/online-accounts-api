
include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

qt5_add_dbus_adaptor(adaptor_files manager.xml manager.h Manager)

add_executable(accountd main.cpp manager.cpp ${adaptor_files})
qt5_use_modules(accountd DBus)
target_link_libraries(accountd -lapparmor)
set_target_properties(accountd PROPERTIES AUTOMOC TRUE)
