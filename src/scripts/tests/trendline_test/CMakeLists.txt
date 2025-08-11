add_executable(trendline_test trendline_test.cpp)
target_link_libraries(trendline_test utils config pubsub trade candles zigzag up_trend_line_zigzag market)

add_executable(trendline_test_2 trendline_test_2.cpp)
target_link_libraries(trendline_test_2 utils config pubsub trade candles zigzag up_trend_line_zigzag market)
