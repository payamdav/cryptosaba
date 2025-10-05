add_executable(scene1 scene1.cpp)
target_link_libraries(scene1 utils config pubsub trade candles zigzag)

add_executable(scene_info scene_info.cpp)
target_link_libraries(scene_info utils config pubsub trade candles)
