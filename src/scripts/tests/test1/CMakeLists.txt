add_executable(test1 test1.cpp)
target_link_libraries(test1 utils config pubsub trade logscale market zigzag frames stepper)

add_executable(utils_test utils_test.cpp)
target_link_libraries(utils_test utils)

