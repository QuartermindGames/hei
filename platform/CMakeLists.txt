#[[
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
]]

project(platform)

############################################################

option(PL_USE_GRAPHICS "Graphics" ON)

option(PL_SUPPORT_DIRECT3D "Graphics Direct3D API support" OFF)
option(PL_SUPPORT_GLIDE "Graphics 3dfx Glide API support" OFF)
option(PL_SUPPORT_OPENGL "Graphics OpenGL API support" ON)

############################################################

option(PL_USE_MODEL "Model" ON)

############################################################

add_definitions("-D_DEBUG")

file(
        GLOB PLATFORM_SOURCE_FILES
        *.cpp *.c *.h

        string/*.*
        memory/*.*

        include/*.h
        include/PL/*.h
        )

file(GLOB PLATFORM_PACKAGE_FILES package/*.*)

if(PL_USE_MODEL)
    file(GLOB PLATFORM_MODEL_FILES model/*.*)
endif()

file(GLOB PLATFORM_IMAGE_FILES image/*.*)

if(PL_USE_GRAPHICS)
    file(GLOB PLATFORM_GRAPHICS_FILES graphics/*.*)
endif()

add_library(platform STATIC
        ${PLATFORM_SOURCE_FILES}
        ${PLATFORM_GRAPHICS_FILES}
        ${PLATFORM_IMAGE_FILES}
        ${PLATFORM_MODEL_FILES}
        ${PLATFORM_PACKAGE_FILES})

#[[
set_target_properties(platform PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/platform/lib/
        C_STANDARD 11
        CXX_STANDARD 14
        )
]]

target_compile_options(platform PUBLIC -fPIC -DPL_INTERNAL)

function(build_glew)
    set(BUILD_UTILS OFF CACHE BOOL "utilities")
    add_subdirectory(3rdparty/glew-2.2.0/build/cmake)
endfunction()

if(PL_USE_GRAPHICS)
    add_definitions("-DPL_USE_GRAPHICS")

    if(PL_SUPPORT_OPENGL)
        add_definitions("-DPL_SUPPORT_OPENGL")
        if (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            build_glew()
            target_link_libraries(platform glew_s)
        else()
            target_link_libraries(platform GLEW GL)
        endif()
    endif()

    if(PL_SUPPORT_GLIDE)
        add_definitions("-DPL_SUPPORT_GLIDE")
    endif()

    if(PL_SUPPORT_DIRECT3D)
        add_definitions("-DPL_SUPPORT_DIRECT3D")
    endif()
endif()

# Platform specific libraries should be provided here
if(UNIX)
    target_link_libraries(platform dl)
elseif(WIN32)
    target_compile_options(platform PRIVATE -static -static-libstdc++ -static-libgcc)
    target_link_libraries(platform Secur32 ws2_32)
endif()

target_include_directories(platform PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_SYSTEM_INCLUDE_PATH})
target_include_directories(platform PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glew-2.2.0/include/)
target_link_libraries(platform m)
