if(LCS_GUI)
    set(IMGUI_DIR external/imgui)
    find_package(glfw3 REQUIRED)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    file(GLOB IMGUI_RES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

    add_library(imgui STATIC ${IMGUI_RES})
    include_directories(
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_link_libraries(imgui glfw OpenGL::GL)



endif()
