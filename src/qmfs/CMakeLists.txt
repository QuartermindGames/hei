set(CMAKE_C_STANDARD 23)

add_library(qm-fs STATIC
        private/qm_fs.c
)

target_include_directories(qm-fs PUBLIC ..)

target_link_libraries(qm-fs qm-os)
