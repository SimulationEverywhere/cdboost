find_package(Catch2 3 REQUIRED)
include(Catch)

set(TEST_FILES
    pdevs_coordinator_test.cpp
    pdevs_coupled_test.cpp
    pdevs_debug_test.cpp
    pdevs_generator_test.cpp
    pdevs_infinite_counter_test.cpp
    pdevs_istream_test.cpp
    pdevs_processor_test.cpp
    pdevs_runner_test.cpp
    pdevs_simulator_test.cpp
)

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE cdboost Catch2::Catch2WithMain)
    catch_discover_tests(${TEST_NAME})
endforeach()
