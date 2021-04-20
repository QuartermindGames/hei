#[[ Copyright (C) 2021 Mark E Sowden <markelswo@gmail.com> ]]

project(opengl_plugin)

add_library(
        opengl_plugin SHARED
        opengl.c
        plugin.c
        plugin.h)

function(build_glew)
    set(BUILD_UTILS OFF CACHE BOOL "utilities")
    add_subdirectory(3rdparty/glew-2.2.0/build/cmake/)
endfunction()

if (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    build_glew()
    target_link_libraries(opengl_plugin glew_s)
    target_include_directories(opengl_plugin PRIVATE
            3rdparty/glew-2.2.0/include/)
else ()
    target_link_libraries(opengl_plugin GLEW GL)
endif ()

target_include_directories(opengl_plugin PRIVATE ../../plcore/include/ ../../plgraphics/include/)
