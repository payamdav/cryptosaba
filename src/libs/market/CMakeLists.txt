add_library(market SHARED market.cpp)
target_link_libraries(market PUBLIC pubsub)

