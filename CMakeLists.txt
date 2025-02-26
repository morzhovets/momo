cmake_minimum_required(VERSION 3.8...4.0)

project(momo
  VERSION 3.10
  LANGUAGES CXX)

option(MOMO_TEST "Build momo tests" OFF)

add_library(momo INTERFACE)

target_include_directories(momo INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_compile_features(momo INTERFACE cxx_std_11)

install(DIRECTORY include/momo DESTINATION include)

if(MOMO_TEST AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
  add_subdirectory(test)
endif()
