file(GLOB ENGINE_RES ./*.cpp)
include_directories(../include/)
add_library(core ${ENGINE_RES})
target_link_libraries(core common)
