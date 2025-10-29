set(CMAKE_C_STANDARD 23)

add_executable(qm-os-tests tests_os.c)
target_link_libraries(qm-os-tests qm-os)

add_test(NAME QmTestOsApi COMMAND qm-os-tests)
