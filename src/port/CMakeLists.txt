FetchContent_Declare(
    tfd
    GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
)
FetchContent_MakeAvailable(tfd)

file(GLOB PORT_RES 
    src/*.cpp
    ${tfd_SOURCE_DIR}/tfd/tinyfiledialogs.c
    ${tfd_SOURCE_DIR}/tfd/more_dialogs/tinyfd_moredialogs.c
)

find_package(CURL REQUIRED)
add_library(port ${PORT_RES})

target_include_directories(port PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${tfd_SOURCE_DIR}
)

target_link_libraries(port PRIVATE
    ${CURL_LIBRARIES}
    common
)
