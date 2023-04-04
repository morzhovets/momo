cmake_minimum_required(VERSION 3.14)

project(momo
  VERSION 4.0
  LANGUAGES CXX)

option(MOMO_TEST "Build momo tests" OFF)

if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 20)
endif()

if(MOMO_TEST)
  add_subdirectory(test)
endif()
