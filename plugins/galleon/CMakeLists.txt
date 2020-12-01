#[[ Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> ]]

project( galleon_plugin )

add_library(
        galleon_plugin SHARED
        plugin.c
        plugin.h
        pkg.c)

set_target_properties(
        galleon_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins/"
)
target_include_directories(
        galleon_plugin PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../platform/include/"
)
