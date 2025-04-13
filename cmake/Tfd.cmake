file(GLOB TFD_RES
    external/tfd/tinyfiledialogs.c
    external/tfd/more_dialogs/tinyfd_moredialogs.c
)
add_library(tfd ${TFD_RES})
include_directories(external/tfd)
target_compile_options(tfd PRIVATE -O3
# no idea what this does but it fixes an instructions
    -no-pie)
