project(plcore)

option(PL_COMPILE_STATIC "Compile as static library" OFF)

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

file(GLOB PLATFORM_PACKAGE_FILES
        package/*.c package/*.h
        package/miniz/miniz.c
        package/miniz/miniz.h)

file(GLOB PLATFORM_IMAGE_FILES image/*.*)

if (PL_COMPILE_STATIC)
    add_library(plcore STATIC
            ${PLATFORM_SOURCE_FILES}
            ${PLATFORM_IMAGE_FILES}
            #${PLATFORM_MODEL_FILES}
            ${PLATFORM_PACKAGE_FILES})
else ()
    add_library(plcore SHARED
            ${PLATFORM_SOURCE_FILES}
            ${PLATFORM_GRAPHICS_FILES}
            ${PLATFORM_IMAGE_FILES}
            ${PLATFORM_PACKAGE_FILES})
endif ()

target_compile_options(plcore PUBLIC -DPL_INTERNAL)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(plcore PUBLIC -fPIC)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(plcore PUBLIC -D_CRT_SECURE_NO_WARNINGS /W3)
endif ()

# Platform specific libraries should be provided here
if (UNIX)
    target_link_libraries(plcore dl m)
elseif (WIN32)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(plcore PRIVATE -static -static-libstdc++ -static-libgcc)
    endif ()
    target_link_libraries(plcore Secur32 ws2_32 Psapi)
endif ()

set_property(TARGET plcore PROPERTY C_STANDARD_REQUIRED 11)

target_include_directories(plcore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_SYSTEM_INCLUDE_PATH})
target_include_directories(plcore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
