project(plgraphics)

option(PLG_COMPILE_STATIC "Compile as static library" ON)
option(PLG_BUILD_PLUGINS "Build all the provided plugins" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(GLOB PLG_SOURCE_FILES
        plg.c
        plg_camera.c
        plg_draw.c
        plg_driver_interface.c
        plg_layer_software.c
        plg_mesh.c
        plg_shader.c
        plg_texture.c
        polygon.c
)

if (PLG_COMPILE_STATIC)
    add_library(plgraphics STATIC ${PLG_SOURCE_FILES})
else ()
    add_library(plgraphics SHARED ${PLG_SOURCE_FILES})
endif ()

target_include_directories(plgraphics PUBLIC include/)
target_link_libraries(plgraphics plcore)

if (PLG_BUILD_PLUGINS)
    add_subdirectory(plugins/opengl)
    #add_subdirectory(plugins/vulkan)
    if (WIN32)
        add_subdirectory(plugins/d3d12)
        if (EXISTS "${CMAKE_SOURCE_DIR}/plugins/glide/CMakeLists.txt")
            add_subdirectory(plugins/glide)
        endif ()
    endif ()
endif ()
