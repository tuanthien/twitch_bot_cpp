macro(setup_project_options targetName)
  add_library("${targetName}" INTERFACE)
  target_compile_definitions("${targetName}" INTERFACE _WIN32_WINNT=${BLT_WINAPI_VERSION})
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${targetName}
      INTERFACE
      /nologo
      /utf-8
      /permissive-
      /Zc:throwingNew-
      /Zc:preprocessor
      /Zc:externConstexpr
      /bigobj
      )
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${targetName} INTERFACE -fvisibility=hidden)
  endif()

  if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")     
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      target_compile_options(${targetName}
        INTERFACE
        /utf-8
        /permissive-
        /clang:-ftemplate-backtrace-limit=0
        )
      endif()

    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
      target_compile_options(${targetName} INTERFACE -fvisibility=hidden -fPIC)
    endif()
  endif()
endmacro()