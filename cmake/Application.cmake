if(LCS_GUI)
    project(${PRJ} C CXX)
    set(LCS_DEPENDS 
        core common io jsoncpp_static base64 ui net imnodes )

    include_directories(include)
    include(cmake/ImGui.cmake)
    include(cmake/Tfd.cmake)
    include(cmake/Json.cmake)
    include(cmake/Others.cmake)

    add_subdirectory(src/common)
    add_subdirectory(src/core)
    add_subdirectory(src/ui)
    add_subdirectory(src/net)
    add_subdirectory(src/io)


    file(GLOB SOURCES src/main.cpp)
    add_executable(${PRJ} ${SOURCES})

    target_link_libraries(${PRJ} ${LCS_DEPENDS})
    
    target_compile_definitions(${PRJ}
        PRIVATE
         __TESTING__=0
    )
endif()
