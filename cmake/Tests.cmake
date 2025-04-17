if(LCS_BUILD_TESTS)
    project(${PRJ}.tst C CXX)
    file(GLOB TESTS test/*.cpp)
    add_subdirectory(external/catch2)
    add_executable(${PRJ}.tst ${TESTS})
    target_link_libraries(
        ${PRJ}.tst
        ${__CORE}
    Catch2::Catch2WithMain)

    add_custom_target(run_tests
        COMMAND ${PRJ}.tst
        DEPENDS ${PRJ}.tst
        COMMENT "Running tests after build"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND LCS_GUI)
        add_dependencies(${PRJ} run_tests)
    endif()
endif()
