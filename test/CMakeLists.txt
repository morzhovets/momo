cmake_minimum_required(VERSION 3.14...4.0)

project(momo_test
  LANGUAGES CXX)

file(GLOB MOMO_TEST_SOURCES CONFIGURE_DEPENDS "sources/*.cpp")
add_executable(momo_test ${MOMO_TEST_SOURCES})

target_compile_features(momo_test PRIVATE cxx_std_11)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
  target_precompile_headers(momo_test PRIVATE "sources/pch.h")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  target_compile_options(momo_test PRIVATE /W4 /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS
    /wd4127 /wd4458 /bigobj)
  if(MSVC_VERSION EQUAL 1900)
    target_compile_options(momo_test PRIVATE /wd4503 /wd4702)
  endif()
  if(MSVC_VERSION GREATER_EQUAL 1910)
    target_compile_options(momo_test PRIVATE /Zc:__cplusplus)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  target_compile_options(momo_test PRIVATE /W4 /D_ITERATOR_DEBUG_LEVEL=0
    -Wold-style-cast -Wsign-conversion -Wno-unused-local-typedefs)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  target_compile_options(momo_test PRIVATE -Wall -Wextra -pedantic -Wold-style-cast -Wsign-conversion
    -Wno-unused-local-typedefs)
endif()
