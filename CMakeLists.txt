cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LANGUAGE CXX)
set(CMAKE_BUILD_PARALLEL_LEVEL 4)
set(CMAKE_GENERATOR "Ninja")
set(CMAKE_LINKER_TYPE LLD)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


project(cryptosaba)

add_subdirectory(src/libs/utils)
add_subdirectory(src/libs/core/config)
add_subdirectory(src/libs/core/pubsub)
add_subdirectory(src/libs/trade)
add_subdirectory(src/libs/market)

add_subdirectory(src/libs/ta/logscale)
add_subdirectory(src/libs/ta/candles)
add_subdirectory(src/libs/ta/zigzag)
add_subdirectory(src/libs/ta/trendlines)
add_subdirectory(src/libs/ta/stepper)
add_subdirectory(src/libs/ta/frames)
add_subdirectory(src/libs/ta/volumebox)


add_subdirectory(src/scripts/tests)
add_subdirectory(src/scripts/market_anal)