file(GLOB NET_RES ./*.cpp)
include_directories(../include/)
include_directories(external/jsoncpp/include)
add_library(net ${NET_RES})
target_link_libraries(net common io curl jsoncpp_static)
