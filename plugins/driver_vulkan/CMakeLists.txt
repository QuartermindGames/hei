#[[ Copyright (C) 2021 Mark E Sowden <markelswo@gmail.com> ]]

project(vulkan_driver)

add_library(
        vulkan_driver SHARED
        vulkan.c
        plugin.c
        plugin.h)

find_package(Vulkan)
if (NOT Vulkan_FOUND)
    message( "Found Vulkan!" )
    find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS
            "$ENV{VK_SDK_PATH}/Lib/"
            "${CMAKE_SOURCE_DIR}/platform/3rdparty/vulkan/Lib/")
    find_path(Vulkan_INCLUDE_DIR PATHS
            "$ENV{VK_SDK_PATH}/Include/"
            "${CMAKE_SOURCE_DIR}/platform/3rdparty/vulkan/Include/")
else ()
    message( "Failed to find Vulkan!" )
endif ()

target_link_libraries(platform Vulkan_LIBRARY)
target_include_directories(platform PRIVATE ${Vulkan_INCLUDE_DIR})

target_include_directories(
        vulkan_driver PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../plcore/include/"
)
