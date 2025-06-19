cmake_minimum_required(VERSION 3.20...4.0)

project(momo_test
  LANGUAGES CXX)

option(MOMO_TEST_EXTRA "Test extra settings" OFF)

message(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: " ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
message(STATUS "CMAKE_CXX_STANDARD: " ${CMAKE_CXX_STANDARD})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_HOST_SYSTEM_PROCESSOR: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
message(STATUS "MOMO_TEST_EXTRA: " ${MOMO_TEST_EXTRA})

file(GLOB MOMO_TEST_SOURCES CONFIGURE_DEPENDS "sources/*.cpp")
add_executable(momo_test ${MOMO_TEST_SOURCES})

target_precompile_headers(momo_test PRIVATE "sources/pch.h")

#target_compile_features(momo_test PRIVATE cxx_std_23)

if(MOMO_TEST_EXTRA)
  target_compile_definitions(momo_test PRIVATE TEST_EXTRA_SETTINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_options(momo_test PRIVATE /EHs-c- /GR-)
    target_compile_definitions(momo_test PRIVATE _HAS_EXCEPTIONS=0)
  else()
    target_compile_options(momo_test PRIVATE -fno-exceptions -fno-rtti)
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  target_compile_options(momo_test PRIVATE /W4 /wd4127 /wd4458 /bigobj /Zc:__cplusplus)
  target_compile_definitions(momo_test PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(momo_test PRIVATE /wd4702)
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
    target_compile_options(momo_test PRIVATE -W4)
  else()
    target_compile_options(momo_test PRIVATE -Wall -Wextra -pedantic)
  endif()

  target_compile_options(momo_test PRIVATE -Wold-style-cast -Wsign-conversion
    -Wno-unused-local-typedefs)

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(momo_test PRIVATE -Wno-unknown-attributes)
    if(WIN32)
      target_compile_definitions(momo_test PRIVATE _ITERATOR_DEBUG_LEVEL=0)
    endif()
  endif()

  if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(momo_test PRIVATE -Wa,-mbig-obj)
  endif()

endif()
