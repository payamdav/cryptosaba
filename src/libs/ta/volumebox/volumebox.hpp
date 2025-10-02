#pragma once
#include "../../core/pubsub/pubsub.hpp"
#include <boost/circular_buffer.hpp>
#include "../../ta/frames/frames.hpp"
#include <string>

class Vols {
    public:
        double v=0;
        double vs=0;
        double vb=0;
        double vd=0;
};


class VBox : public boost::circular_buffer<Vols> {
    private:
        PubSub& pubsub = PubSub::getInstance();
        Frames * frames;
        size_t back_count;
        double upper_price_percent;
        double lower_price_percent;
        bool publish_appends = false;
        std::string topic_appends = "volumebox_append";
        vector<double> kernel_values;

    public:


        VBox(Frames * frames, size_t back_count=60, double upper_price_percent=1.0, double lower_price_percent=1.0, std::string kernel = "uniform");
        void new_frame();
        VBox * set_publish_appends(std::string topic = "volumebox_append");


};
