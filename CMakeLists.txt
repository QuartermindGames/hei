cmake_minimum_required(VERSION 3.7)

project(PlatformLibrary)

option(BUILD_EXAMPLES "Build example applications" ON)
option(PL_BUILD_PLUGINS "Build plugins" ON)

# Set all of our output directories.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-function")

add_subdirectory(platform)
add_subdirectory(tests)

if (BUILD_EXAMPLES)
    add_subdirectory(examples/viewer)
    add_subdirectory(examples/package_loader)
    add_subdirectory(examples/pcmd)
endif (BUILD_EXAMPLES)

if (PL_BUILD_PLUGINS)
    add_subdirectory(plugins/driver_opengl/)
    #add_subdirectory(plugins/driver_vulkan/)

    #add_subdirectory(plugins/jade/)
    add_subdirectory(plugins/valve/)
    #add_subdirectory(plugins/galleon/)
    add_subdirectory(plugins/renderware/)
endif (PL_BUILD_PLUGINS)
