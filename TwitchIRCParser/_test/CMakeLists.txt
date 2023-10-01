set(PROJECT_TEST_MAIN_TARGET IRCParserTest_Main)

add_library(${PROJECT_TEST_MAIN_TARGET} STATIC)

target_sources(
    ${PROJECT_TEST_MAIN_TARGET}
    PUBLIC
    "gtest_main.cpp"
)

include(GoogleTest)

target_link_libraries(
  ${PROJECT_TEST_MAIN_TARGET}
  PUBLIC 
    GTest::gtest
  PRIVATE 
    "$<BUILD_INTERFACE:${PROJECT_WARNINGS_TARGET}>" 
    "$<BUILD_INTERFACE:${PROJECT_OPTIONS_TARGET}>"
 )

add_subdirectory(IRCParserTest)