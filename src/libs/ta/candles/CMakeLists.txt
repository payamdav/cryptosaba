add_library(candles SHARED candles.cpp)
target_link_libraries(candles pubsub trade)
