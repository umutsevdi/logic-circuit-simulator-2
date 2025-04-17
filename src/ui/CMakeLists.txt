file(GLOB UI_RES ./*.cpp)
include_directories(../include/)
add_library(ui ${UI_RES})
target_link_libraries(ui
    tfd
    ${GTKMM_LIBRARIES}
)
