set(CMAKE_C_STANDARD 23)

add_library(qm-os STATIC
        private/qm_os_memory.c
        private/qm_os_random.c
        private/qm_os_string.c
        private/qm_os_time.c

        public/qm_os.h
        public/qm_os_random.h
        public/qm_os_time.h
)

target_include_directories(qm-os PUBLIC ..)
