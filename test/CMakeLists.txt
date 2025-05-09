cmake_minimum_required(VERSION 3.10...4.0)

project(momo_test
  LANGUAGES CXX)

option(MOMO_TEST_LIBCXX_NEW "Use new libcxx tests" OFF)
option(MOMO_TEST_EXTRA "Test extra settings" OFF)
option(MOMO_TEST_MSVC_CLI "Test C++/CLI" OFF)

message(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: " ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
message(STATUS "CMAKE_CXX_STANDARD: " ${CMAKE_CXX_STANDARD})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "MOMO_TEST_LIBCXX_NEW: " ${MOMO_TEST_LIBCXX_NEW})
message(STATUS "MOMO_TEST_EXTRA: " ${MOMO_TEST_EXTRA})
message(STATUS "MOMO_TEST_MSVC_CLI: " ${MOMO_TEST_MSVC_CLI})

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

if(MOMO_TEST_LIBCXX_NEW)
  target_compile_definitions(momo_test PRIVATE TEST_LIBCXX_NEW)
endif()

if(MOMO_TEST_EXTRA)
  target_compile_definitions(momo_test PRIVATE TEST_EXTRA_SETTINGS)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  target_compile_options(momo_test PRIVATE /W4 /wd4127 /wd4458 /bigobj)
  target_compile_definitions(momo_test PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(momo_test PRIVATE /wd4702)
  endif()
  if(MSVC_VERSION EQUAL 1900)
    target_compile_options(momo_test PRIVATE /wd4503)
  endif()
  if(MSVC_VERSION GREATER_EQUAL 1910)
    target_compile_options(momo_test PRIVATE /permissive- /Zc:__cplusplus)
  endif()

  if(MOMO_TEST_MSVC_CLI)
    set_target_properties(momo_test PROPERTIES COMMON_LANGUAGE_RUNTIME "")
  endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")

  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-msse2" HAS_SSE2_FLAG)

  if(HAS_SSE2_FLAG AND NOT MOMO_TEST_EXTRA)
    target_compile_options(momo_test PRIVATE -msse2)
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(momo_test PRIVATE /W4)
  else()
    if(MOMO_TEST_EXTRA)
      target_compile_options(momo_test PRIVATE -fno-rtti)
    endif()
    target_compile_options(momo_test PRIVATE -Wall -Wextra -pedantic)
  endif()

  target_compile_options(momo_test PRIVATE -Wold-style-cast -Wsign-conversion
    -Wno-unused-local-typedefs)

  if(WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_definitions(momo_test PRIVATE _ITERATOR_DEBUG_LEVEL=0)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(momo_test PRIVATE -Wa,-mbig-obj)
    endif()
  endif()

endif()
