set(MODULE_NAME TwitchBotCpp)

add_executable(${MODULE_NAME})

target_sources(${MODULE_NAME} 
  PUBLIC
    FILE_SET HEADERS 
    BASE_DIRS Source
    FILES
      $<$<PLATFORM_ID:Windows>:Source/Windows/PlatformInclude.hpp>
      Source/CertificateStore.hpp
      Source/ServerConfig.hpp
      Source/TwitchBotConfig.hpp
      Source/MessageSerializer.hpp
      Source/Command.hpp
      Source/Commands/CppFormat/CppFormat.hpp
      Source/Commands/CppFormat/CppFormatMessage.hpp
      Source/Commands/CommandList/CommandList.hpp
      Source/Commands/CommandList/CommandListMessage.hpp
      Source/TwitchBot.hpp

  PRIVATE 
    $<$<PLATFORM_ID:Windows>:Source/Windows/CertificateStore.cpp>
    Source/ServerConfig.cpp
    Source/TwitchBotConfig.cpp
    Source/MessageSerializer.cpp
    Source/Command.cpp
    Source/Commands/CppFormat/CppFormat.cpp
    Source/Commands/CppFormat/CppFormatMessage.cpp
    Source/Commands/CommandList/CommandList.cpp
    Source/Commands/CommandList/CommandListMessage.cpp
    Source/TwitchBot.cpp
)

include(_cmake/Dependencies.cmake)
add_subdirectory(Modules/TwitchIRCParser)
add_subdirectory(Modules/ChatServer)

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
  
  PRIVATE
  TwitchIRCParser
  ChatServer

  fmt::fmt-header-only
  #spdlog::spdlog
  simdjson::simdjson
  
  ssl crypto
    
  Boost::asio
  Boost::beast
  Boost::json
  Boost::process

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
