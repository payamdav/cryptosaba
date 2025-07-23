#pragma once
#include <vector>
#include <deque>
#include "../../../core/pubsub/pubsub.hpp"
#include "../../zigzag/zigzag.hpp"
#include <iostream>

using namespace std;

class UpTrendLineZigZag {
    private:
        ZigZag * zigzag;
        PubSub & pubsub = PubSub::getInstance();

        deque<Zig> lows;
        deque<Zig> highs;
        bool exists = false;
        size_t start_t = 0;
        double start_p = 0.0;
        double slope = 0.0;
        double min_intercept = 0.0;
        double max_intercept = 0.0;
        
        void clear(string reason = "");

    public:
        double delta;
        double min_threshold;
        string publish_topic = "up_trend_line_zigzag";

        UpTrendLineZigZag(double delta = 0.0050, double min_threshold = 0.0001);
        void check();

};




