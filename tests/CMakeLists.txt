add_executable(tests_core test_core.c)
target_link_libraries(tests_core plcore)

add_executable(tests_model test_model.c)
target_link_libraries(tests_model plcore plmodel)
