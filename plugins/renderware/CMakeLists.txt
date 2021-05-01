#[[ Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> ]]

project( renderware_plugin )

add_library(
        renderware_plugin SHARED
        plugin.c
        plugin.h
        pak.c)

set_target_properties(
        renderware_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins/"
)
target_include_directories(
        renderware_plugin PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../plcore/include/"
)
