add_library(ChatServer STATIC)

target_sources(ChatServer PRIVATE 
  Source/ChatServer.cpp
  Source/ChatServer.hpp
  Source/Listener.cpp
  Source/Listener.hpp
  Source/HttpSession.cpp
  Source/HttpSession.hpp
  Source/WebSocketSession.cpp
  Source/WebSocketSession.hpp
  Source/Broadcaster.cpp
  Source/Broadcaster.hpp
)


target_link_libraries(ChatServer 
  PUBLIC
  Boost::lockfree
  Boost::asio
  Boost::beast
)

target_include_directories(ChatServer 
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
)


if (ENABLE_TESTING)
  add_subdirectory(_test)
endif ()
