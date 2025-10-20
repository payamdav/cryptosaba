add_library(binance SHARED binance.cpp)
target_link_libraries(binance PUBLIC utils curl pubsub config trade)

