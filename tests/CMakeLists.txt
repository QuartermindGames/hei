add_executable(tests_core test_core.c)
target_link_libraries(tests_core plcore)

add_executable(tests_model test_model.c)
target_link_libraries(tests_model plcore plmodel)

add_executable(tests_window ../tests/test_window.c)
target_link_libraries(tests_window plcore plgraphics plwindow)

add_test(NAME TestCoreAPI COMMAND tests_core)
add_test(NAME TestModelAPI COMMAND tests_model)
add_test(NAME TestWindowAPI COMMAND tests_window)
