include(_cmake/CPMPackageManager.cmake)

# OpenSSL
find_package(OpenSSL REQUIRED COMPONENTS SSL)

# Boost
CPMAddPackage(
  NAME Boost
  VERSION 1.83.0
  GITHUB_REPOSITORY "boostorg/boost"
  GIT_TAG "boost-1.83.0"
  OPTIONS
  "BOOST_INCLUDE_LIBRARIES:STRING=beast\\\\;atomic\\\\;chrono\\\\;container\\\\;date_time\\\\;exception\\\\;filesystem\\\\;thread\\\\;program_options\\\\;asio\'"
  "BOOST_EXCLUDE_LIBRARIES \'python\\\\;parameter_python\'"
  "BOOST_ENABLE_MPI OFF"
  "BOOST_ENABLE_PYTHON OFF"
  "CMAKE_BUILD_TYPE Release"
  "BUILD_SHARED_LIBS ON"
  FIND_PACKAGE_ARGUMENTS "COMPONENTS program_options beast asio"
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)

# fmt
# CPMAddPackage(
#   GITHUB_REPOSITORY fmtlib/fmt
#   GIT_TAG 9.1.0
#   OPTIONS "FMT_HEADER_ONLY ON" 
#   SYSTEM YES
#   EXCLUDE_FROM_ALL YES
# )

# spdlog
# if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
#   set(SPDLOG_WCHAR_SUPPORT "SPDLOG_WCHAR_SUPPORT ON")
#   set(SPDLOG_WCHAR_FILENAME "SPDLOG_WCHAR_FILENAMES ON")
# else()
#   set(SPDLOG_WCHAR_SUPPORT "SPDLOG_WCHAR_SUPPORT OFF")
#   set(SPDLOG_WCHAR_FILENAME "SPDLOG_WCHAR_FILENAMES OFF")
# endif()

# CPMAddPackage(
#   GITHUB_REPOSITORY gabime/spdlog 
#   VERSION 1.11.0 
#   OPTIONS
#     "SPDLOG_FMT_EXTERNAL_HO ON"
#     "SPDLOG_ENABLE_PCH ON"
#     "SPDLOG_BUILD_PIC ON"
#     "SPDLOG_NO_EXCEPTIONS ON"
#     "${SPDLOG_WCHAR_SUPPORT}"
#     "${SPDLOG_WCHAR_FILENAME}"
#   SYSTEM YES
#   EXCLUDE_FROM_ALL YES
# )

# simdjson
# CPMAddPackage(
# 	GIT_REPOSITORY https://github.com/simdjson/simdjson.git
# 	GIT_SHALLOW    ON
# 	GIT_TAG        v3.3.0
#   OPTIONS
#   "SIMDJSON_DEVELOPER_MODE OFF"
#   "SIMDJSON_EXCEPTIONS OFF"
#   "SIMDJSON_BUILD_STATIC_LIB ON"
#   SYSTEM YES
#   EXCLUDE_FROM_ALL YES
# )
