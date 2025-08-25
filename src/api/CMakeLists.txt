file(GLOB api_RES src/*.cpp)
add_library(api ${api_RES})
target_link_libraries(api common jsoncpp_static)
