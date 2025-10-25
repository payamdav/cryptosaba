add_executable(candles_test candles_test.cpp)
target_link_libraries(candles_test utils config pubsub trade candles statistics)
