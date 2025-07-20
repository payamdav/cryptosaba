#include "zigzag.hpp"
#include "../../trade/trade.hpp"
#include "../frames/frames.hpp"


Zig::Zig(size_t t, double p, bool h) {
    this->t = t;
    this->p = p;
    this->h = h;
}

std::ostream& operator<<(std::ostream& os, const Zig& zig) {
    os << "Zig: t=" << zig.t << ", p=" << zig.p << ", h=" << (zig.h ? "true" : "false");
    return os;
}

ZigZag::ZigZag(double delta, size_t retain_size) : boost::circular_buffer<Zig>(retain_size), d(delta), pubsub(PubSub::getInstance()) {
}

ZigZag * ZigZag::subscribe_to_pubsub() {
    pubsub.subscribe("trade", [this](void* data) { 
        Trade* trade = static_cast<Trade*>(data);
        this->push(trade->t, trade->p);
    });
    return this;
}

ZigZag * ZigZag::subscribe_to_pubsub_frames_vwap(size_t miliseconds) {
    pubsub.subscribe("frame_" + std::to_string(miliseconds), [this](void* data) { 
        Frame* frame = static_cast<Frame*>(data);
        this->push(frame->t, frame->vwap);
    });
    return this;
}

ZigZag * ZigZag::set_publish_appends(std::string topic) {
    publish_appends = true;
    topic_appends = topic;
    return this;
}

ZigZag * ZigZag::set_publish_updates(std::string topic) {
    publish_updates = true;
    topic_updates = topic;
    return this;
}

void ZigZag::push(size_t t, double p) {
    update_in_last_push = false;
    append_in_last_push = false;
    if (empty()) {
        if (first_low.t == 0) {
            first_low.t = t;
            first_low.p = p;
        }
        if (first_high.t == 0) {
            first_high.t = t;
            first_high.p = p;
        }
        if (p > first_high.p) {
            first_high.t = t;
            first_high.p = p;
        }
        if (p < first_low.p) {
            first_low.t = t;
            first_low.p = p;
        }
        if (first_low.t < first_high.t) {
            if (first_high.p > first_low.p * (1 + d)) {
                push_back(first_low);
                push_back(first_high);
                append_in_last_push = true;
                if (publish_appends) {
                    pubsub.publish(topic_appends, nullptr);
                }
            }
        }
        else {
            if (first_low.p < first_high.p * (1 - d)) {
                push_back(first_high);
                push_back(first_low);
                append_in_last_push = true;
                if (publish_appends) {
                    pubsub.publish(topic_appends, nullptr);
                }
            }
        }
    }
    else {
        Zig & last = back();
        if (last.h) {
            if (p > last.p) {
                last.t = t;
                last.p = p;
                update_in_last_push = true;
                if (publish_updates) {
                    pubsub.publish(topic_updates, nullptr);
                }
            } else if (p < last.p * (1 - d)) {
                push_back(Zig(t, p, false));
                append_in_last_push = true;
                if (publish_appends) {
                    pubsub.publish(topic_appends, nullptr);
                }
            }
        } else {
            if (p < last.p) {
                last.t = t;
                last.p = p;
                update_in_last_push = true;
                if (publish_updates) {
                    pubsub.publish(topic_updates, nullptr);
                }
            } else if (p > last.p * (1 + d)) {
                push_back(Zig(t, p, true));
                append_in_last_push = true;
                if (publish_appends) {
                    pubsub.publish(topic_appends, nullptr);
                }
            }
        }
    }
}
