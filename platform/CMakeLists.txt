project(platform)

option(PL_USE_GRAPHICS "Graphics" ON)
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
