if(LCS_BUILD_TESTS)
    project(${PRJ}.tst C CXX)
    set(LCS_TEST_DEP core common io jsoncpp_static base64)

    include_directories(include)
    include_directories(external/doctest/doctest)

    # Only build them if they are not built by the Application
    if(NOT LCS_GUI)
        include(cmake/Tfd.cmake)
        include(cmake/Json.cmake)
        include(cmake/Others.cmake)
        add_subdirectory(src/common)
        add_subdirectory(src/core)
        add_subdirectory(src/io)
    endif()
    add_subdirectory(external/doctest)

    file(GLOB TESTS src/main.cpp test/*.cpp)
    add_executable(${PRJ}.tst ${TESTS})
    target_link_libraries(
        ${PRJ}.tst
        ${LCS_TEST_DEP}
    doctest::doctest)
target_compile_definitions(${PRJ}.tst
    PRIVATE
     __TESTING__=1
)
    add_compile_definitions()

    add_custom_target(run_tests
        COMMAND ${PRJ}.tst
        DEPENDS ${PRJ}.tst
        COMMENT "Running tests after build"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND LCS_GUI)
        add_dependencies(${PRJ} run_tests)
    endif()
endif()
