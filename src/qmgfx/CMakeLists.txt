set(CMAKE_C_STANDARD 23)

add_library(qm-gfx STATIC
        private/qm_fs.c
)

target_include_directories(qm-gfx PUBLIC ..)

target_link_libraries(qm-gfx qm-os qm-math)
