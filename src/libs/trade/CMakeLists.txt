add_library(trade SHARED trade.cpp tradecache.cpp)
target_link_libraries(trade PUBLIC utils pubsub)


