# ┌───────────┐
# │ Build NFD │
# └───────────┘
set(NFD_PORTAL ON)
FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.2.1
)
FetchContent_MakeAvailable(nfd)

# ┌─────────────┐
# │ Build ImGUI │
# └─────────────┘
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.1-docking
)
FetchContent_MakeAvailable(imgui)

find_package(glfw3 REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
file(GLOB IMGUI_RES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_RES})
target_include_directories(imgui PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui glfw OpenGL::GL)


# ┌───────────────┐
# │ Build ImNodes │
# └───────────────┘
set(IMNODES_IMGUI_TARGET_NAME imgui)
FetchContent_Declare(
    imnodes
    GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
    GIT_TAG v0.5
)
file(GLOB_RECURSE UI_RES *.cpp ${imnodes_SOURCE_DIR}/imnodes.cpp
.cpp)
add_library(ui ${UI_RES})
target_include_directories(ui PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imnodes_SOURCE_DIR}
    ${nfd_SOURCE_DIR}
)

target_link_libraries(ui PRIVATE
    core
    imgui
    nfd
)

