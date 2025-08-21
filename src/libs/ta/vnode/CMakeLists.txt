add_library(vnode SHARED vnode.cpp)
target_link_libraries(vnode pubsub statistics)
