project(OnlineAccounts)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

add_library(OnlineAccounts SHARED
    account.cpp
    authentication_data.cpp
    manager.cpp
)
qt5_use_modules(OnlineAccounts DBus)
set_target_properties(OnlineAccounts PROPERTIES AUTOMOC TRUE)
