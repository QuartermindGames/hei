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

option(PL_USE_GRAPHICS "Graphics" ON)
option(PL_SUPPORT_VULKAN "Vulkan API support" OFF)
option(PL_COMPILE_STATIC "Compile as static library" OFF)
option(PL_USE_MODEL "Model" ON)

add_definitions("-D_DEBUG")

file(
        GLOB PLATFORM_SOURCE_FILES

        platform.c
        platform_console.c
        platform_filesystem.c
        platform_memory.c
        platform_parser.c

        LibraryLoader.c
        llist.c
        polygon.c
        math_matrix.c
        math_vector.c
        physics.c

        string/*.c string/*.h
        memory/*.*

        include/*.h
        include/PL/*.h
)

file(GLOB PLATFORM_PACKAGE_FILES package/*.*)

if (PL_USE_MODEL)
    file(GLOB PLATFORM_MODEL_FILES model/*.*)
endif ()

file(GLOB PLATFORM_IMAGE_FILES image/*.*)

if (PL_USE_GRAPHICS)
    file(GLOB PLATFORM_GRAPHICS_FILES graphics/*.*)
endif ()

if (PL_COMPILE_STATIC)
    add_library(platform STATIC
            ${PLATFORM_SOURCE_FILES}
            ${PLATFORM_GRAPHICS_FILES}
            ${PLATFORM_IMAGE_FILES}
            ${PLATFORM_MODEL_FILES}
            ${PLATFORM_PACKAGE_FILES})
else ()
    add_library(platform SHARED
            ${PLATFORM_SOURCE_FILES}
            ${PLATFORM_GRAPHICS_FILES}
            ${PLATFORM_IMAGE_FILES}
            ${PLATFORM_MODEL_FILES}
            ${PLATFORM_PACKAGE_FILES})
endif ()

target_compile_options(platform PUBLIC -DPL_INTERNAL)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(platform PUBLIC -fPIC)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(platform PUBLIC -D_CRT_SECURE_NO_WARNINGS /W3)
endif ()

if (PL_USE_GRAPHICS)
    add_definitions("-DPL_USE_GRAPHICS")

    if (PL_SUPPORT_VULKAN)
        add_definitions("-DPL_SUPPORT_VULKAN")

        find_package(Vulkan)
        if (NOT Vulkan_FOUND)
            find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS
                    "$ENV{VK_SDK_PATH}/Lib/"
                    "${CMAKE_SOURCE_DIR}/platform/3rdparty/vulkan/Lib/")
            find_path(Vulkan_INCLUDE_DIR PATHS
                    "$ENV{VK_SDK_PATH}/Include/"
                    "${CMAKE_SOURCE_DIR}/platform/3rdparty/vulkan/Include/")
        endif ()

        target_link_libraries(platform Vulkan_LIBRARY)
        target_include_directories(platform PRIVATE ${Vulkan_INCLUDE_DIR})
    endif (PL_SUPPORT_VULKAN)
endif ()

# Platform specific libraries should be provided here
if (UNIX)
    target_link_libraries(platform dl m)
elseif (WIN32)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(platform PRIVATE -static -static-libstdc++ -static-libgcc)
    endif ()
    target_link_libraries(platform Secur32 ws2_32 Psapi)
endif ()

set_property(TARGET platform PROPERTY C_STANDARD_REQUIRED 11)

target_include_directories(platform PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_SYSTEM_INCLUDE_PATH})
target_include_directories(platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
