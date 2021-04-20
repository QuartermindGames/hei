#[[ Copyright (C) 2021 Mark E Sowden <markelswo@gmail.com> ]]

project(plgraphics)

add_library(plgraphics SHARED
        plg.c
        plg_camera.c
        plg_draw.c
        plg_layer_glide.c
        plg_layer_software.c
        plg_light.c
        plg_mesh.c
        plg_shader.c
        plg_texture.c
        polygon.c)

target_include_directories(plgraphics PRIVATE ../plcore/include/)
target_include_directories(plgraphics PUBLIC include/)

target_link_libraries(plgraphics plcore)
