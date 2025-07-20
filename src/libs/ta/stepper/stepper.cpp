#include "stepper.hpp"
#include "../../trade/trade.hpp"
#include "../frames/frames.hpp"


Step::Step(size_t t, double p, bool h) {
    this->t = t;
    this->p = p;
    this->h = h;
}

std::ostream& operator<<(std::ostream& os, const Step& step) {
    os << "Step: t=" << step.t << ", p=" << step.p << ", h=" << (step.h ? "true" : "false");
    return os;
}

Stepper::Stepper(double delta, size_t retain_size) : boost::circular_buffer<Step>(retain_size), d(delta), pubsub(PubSub::getInstance()) {
}

Stepper * Stepper::subscribe_to_pubsub() {
    pubsub.subscribe("trade", [this](void* data) { 
        Trade* trade = static_cast<Trade*>(data);
        this->push(trade->t, trade->p);
    });
    return this;
}

Stepper * Stepper::subscribe_to_pubsub_frames_vwap(size_t miliseconds) {
    pubsub.subscribe("frame_" + std::to_string(miliseconds), [this](void* data) { 
        Frame* frame = static_cast<Frame*>(data);
        this->push(frame->t, frame->vwap);
    });
    return this;
}

Stepper * Stepper::set_publish_appends(std::string topic) {
    publish_appends = true;
    topic_appends = topic;
    return this;
}

void Stepper::push(size_t t, double p) {
    if (empty()) {
        this->push_back(Step(t, p, true));
        if (publish_appends) {
            pubsub.publish(topic_appends, &this->back());
        }
    } else {
        Step & last = back();
        double diff = p > last.p ? p - last.p : last.p - p;
        double percentage_diff = diff / last.p;
        if (percentage_diff > d) {
            this->push_back(Step(t, p, p > last.p));
            if (publish_appends) {
                pubsub.publish(topic_appends, &this->back());
            }
        }        
    }
}
