file(GLOB ENGINE_RES ./*.cpp)
include_directories(.)
add_library(core ${ENGINE_RES})
target_link_libraries(core common)
