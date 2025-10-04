#include "pubsub.hpp"


PubSub& PubSub::getInstance() {
    static PubSub instance; // Guaranteed to be destroyed.
    return instance;        // Instantiated on first use.
}

void PubSub::subscribe(const std::string& topic, Callback callback) {
    subscribers[topic].push_back(callback);
}

void PubSub::publish(const std::string& topic, void* data) {
    auto it = subscribers.find(topic);
    if (it != subscribers.end()) {
        for (const auto& callback : it->second) {
            callback(data);
        }
    }
}

void PubSub::reset() {
    subscribers.clear();
}
