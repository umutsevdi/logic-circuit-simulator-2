
file(GLOB COMMON_RES src/*.cpp)

if (WIN32)
    find_package(Intl REQUIRED)
endif()
find_package(CURL REQUIRED)
add_library(common ${COMMON_RES})

target_include_directories(common PUBLIC
    ${CURL_INCLUDE_DIRS}
    ${Intl_INCLUDE_DIRS}
    ../../i18n/
)

target_link_libraries(common PUBLIC
    ${CURL_LIBRARIES}
    ${Intl_LIBRARY}
)
