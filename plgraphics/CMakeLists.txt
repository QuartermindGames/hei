set(CMAKE_C_STANDARD 23)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(QM_GFX_OPENGL "Include OpenGL support." ON)
option(QM_GFX_VULKAN "Include Vulkan support." OFF)

add_library(plgraphics STATIC
        private/plg_draw.c
        private/plg_driver_interface.c
        private/plg_texture.c

        private/qm_gfx.c
        private/qm_gfx_framebuffer.c
        private/qm_gfx_mesh.c
        private/qm_gfx_shader.c

        private/driver_vulkan/vulkan.c
)

if(QM_GFX_VULKAN)
    target_compile_definitions(plgraphics PRIVATE QM_GFX_VULKAN)
endif()

target_include_directories(plgraphics PUBLIC include/)
target_link_libraries(plgraphics plcore)
