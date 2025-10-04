// PubSub Class based on Meyers Singleton pattern
#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>


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
