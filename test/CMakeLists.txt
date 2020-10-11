cmake_minimum_required(VERSION 3.8)

project(momo_test)

file(GLOB tests "tests/*.cpp")
add_executable(momo_test ${tests})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(momo_test PRIVATE -Wall -Wextra -pedantic
    -Wold-style-cast -Wsign-conversion -Wno-unused-local-typedefs -msse2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(momo_test PRIVATE /W4 /bigobj)
endif()
