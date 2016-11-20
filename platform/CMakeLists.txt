# Copyright (C) 2011-2016 Mark E Sowden
# Public Domain, do what you like. A pirate is you.

project(platform)

file(GLOB PLATFORM_SOURCE_FILES "*.cpp" "*.c")

add_library(platform STATIC ${PLATFORM_SOURCE_FILES})

set_target_properties(platform PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/platform/lib/)

target_compile_options(platform PUBLIC -fPIC)
target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SYSTEM_INCLUDE_PATH})