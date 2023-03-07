cmake_minimum_required(VERSION 3.5)

project(momo_test)

file(GLOB tests "tests/*.cpp")
add_executable(momo_test ${tests})

if(NOT (${CMAKE_VERSION} VERSION_LESS "3.16"))
  target_precompile_headers(momo_test PRIVATE "tests/pch.h")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(momo_test PRIVATE -std=c++20 -Wall -Wextra -pedantic
    -Wold-style-cast -Wsign-conversion -Wno-unused-local-typedefs)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(momo_test PRIVATE /W4 /bigobj)
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
