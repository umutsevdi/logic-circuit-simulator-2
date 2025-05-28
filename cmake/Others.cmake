file(GLOB BASE64_RES external/cpp-base64/base64.cpp)
include_directories(external/cpp-base64)
add_library(base64 ${BASE64_RES})
