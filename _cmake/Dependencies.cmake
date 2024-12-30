include(_cmake/CPMPackageManager.cmake)

# OpenSSL
if (NOT TARGET ssl)
CPMAddPackage(
  GITHUB_REPOSITORY "google/boringssl"
  GIT_SHALLOW    ON
  GIT_TAG 0.20240930.0
  OPTIONS 
  "BUILD_SHARED_LIBS OFF"
  "OPENSSL_NO_ASM ON" # this is not secure but there a conflict ASM and NASM conflict with Boost context
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
endif()

#set(BOOST_ASIO_HAS_IO_URING ON)
#set(BOOST_ASIO_DISABLE_EPOLL ON)
#set(BOOST_ASIO_HAS_IO_URING_AS_DEFAULT ON)


# Boost
CPMAddPackage(
  NAME Boost
  VERSION 1.87.0
  GITHUB_REPOSITORY "boostorg/boost"
  GIT_TAG "boost-1.87.0"
  GIT_SHALLOW ON
  OPTIONS
  "_WIN32_WINNT ${BLT_WINAPI_VERSION}"
  "BOOST_ENABLE_CMAKE ON"
  "BOOST_INCLUDE_LIBRARIES:STRING=lockfree\\\\;beast\\\\;atomic\\\\;chrono\\\\;container\\\\;date_time\\\\;exception\\\\;filesystem\\\\;thread\\\\;program_options\\\\;asio\'"
  "BOOST_EXCLUDE_LIBRARIES \'python\\\\;parameter_python\'"
  "BOOST_ENABLE_MPI OFF"
  "BOOST_ENABLE_PYTHON OFF"
  "BUILD_SHARED_LIBS OFF"
  #"BOOST_ASIO_HAS_IO_URING ON"
  #"BOOST_ASIO_DISABLE_EPOLL ON"
  #"BOOST_ASIO_HAS_IO_URING_AS_DEFAULT ON"
  FIND_PACKAGE_ARGUMENTS "COMPONENTS program_options beast asio lockfree atomic lockfree program_options json process"
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)

# fmt
CPMAddPackage(
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 11.0.2
  OPTIONS "FMT_HEADER_ONLY ON" 
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)

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
CPMAddPackage(
	GIT_REPOSITORY https://github.com/simdjson/simdjson.git
	GIT_SHALLOW ON
	GIT_TAG v3.11.3
  OPTIONS
  "SIMDJSON_DEVELOPER_MODE OFF"
  "SIMDJSON_EXCEPTIONS OFF"
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)

# explicit-memset
# CPMAddPackage(
# 	GIT_REPOSITORY https://github.com/arnavyc/explicit-memset.git
# 	GIT_SHALLOW    ON
# 	GIT_TAG        28b3d5e969bbca4be341e2e05f4f91f972783087
#   SYSTEM YES
#   EXCLUDE_FROM_ALL YES
# )

if(NOT TARGET Eao::Detail::ztd::text)
  CPMAddPackage(
    GITHUB_REPOSITORY frkami123/ztd_text
    GIT_SHALLOW    ON
    GIT_TAG 57b75aa1b1a6f9817898611f2e3ea86eac76379e
    OPTIONS
      "EAO_ZTD_TEXT_STANDALONE ON"
    SYSTEM YES
    EXCLUDE_FROM_ALL YES)
endif()