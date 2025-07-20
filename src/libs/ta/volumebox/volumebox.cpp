#include "volumebox.hpp"
#include "../general/kernels.hpp"



VBox::VBox(Frames * frames, size_t back_count, double upper_price_percent, double lower_price_percent, std::string kernel) : boost::circular_buffer<Vols>(back_count) {
    this->frames = frames;
    this->back_count = back_count;
    this->upper_price_percent = upper_price_percent;
    this->lower_price_percent = lower_price_percent;
    this->kernel_values = get_kernel(back_count, kernel);
    pubsub.subscribe("frame_" + std::to_string(frames->miliseconds), [this](void* data) { this->new_frame(); });
}

void VBox::new_frame() {
    if (frames->size() < back_count) return;

    const Frame& frame = frames->back();
    const double upper_price_bound = frame.vwap * (1 + upper_price_percent);
    const double lower_price_bound = frame.vwap * (1 - lower_price_percent);
    double v = 0;
    double vs = 0;
    double vb = 0;
    double vd = 0;

    size_t start_index = frames->size() - back_count;

    for (size_t i = 0; i < back_count; ++i) {
        if (frames->at(start_index + i).vwap > lower_price_bound && frames->at(start_index + i).vwap < upper_price_bound) {
            v += frames->at(start_index + i).v * kernel_values[i];
            vs += frames->at(start_index + i).vs * kernel_values[i];
            vb += frames->at(start_index + i).vb * kernel_values[i];
        }
    }
    Vols vols;
    vols.v = v;
    vols.vs = vs;
    vols.vb = vb;
    vols.vd = vb - vs;

    this->push_back(vols);
    if (publish_appends) {
        pubsub.publish(topic_appends, &this->back());
    }

}

VBox * VBox::set_publish_appends(std::string topic) {
    publish_appends = true;
    topic_appends = topic;
    return this;
}

