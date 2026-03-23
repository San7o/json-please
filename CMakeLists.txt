# ====================================================================
# Project:   json-please
# Author:    Giovanni Santini
# Mail:      giovanni.santini@proton.me
# Github:    @San7o
# License:   MIT
# ====================================================================

cmake_minimum_required(VERSION 3.16)

project(json-please
  VERSION 1.0.0
  DESCRIPTION "Simple json serializer / deserializer"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(JP_SOURCES)
set(JP_TEST_SOURCES)
set(JP_TEST_INCLUDES)

find_program(CCACHE_PROGRAM ccache) # use ccache if available
if(CCACHE_PROGRAM)
  message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(JP_DEFINES)

if(MSVC)
  set(JP_WARNING_FLAGS /W4 /permissive-)
  set(JP_DEBUG_FLAGS /Zi /Od)
  set(JP_RELEASE_FLAGS /O2)
  set(JP_DEBUG_DEFINES _DEBUG)
elseif(MINGW)
  set(JP_WARNING_FLAGS -Wall -Wextra -Wpedantic)
  set(JP_DEBUG_FLAGS -O0 -ggdb)
  set(JP_RELEASE_FLAGS -O3)
  set(JP_DEBUG_DEFINES DEBUG)
else()
  set(JP_WARNING_FLAGS -Wall -Wextra -Werror -Wpedantic)
  set(JP_DEBUG_FLAGS -O0 -ggdb -fsanitize=address)
  set(JP_RELEASE_FLAGS -O3)
  set(JP_DEBUG_DEFINES DEBUG)
  set(JP_DEBUG_LINK_FLAGS -fsanitize=address)
endif()

#
# Options
#

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(JP_BUILD_TESTS    "Build tests" OFF)

#
# Dependencies
#

# Use CPM for dependency management
include(cmake/CPM.cmake)

if(JP_BUILD_TESTS)
  CPMAddPackage(
    NAME valfuzz
    GITHUB_REPOSITORY San7o/valFuzz
    GIT_TAG origin/main)
  CPMUsePackageLock(package-lock.cmake)
endif()

#
# Targets
#

file(GLOB JP_SOURCES src/*.cpp)

if (BUILD_SHARED_LIBS)
  message(STATUS "Building shared library")
  add_library(${PROJECT_NAME} SHARED
    ${JP_SOURCES})
else()
  message(STATUS "Building static library")
  add_library(${PROJECT_NAME} STATIC
    ${JP_SOURCES})  
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  ${JP_DEFINES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Configuring Debug build options...")

  target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${JP_DEBUG_DEFINES})
  target_compile_options(${PROJECT_NAME} PRIVATE
    ${JP_DEBUG_FLAGS}
    ${JP_WARNING_FLAGS})
  if(NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
      ${JP_DEBUG_LINK_FLAGS})
  endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Configuring Release build options...")
  target_compile_options(${PROJECT_NAME} PRIVATE
    ${JP_RELEASE_FLAGS}
    ${JP_WARNING_FLAGS})
endif()

if (JP_BUILD_TESTS)
  message(STATUS "Building tests")
  
  if (NOT (valfuzz_ADDED))
    message(ERROR "Failed to configure dependencies")
  endif()
  
  file(GLOB JP_TESTS_SOURCES tests/*.cpp)
  list(APPEND JP_TEST_INCLUDES
    include
    ${valfuzz_SOURCE_DIR}/include)
  add_executable(${PROJECT_NAME}_tests
    ${JP_TESTS_SOURCES})
  target_include_directories(${PROJECT_NAME}_tests PRIVATE
    ${JP_TEST_INCLUDES})
  target_link_libraries(${PROJECT_NAME}_tests PUBLIC
    valfuzz
    ${PROJECT_NAME})

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build options...")

    target_compile_definitions(${PROJECT_NAME}_tests PRIVATE
      ${JP_DEBUG_DEFINES})
    target_compile_options(${PROJECT_NAME}_tests PRIVATE
      ${JP_DEBUG_FLAGS}
      ${JP_WARNING_FLAGS})
    if(NOT MSVC)
      target_link_options(${PROJECT_NAME}_tests PRIVATE
        ${JP_DEBUG_LINK_FLAGS})
    endif()
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build options...")
    target_compile_options(${PROJECT_NAME}_tests PRIVATE
      ${JP_RELEASE_FLAGS}
      ${JP_WARNING_FLAGS})
  endif()
  
endif()

#
# End of CMakeLists.txt
# ====================================================================
