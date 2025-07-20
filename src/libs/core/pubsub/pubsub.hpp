// PubSub Class based on Meyers Singleton pattern
#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <memory>


using namespace std;

class PubSub {
    public:
        using Callback = std::function<void(void*)>;
        static PubSub& getInstance();
    private:
        PubSub() = default;
        PubSub(const PubSub&) = delete;
        PubSub& operator=(const PubSub&) = delete;
        std::unordered_map<std::string, std::vector<Callback>> subscribers;
    public:
        void subscribe(const std::string& topic, Callback callback);
        void publish(const std::string& topic, void* data);
        void reset();
};
