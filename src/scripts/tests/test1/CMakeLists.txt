add_executable(test1 test1.cpp)
target_link_libraries(test1 utils config pubsub trade logscale market zigzag frames stepper)
