set(JSONCPP_WITH_TESTS OFF)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

include_directories(external/jsoncpp/include)

add_subdirectory(external/jsoncpp)

# find_package(jsoncpp REQUIRED)
# include_directories(${jsoncpp_INCLUDE_DIRS})
# link_libraries(${jsoncpp_LIBRARIES})
