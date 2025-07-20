#pragma once
#include <boost/circular_buffer.hpp>
#include <string>
#include "../../core/pubsub/pubsub.hpp"
#include <iostream>


class Step {
    public:
        size_t t = 0;
        double p = 0;
        bool h = false;

        Step(size_t t = 0, double p = 0, bool h = false);
};

std::ostream& operator<<(std::ostream& os, const Step& step);

class Stepper : public boost::circular_buffer<Step> {
    private:
        bool publish_appends = false;
        std::string topic_appends = "stepper_append";
        PubSub & pubsub;
    public:
        double d = 0.0001;

        Stepper(double delta, size_t retain_size = 1000);

        void push(size_t t, double p);
        Stepper * subscribe_to_pubsub();
        Stepper * subscribe_to_pubsub_frames_vwap(size_t miliseconds);
        Stepper * set_publish_appends(std::string topic = "stepper_append");
};
