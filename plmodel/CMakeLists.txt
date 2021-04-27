#[[ Copyright (C) 2021 Mark E Sowden <markelswo@gmail.com> ]]

project(plmodel)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(plmodel SHARED
        plm.c
        plm_format_cyclone.c
        plm_format_dpc.c
        plm_format_hdv.c
        plm_format_obj.c
        plm_format_smd.c
        plm_format_u3d.c)

target_include_directories(plmodel PUBLIC include/)
target_link_libraries(plmodel plcore plgraphics)
