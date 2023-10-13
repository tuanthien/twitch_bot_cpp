set(MODULE_NAME TwitchBotCpp)

add_executable(${MODULE_NAME}
)

target_sources(${MODULE_NAME} PRIVATE 
  Source/Certificates.hpp
  Source/Certificates.cpp
  Source/ServerConfig.hpp
  Source/ServerConfig.cpp
  Source/TwitchBotConfig.hpp
  Source/TwitchBotConfig.cpp
  Source/MessageSerializer.hpp
  Source/MessageSerializer.cpp
  Source/Command.hpp
  Source/Command.cpp
  Source/Commands/CppFormat.hpp
  Source/Commands/CppFormat.cpp
  Source/TwitchBot.hpp
  Source/TwitchBot.cpp
)

include(_cmake/Dependencies.cmake)
add_subdirectory(Modules/TwitchIRCParser)
add_subdirectory(Modules/ChatServer)

target_link_libraries(
  ${MODULE_NAME} 
  PUBLIC
  Boost::program_options

  PRIVATE
  TwitchIRCParser
  ChatServer

  fmt::fmt-header-only
  #spdlog::spdlog
  simdjson::simdjson_static
  OpenSSL::SSL
  Boost::asio
  Boost::beast
  Boost::json
  Boost::process
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
