set(CMAKE_C_STANDARD 23)

add_library(qm-parse INTERFACE)

target_include_directories(qm-parse INTERFACE ..)

#############################################
# Tests
#############################################
#if (CMAKE_TESTING_ENABLED)
#    add_executable(qm-parse-test test/qm_parse_test.c)
#    target_link_libraries(qm-parse-test qm-parse)
#
#    add_test(NAME QmTestParseApi COMMAND qm-parse-test)
#endif()
