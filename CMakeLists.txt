cmake_minimum_required(VERSION 3.14)

project(momo
  VERSION 3.7
  HOMEPAGE_URL "https://github.com/morzhovets/momo"
  LANGUAGES CXX)

option(MOMO_TEST "Build momo tests" OFF)

if(MOMO_TEST AND (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
  add_subdirectory(test)
endif()
