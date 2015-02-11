add_executable(
  unit-test-template
  unit-test-template.cpp
)

target_link_libraries(
  unit-test-template
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
  ${TEST_LDFLAGS}
)

add_test(
  unit-test-template
  unit-test-template
)
