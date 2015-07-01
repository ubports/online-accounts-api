include(FindGMock)

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  ${TEST_INCLUDE_DIRS}
)

add_subdirectory(daemon)
add_subdirectory(lib/OnlineAccounts)
add_subdirectory(unit)
