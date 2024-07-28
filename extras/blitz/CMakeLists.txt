add_library(blitz_plugin SHARED
        blitz.c
        blitz_format_dat.c
        blitz_format_psi.c
        blitz_format_spt.c
)

target_compile_definitions(blitz_plugin PRIVATE
        PL_COMPILE_PLUGIN)
target_include_directories(blitz_plugin PRIVATE
        ../../plcore/include/)

set_target_properties(
        blitz_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins/")
