cmake_minimum_required(VERSION 3.27)

set(PROJECT_NAME "TwitchBot")

project(${PROJECT_NAME} VERSION 0.0.1
  DESCRIPTION "${PROJECT_NAME}")

set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
set(CMAKE_POSITION_INDEPENDENT_CODE True)


include(_cmake/ProjectWarnings.cmake)
option(TWB_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" FALSE)
set(PROJECT_WARNINGS_TARGET "${PROJECT_NAME}_WARNINGS")
setup_project_warnings(${PROJECT_WARNINGS_TARGET})

# setup project options
include(_cmake/ProjectOptions.cmake)
set(PROJECT_OPTIONS_TARGET "${PROJECT_NAME}_OPTIONS")
setup_project_options(${PROJECT_OPTIONS_TARGET})

option(ENABLE_TESTING "Enable Test Builds" ON)
option(ENABLE_PCH "Enable Precompiled Headers" ON)

include(_cmake/CPMPackageManager.cmake)

if (ENABLE_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/7a7231c442484be389fdf01594310349ca0e42a8.zip DOWNLOAD_EXTRACT_TIMESTAMP
  )
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  FetchContent_MakeAvailable(googletest)
endif ()

# project's modules
include(_cmake/Module.cmake)