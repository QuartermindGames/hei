#[[ Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> ]]

project( jade_plugin )

add_library(
        jade_plugin SHARED
        bf_loader.c
        plugin.c
        )

set_target_properties(
        jade_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins/"
)
target_include_directories(
        jade_plugin PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../platform/include/"
)
