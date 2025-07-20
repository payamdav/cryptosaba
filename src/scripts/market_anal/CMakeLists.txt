add_executable(market_anal market_anal_1.cpp)
target_link_libraries(market_anal utils trade logscale pubsub market zigzag frames stepper config)

add_executable(market_chart_data market_chart_data.cpp)
target_link_libraries(market_chart_data utils trade logscale pubsub market zigzag frames stepper volumebox config)
