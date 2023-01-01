project(core_plugin)
add_library(core_plugin SHARED
        core.c
        hg/core_hg_loader_clu.c
        hg/core_hg_loader_hgt.c)

target_compile_definitions(core_plugin PRIVATE
        PL_COMPILE_PLUGIN)
target_include_directories(core_plugin PRIVATE
        ../../plcore/include/)

set_target_properties(
        core_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins/")
