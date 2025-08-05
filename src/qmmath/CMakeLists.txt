set(CMAKE_C_STANDARD 23)

add_library(qm-math STATIC
        public/qm_math.h
        public/qm_math_vector.h
)

target_include_directories(qm-math PUBLIC ..)
