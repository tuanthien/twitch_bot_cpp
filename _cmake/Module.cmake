set(MODULE_NAME TwitchBotCpp)

add_executable(${MODULE_NAME}
)

target_sources(${MODULE_NAME} PRIVATE 
  Source/Certificates.hpp
  Source/Certificates.cpp
  Source/TwitchBot.hpp
  Source/TwitchBot.cpp
)

include(_cmake/Dependencies.cmake)
add_subdirectory(TwitchIRCParser)

target_link_libraries(
  ${MODULE_NAME} 
  PUBLIC
    
  PRIVATE
  
  #fmt::fmt-header-only
  #spdlog::spdlog
  #simdjson::simdjson_static
  OpenSSL::SSL
  Boost::asio
  Boost::beast
  #Boost::program_options
  
  TwitchIRCParser
  
  "$<BUILD_INTERFACE:${PROJECT_WARNINGS_TARGET}>" 
  "$<BUILD_INTERFACE:${PROJECT_OPTIONS_TARGET}>" 
)

target_include_directories(
  ${MODULE_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${GENERATED_PROJECT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/Source>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (ENABLE_PCH)
  target_precompile_headers(
    ${MODULE_NAME}
    PRIVATE
    <boost/beast/core.hpp>
    <boost/beast/websocket.hpp>
    <boost/beast/ssl.hpp>
    <boost/beast/websocket/ssl.hpp>
    <cstdlib>
    <functional>
    <iostream>
    <string>
    <boost/asio/as_tuple.hpp>
    <boost/asio/awaitable.hpp>
    <boost/asio/co_spawn.hpp>
    <boost/asio/detached.hpp>
    <boost/asio/use_awaitable.hpp>
    <Certificates.hpp>
    )
endif()
