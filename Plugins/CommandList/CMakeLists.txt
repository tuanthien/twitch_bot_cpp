set(MODULE_NAME CommandList)

add_library(${MODULE_NAME} SHARED)

target_sources(${MODULE_NAME} 
  PUBLIC
    FILE_SET HEADERS 
    BASE_DIRS Source
    FILES
      Source/CommandList.hpp
      Source/CommandListMessage.hpp

  PRIVATE 
    Source/CommandList.cpp
    Source/CommandListMessage.cpp
)

target_compile_definitions(${MODULE_NAME} 
  PUBLIC 
  #BOOST_ASIO_HAS_IO_URING
  #BOOST_ASIO_DISABLE_EPOLL
  #BOOST_ASIO_HAS_IO_URING_AS_DEFAULT
)

target_link_libraries(
  ${MODULE_NAME} 
  PRIVATE
  Eao::Detail::ztd::text
  TwitchBot_Core
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

set(PROJECT_GENERATED_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include/Generated)
file(MAKE_DIRECTORY ${PROJECT_GENERATED_INCLUDE_DIR})
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)
set(GENERATED_HEADER_${MODULE_NAME_UPPER}_EXTRA "// Add extra")
include(GenerateExportHeader)
generate_export_header(${MODULE_NAME}
  BASE_NAME ${MODULE_NAME_UPPER}
  EXPORT_MACRO_NAME ${MODULE_NAME_UPPER}_API
  EXPORT_FILE_NAME "${PROJECT_GENERATED_INCLUDE_DIR}/${MODULE_NAME}Export.hpp"
  DEPRECATED_MACRO_NAME ${MODULE_NAME_UPPER}_DEPRECATED
  NO_EXPORT_MACRO_NAME ${MODULE_NAME_UPPER}_PRIVATE
  INCLUDE_GUARD_NAME ${MODULE_NAME_UPPER}_EXPORT_HPP
  STATIC_DEFINE ${MODULE_NAME_UPPER}_STATIC_DEFINE
  NO_DEPRECATED_MACRO_NAME ${MODULE_NAME_UPPER}_OMIT_DEPRECATED
  ${deprecatedOption}
  CUSTOM_CONTENT_FROM_VARIABLE GENERATED_HEADER_${MODULE_NAME_UPPER}_EXTRA)
  
target_include_directories(
  ${MODULE_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_GENERATED_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_NAME_SOURCE_DIR}/Source>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Source/Public>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


