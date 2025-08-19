cmake_minimum_required(VERSION 3.15)
project(LogicCircuitSimulator C CXX)

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

# OPTIONS
option(LCS_ENABLE_DOXYGEN "Generate Doxygen documentation" YES)
option(LCS_BUILD_TESTS "Build and run tests" YES)
option(LCS_GUI "Build with user interface" YES)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")
message("Build Tests: ${LCS_BUILD_TESTS}")
message("Doxygen: ${LCS_ENABLE_DOXYGEN}")
message("GUI: ${LCS_GUI}")
message("Build Type: ${CMAKE_BUILD_TYPE}")

add_compile_options(-Wall -Wextra)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -Og -Wpedantic)
else()
    add_compile_options(-O3)
endif()

include(FetchContent)
FetchContent_Declare(
    keychain
    GIT_REPOSITORY https://github.com/hrantzsch/keychain.git
    GIT_TAG v1.3.1
)
FetchContent_MakeAvailable(keychain)

set(JSONCPP_WITH_TESTS OFF)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.6
)
FetchContent_MakeAvailable(jsoncpp)

include_directories(${keychain_SOURCE_DIR}/include)
include_directories(${jsoncpp_SOURCE_DIR}/include)

include_directories(src/common)
include_directories(src/core)
add_subdirectory(src/common)
add_subdirectory(src/core)

if(LCS_GUI)
    include_directories(include)
    include_directories(src/api)
    add_subdirectory(src/api)
    include_directories(src/ui)
    add_subdirectory(src/ui)
    file(GLOB SOURCES src/main.cpp)
    add_executable(LogicCircuitSimulator ${SOURCES})
    target_link_libraries(LogicCircuitSimulator PRIVATE
        api
        common
        core
        jsoncpp_static
        keychain
        ui
        api)

    target_include_directories(LogicCircuitSimulator PRIVATE
        include
    )
endif()
if(LCS_BUILD_TESTS)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.12
    )
    FetchContent_MakeAvailable(doctest)
    file(GLOB TESTS src/main.cpp test/*.cpp)
    add_executable(LogicCircuitSimulator.tst ${TESTS})
    target_link_libraries(LogicCircuitSimulator.tst PRIVATE
        common
        core
        jsoncpp_static
        doctest::doctest)

    target_include_directories(LogicCircuitSimulator.tst PRIVATE
        ${doctest_SOURCE_DIR}/doctest
    )

    target_compile_definitions(LogicCircuitSimulator.tst
        PRIVATE
         __TESTING__=1
    )

    add_custom_target(run_tests
        COMMAND LogicCircuitSimulator.tst
        DEPENDS LogicCircuitSimulator.tst
        COMMENT "Running tests after build"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND LCS_GUI)
        add_dependencies(${PRJ} run_tests)
    endif()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND LCS_ENABLE_DOXYGEN)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)

        FetchContent_Declare(
            doxygen-awesome-css
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
            GIT_TAG v2.3.4
        )
        FetchContent_MakeAvailable(doxygen-awesome-css)
        
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/docs)
        message("${DOX} ${DOXYFILE_IN} ${DOXYFILE_OUT} Doxfile at: ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
        file(MAKE_DIRECTORY ${DOXYGEN_OUT})
        # Configure Doxygen output directory
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_OUTPUT_DIRECTORY ${DOXYGEN_OUT})
        # Add a custom target to generate documentation
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
        if(LCS_GUI)
            add_dependencies(${PROJECT_NAME} doc)
        else()
            add_dependencies(${PROJECT_NAME}.tst doc)
        endif()
    endif()
endif()
