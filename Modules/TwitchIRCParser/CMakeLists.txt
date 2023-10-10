add_library(TwitchIRCParser STATIC)

target_sources(TwitchIRCParser PRIVATE 
  Source/TwitchIRCParser.cpp
  Source/TwitchIRCParser.hpp
)

target_include_directories(TwitchIRCParser 
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
)


if (ENABLE_TESTING)
  add_subdirectory(_test)
endif ()
