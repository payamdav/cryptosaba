add_executable(test1 test1.cpp)
target_link_libraries(test1 utils config pubsub trade logscale market zigzag frames stepper)

add_executable(utils_test utils_test.cpp)
target_link_libraries(utils_test utils)

# add_executable(eigen_test_1 eigen_test_1.cpp)
# target_link_libraries(eigen_test_1 utils config pubsub trade Eigen3::Eigen)

# add_executable(eigen_compare_regression eigen_compare_regression.cpp)
# target_link_libraries(eigen_compare_regression utils config pubsub trade Eigen3::Eigen)
