# Add test executable
add_executable(interstate3d_tests
    test_main.cpp
    config_manager_test.cpp
)

# Link test executable with GTest and project libraries
target_link_libraries(interstate3d_tests PRIVATE
    ${PROJECT_NAME}
    gtest_main
)

# Add test
add_test(NAME interstate3d_tests COMMAND interstate3d_tests)

# Make sure tests are built with the project
add_dependencies(${PROJECT_NAME} interstate3d_tests)
