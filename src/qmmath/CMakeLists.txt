set(CMAKE_C_STANDARD 23)

add_library(qm-math STATIC
        private/qm_math_plane.c
        private/qm_math_quaternion.c
        private/qm_math_vector.c

        public/qm_math.h
        public/qm_math_vector.h
)

target_include_directories(qm-math PUBLIC ..)
