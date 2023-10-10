cmake_minimum_required( VERSION 3.16 )

set(TARGET_NAME ChatServerTest)
# Add test executable
add_executable(${TARGET_NAME})

# Add dependencies
target_link_libraries( 
  ${TARGET_NAME}
  PUBLIC
  
  PRIVATE 
  ChatServer
  ${PROJECT_OPTIONS_TARGET}
  ${PROJECT_TEST_MAIN_TARGET}
)

target_sources(
  ${TARGET_NAME}
  PRIVATE
  "ChatServerTest.cpp")


gtest_discover_tests(
  ${TARGET_NAME}
  TEST_PREFIX
  "${TARGET_NAME}."
  EXTRA_ARGS
  -s
  --reporter=xml
  --out=${TARGET_NAME}.xml
  DISCOVERY_MODE POST_BUILD)