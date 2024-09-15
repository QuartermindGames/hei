add_executable(pl_example_triangle WIN32
        triangle.c
)

target_link_libraries(pl_example_triangle
        SDL2

        plcore
        plgraphics
)
