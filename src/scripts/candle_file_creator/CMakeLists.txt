add_executable(candle_file_test_1 candle_file_test_1.cpp)
target_link_libraries(candle_file_test_1 utils config pubsub trade binance candles)

add_executable(candle_file_creator candle_file_creator.cpp)
target_link_libraries(candle_file_creator utils config pubsub trade binance candles)
