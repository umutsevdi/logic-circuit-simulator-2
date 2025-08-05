FetchContent_Declare(
    tfd
    GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
)
FetchContent_MakeAvailable(tfd)

file(GLOB COMMON_RES 
    src/*.cpp
    ${tfd_SOURCE_DIR}/tinyfiledialogs.c
    ${tfd_SOURCE_DIR}/more_dialogs/tinyfd_moredialogs.c
)

find_package(CURL REQUIRED)
add_library(common ${COMMON_RES})

target_include_directories(common PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${tfd_SOURCE_DIR}
)

target_link_libraries(common PRIVATE
    ${CURL_LIBRARIES}
)
