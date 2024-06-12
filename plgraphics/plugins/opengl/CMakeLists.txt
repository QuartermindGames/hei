project(pl_gl_driver)

set(GLEW_SOURCE
        3rdparty/glew-2.2.0/src/glew.c)

add_library(
        pl_gl_driver SHARED
        ${GLEW_SOURCE}
        opengl.c
        plugin.c
        plugin.h)

target_include_directories(pl_gl_driver PRIVATE
        3rdparty/glew-2.2.0/include
        ../../../plcore/include
        ../../include
)
if (WIN32)
    target_link_libraries(pl_gl_driver OpenGL32)
else ()
    target_link_libraries(pl_gl_driver GL)
endif ()
target_compile_definitions(pl_gl_driver PRIVATE GLEW_STATIC)
