set(MODULE_NAME TwitchBot_Core)

add_library(${MODULE_NAME} INTERFACE)

target_sources(${MODULE_NAME} 
  PUBLIC
    FILE_SET HEADERS 
    BASE_DIRS Source
    FILES
      $<$<PLATFORM_ID:Windows>:Source/Windows/PlatformInclude.hpp>
      Source/SystemMessageType.hpp
      Source/CertificateStore.hpp
      Source/Conversion.hpp
      Source/MessageSerializer.hpp
      Source/Command.hpp
      Source/Database.hpp
      Source/PluginInfo.hpp
)

target_compile_definitions(${MODULE_NAME} 
  PUBLIC 
  #BOOST_ASIO_HAS_IO_URING
  #BOOST_ASIO_DISABLE_EPOLL
  #BOOST_ASIO_HAS_IO_URING_AS_DEFAULT
)

target_link_libraries(
  ${MODULE_NAME} 
  INTERFACE
  
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
  INTERFACE
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_NAME_SOURCE_DIR}/Source>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
