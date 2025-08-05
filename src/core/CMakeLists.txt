file(GLOB ENGINE_RES src/*.cpp)
add_library(core ${ENGINE_RES})
target_link_libraries(core common)
