#pragma once
#include "../../core/pubsub/pubsub.hpp"
#include "../../trade/tradecache.hpp"
#include <boost/circular_buffer.hpp>
#include <iostream>


class Frame {
    public:
        size_t t=0;
        double h=0;
        double l=0;
        double vwap=0;
        size_t n=0;
        double v=0;
        double vs=0;
        double vb=0;
        double q=0;
};

std::ostream& operator<<(std::ostream& os, const Frame& frame);

class Frames : public boost::circular_buffer<Frame> {
    private:
        PubSub& pubsub = PubSub::getInstance();
        TradeCache& trade_cache = TradeCache::getInstance();
        
    public:
        size_t miliseconds;

        Frames(size_t size, size_t miliseconds);
        void new_slot();
};

