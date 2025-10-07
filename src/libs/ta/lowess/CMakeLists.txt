add_library(lowess SHARED lowess.cpp)
target_link_libraries(lowess pubsub trade)
