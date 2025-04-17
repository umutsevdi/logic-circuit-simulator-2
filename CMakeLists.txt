cmake_minimum_required(VERSION 3.12)

set(PRJ LogicCircuitSimulator)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
endif()

project(${PRJ} C CXX)

# OPTIONS
option(__ENABLE_DOXYGEN "Generate Doxygen documentation" YES)
option(__BUILD_TESTS "Build and run tests" YES)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")
message("Build Tests: ${__BUILD_TESTS}")
message("Doxygen: ${__ENABLE_DOXYGEN}")

set(__CORE core common parse jsoncpp_static)
set(__DEPENDS ui $__CORE)

message("Build Type: ${CMAKE_BUILD_TYPE}")
add_compile_options(-Wall -Wextra)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O3)
endif()

file(GLOB SOURCES src/main.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

include(cmake/Doxygen.cmake)
include(cmake/Tests.cmake)
include(cmake/Gtkmm.cmake)
include(cmake/Tfd.cmake)
include(cmake/Json.cmake)

include_directories(include)

add_subdirectory(src/common)
add_subdirectory(src/core)
add_subdirectory(src/ui)
add_subdirectory(src/parse)

target_link_libraries(${PRJ} ${__DEPENDS})

