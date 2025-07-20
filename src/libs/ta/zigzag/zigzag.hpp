#pragma once
#include <boost/circular_buffer.hpp>
#include <string>
#include "../../core/pubsub/pubsub.hpp"
#include <iostream>


class Zig {
    public:
        size_t t = 0;
        double p = 0;
        bool h = false;

        Zig(size_t t = 0, double p = 0, bool h = false);
};

std::ostream& operator<<(std::ostream& os, const Zig& zig);

class ZigZag : public boost::circular_buffer<Zig> {
    private:
        Zig first_low = {0, 0, false};
        Zig first_high = {0, 0, true};
        bool publish_appends = false;
        bool publish_updates = false;
        std::string topic_appends = "zigzag_append";
        std::string topic_updates = "zigzag_update";
        PubSub & pubsub;
    public:
        double d = 0.0001;

        bool update_in_last_push = false;
        bool append_in_last_push = false;

        ZigZag(double delta, size_t retain_size = 1000);

        void push(size_t t, double p);
        ZigZag * subscribe_to_pubsub();
        ZigZag * subscribe_to_pubsub_frames_vwap(size_t miliseconds);
        ZigZag * set_publish_appends(std::string topic = "zigzag_append");
        ZigZag * set_publish_updates(std::string topic = "zigzag_update");
};
