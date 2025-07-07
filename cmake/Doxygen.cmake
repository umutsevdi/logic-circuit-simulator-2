if(CMAKE_BUILD_TYPE STREQUAL "Release" AND LCS_ENABLE_DOXYGEN)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
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
            add_dependencies(${PROJECT_NAME} doc)
        endif()
    endif()
endif()
