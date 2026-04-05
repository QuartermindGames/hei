set(CMAKE_C_STANDARD 23)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(plgraphics STATIC
        private/plg_draw.c
        private/plg_driver_interface.c
        private/plg_mesh.c
        private/plg_texture.c

        private/qm_gfx.c
        private/qm_gfx_framebuffer.c
        private/qm_gfx_shader.c
)

target_include_directories(plgraphics PUBLIC include/)
target_link_libraries(plgraphics plcore)
