file(GLOB NET_RES ./*.cpp)
add_library(net ${NET_RES})
target_link_libraries(net common jsoncpp_static)
