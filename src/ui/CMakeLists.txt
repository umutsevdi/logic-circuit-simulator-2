file(GLOB_RECURSE UI_RES ./*.cpp)
include_directories(../include/)
add_library(ui ${UI_RES})
target_link_libraries(ui
    core
    io
    tfd
    imgui
    imnodes
)
