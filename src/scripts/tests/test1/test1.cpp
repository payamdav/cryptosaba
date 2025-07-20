#include <string>
#include <iostream>
#include <algorithm>
#include "../../../libs/utils/timer.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include <numeric>
#include <vector>


using namespace std;

void config_test() {
    Config& config = Config::getInstance();
    cout << "Home path: " << config.home_path << endl;
    cout << "Data path: " << config.data_path << endl;
    cout << "Files path: " << config.files_path << endl;
    vector<string> symbols = config.get_csv_strings("symbols");
    cout << "Symbols: ";
    for (const auto& symbol : symbols) {
        cout << symbol << " ";
    }
    cout << endl;
    cout << "Config keys: " << endl;
    for (const auto& kv : config.kv) {
        cout << kv.first << ": " << kv.second << endl;
    }
    cout << endl;
}

void pubsub_test() {
    PubSub& pubsub = PubSub::getInstance();
    pubsub.subscribe("test_topic", [](void* data) {
        string* message = static_cast<string*>(data);
        cout << "Received message: " << *message << endl;
    });

    string message = "Hello, PubSub!";
    pubsub.publish("test_topic", &message);
}

void timer_test() {
    utils::Timer timer("Test Timer");
    timer.checkpoint("Start");

    // Simulate some work
    for (int i = 0; i < 1000000; ++i);

    timer.checkpoint("After loop");
    cout << "Timer name: " << timer.name << endl;
}


int main() {
    cout << "Running config test..." << endl;
    config_test();
    cout << "Running PubSub test..." << endl;
    pubsub_test();
    cout << "Running Timer test..." << endl;
    timer_test();
    cout << "All tests completed." << endl;
    return 0;
}


