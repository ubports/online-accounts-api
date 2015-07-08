include(FindGMock)

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  ${TEST_INCLUDE_DIRS}
)

set(XVFB_COMMAND)
find_program(XVFBRUN xvfb-run)
if(XVFBRUN)
  set(XVFB_COMMAND ${XVFBRUN} -s "-screen 0 640x480x24" -a)
else()
  message(WARNING "Cannot find xvfb-run.")
endif()

add_subdirectory(daemon)
add_subdirectory(lib/OnlineAccounts)
add_subdirectory(unit)
