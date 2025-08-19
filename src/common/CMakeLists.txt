
file(GLOB COMMON_RES src/*.cpp)

find_package(CURL REQUIRED)
add_library(common ${COMMON_RES})

target_include_directories(common PRIVATE
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(common PRIVATE
    ${CURL_LIBRARIES}
)
