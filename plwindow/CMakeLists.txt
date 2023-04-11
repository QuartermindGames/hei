option(PLW_COMPILE_STATIC "Compile as static library" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(GLOB PLW_SOURCE_FILES
        window.c)

if (PLW_COMPILE_STATIC)
    add_library(plwindow STATIC ${PLW_SOURCE_FILES})
else ()
    add_library(plwindow SHARED ${PLW_SOURCE_FILES})
endif ()

target_include_directories(plwindow PUBLIC include/)
target_link_libraries(plwindow plcore plgraphics)
