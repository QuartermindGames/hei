set(CMAKE_C_STANDARD 23)

add_library(qm-os STATIC
        private/qm_os_random.c
)

target_include_directories(qm-os PUBLIC ..)
