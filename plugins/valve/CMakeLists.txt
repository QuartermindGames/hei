#[[ Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> ]]

project( valve_plugin )

add_library(
        valve_plugin SHARED
        plugin.c plugin.h
        vtf.c
        )

set_target_properties(
        valve_plugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/plugins/"
)
target_include_directories(
        valve_plugin PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../platform/include/"
)
