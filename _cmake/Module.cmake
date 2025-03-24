set(MODULE_NAME TwitchBotCpp)

add_executable(${MODULE_NAME})

target_sources(${MODULE_NAME} 
  PUBLIC
    FILE_SET HEADERS 
    BASE_DIRS Source
    FILES
      Source/ServerConfig.hpp
      Source/TwitchBotConfig.hpp
      Source/DatabaseConfig.hpp
      Source/PluginModule.hpp
      Source/PluginRegistry.hpp
      Source/TwitchBot.hpp

  PRIVATE 
    Source/DatabaseConfig.cpp
    Source/ServerConfig.cpp
    Source/TwitchBotConfig.cpp
    Source/TwitchBot.cpp
)

include(_cmake/Dependencies.cmake)
add_subdirectory(Source/Core)
add_subdirectory(Modules/TwitchIRCParser)
add_subdirectory(Modules/ChatServer)
add_subdirectory(Plugins/CppFormat)
add_subdirectory(Plugins/CommandList)
add_subdirectory(Plugins/Poke)

target_compile_definitions(${MODULE_NAME} 
  PUBLIC 
  #BOOST_ASIO_HAS_IO_URING
  #BOOST_ASIO_DISABLE_EPOLL
  #BOOST_ASIO_HAS_IO_URING_AS_DEFAULT
)

target_link_libraries(
  ${MODULE_NAME} 
 
  PUBLIC
  Boost::program_options
  Eao::Detail::ztd::text
  TwitchBot_Core

  PRIVATE
  TwitchIRCParser
  ChatServer
  fmt::fmt-header-only
  simdjson::simdjson
  
  ssl crypto
  
  Boost::asio
  Boost::beast
  Boost::json
  Boost::process
  Boost::charconv
  Boost::mysql
  
  $<$<PLATFORM_ID:Linux>:uring>
  $<$<PLATFORM_ID:Windows>:Crypt32.lib>
  
  "$<BUILD_INTERFACE:${PROJECT_WARNINGS_TARGET}>" 
  "$<BUILD_INTERFACE:${PROJECT_OPTIONS_TARGET}>" 
)

target_include_directories(
  ${MODULE_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${GENERATED_PROJECT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_NAME_SOURCE_DIR}/Source>
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
