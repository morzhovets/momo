cmake_minimum_required(VERSION 3.10...4.0)

project(momo_test
  LANGUAGES CXX)

option(MOMO_TEST_LIBCXX_NEW "Use new libcxx tests" OFF)
option(MOMO_TEST_EXTRA "Test extra settings" OFF)
option(MOMO_TEST_MSVC_CLI "Test C++/CLI" OFF)
message(STATUS "MOMO_TEST_LIBCXX_NEW: " ${MOMO_TEST_LIBCXX_NEW})
message(STATUS "MOMO_TEST_EXTRA: " ${MOMO_TEST_EXTRA})
message(STATUS "MOMO_TEST_MSVC_CLI: " ${MOMO_TEST_MSVC_CLI})

message(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: " ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
message(STATUS "CMAKE_CXX_STANDARD: " ${CMAKE_CXX_STANDARD})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME})
message(STATUS "CMAKE_GENERATOR: " ${CMAKE_GENERATOR})

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
  file(GLOB MOMO_TEST_SOURCES CONFIGURE_DEPENDS "sources/*.cpp")
else()
  file(GLOB MOMO_TEST_SOURCES "sources/*.cpp")
endif()
add_executable(momo_test ${MOMO_TEST_SOURCES})

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
  target_precompile_headers(momo_test PRIVATE "sources/pch.h")
endif()

#target_compile_features(momo_test PRIVATE cxx_std_11)

target_compile_options(momo_test PRIVATE -UNDEBUG)

if(MOMO_TEST_LIBCXX_NEW)
  target_compile_definitions(momo_test PRIVATE TEST_LIBCXX_NEW)
endif()

if(MOMO_TEST_EXTRA)
  target_compile_definitions(momo_test PRIVATE TEST_EXTRA_SETTINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    string(REPLACE "/GR " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_options(momo_test PRIVATE /GR-)
  else()
    target_compile_options(momo_test PRIVATE -fno-rtti)
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  target_compile_options(momo_test PRIVATE /W4 /wd4127 /wd4458 /bigobj)
  target_compile_definitions(momo_test PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
  target_compile_options(momo_test PRIVATE $<$<NOT:$<CONFIG:Debug>>:/wd4702>)
  if(MSVC_VERSION EQUAL 1900)
    target_compile_options(momo_test PRIVATE /wd4503)
  endif()
  if(MSVC_VERSION GREATER_EQUAL 1910)
    target_compile_options(momo_test PRIVATE /Zc:__cplusplus)
    if(NOT MOMO_TEST_MSVC_CLI)
      target_compile_options(momo_test PRIVATE /permissive-)
    endif()
  endif()
  if(MOMO_TEST_MSVC_CLI)
    set_target_properties(momo_test PROPERTIES COMMON_LANGUAGE_RUNTIME "")
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-msse2" HAS_SSE2_FLAG)
  if(HAS_SSE2_FLAG)
    target_compile_options(momo_test PRIVATE -msse2)
  endif()
  if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(momo_test PRIVATE /W4)
  else()
    target_compile_options(momo_test PRIVATE -Wall -Wextra -pedantic)
  endif()
  target_compile_options(momo_test PRIVATE -Wold-style-cast -Wsign-conversion
      -Wno-unused-local-typedefs)
  if(MOMO_TEST_LIBCXX_NEW)
    target_compile_options(momo_test PRIVATE -Wno-missing-field-initializers)
  endif()
  if(WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_definitions(momo_test PRIVATE _ITERATOR_DEBUG_LEVEL=0)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(momo_test PRIVATE -Wa,-mbig-obj)
    endif()
  endif()
endif()

set(USE_NINJA_GENERATOR OFF)
if(CMAKE_GENERATOR STREQUAL "Ninja")
  set(USE_NINJA_GENERATOR ON)
endif()
option(MOMO_TEST_PRINT_BIN_SIZES "Print binary file sizes" ${USE_NINJA_GENERATOR})
if(MOMO_TEST_PRINT_BIN_SIZES)
  if(WIN32)
    add_custom_command(TARGET momo_test POST_BUILD
      COMMAND cmd /c dir *.obj /s
      COMMAND cmd /c echo: && echo Executable size:
      COMMAND cmd /c for %F in ("$<TARGET_FILE:momo_test>") do @echo %~zF %~fF)
  endif()
  if(UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_custom_command(TARGET momo_test POST_BUILD
      COMMAND sh -c "find '${CMAKE_CURRENT_BINARY_DIR}' -type f -name '*.o' -exec stat -c '%s %n' {} + | sort -k2"
      COMMAND sh -c "stat -c '%s %n' '$<TARGET_FILE:momo_test>'"
      VERBATIM)
  endif()
  if(UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_custom_command(TARGET momo_test POST_BUILD
      COMMAND sh -c "find '${CMAKE_CURRENT_BINARY_DIR}' -type f -name '*.o' -exec stat -f '%z %N' {} + | sort -k2"
      COMMAND sh -c "stat -f '%z %N' '$<TARGET_FILE:momo_test>'"
      VERBATIM)
  endif()
endif()
