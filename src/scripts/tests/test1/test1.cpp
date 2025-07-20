#include <string>
#include <iostream>
#include <algorithm>
#include "../../../libs/utils/timer.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include "../../../libs/trade/trade.hpp"
#include <numeric>
#include <vector>
#include "../../../libs/market/market.hpp"
#include "../../../libs/ta/zigzag/zigzag.hpp"
#include "../../../libs/ta/stepper/stepper.hpp"
#include "../../../libs/trade/tradecache.hpp"
#include "../../../libs/ta/frames/frames.hpp"
#include "../../../libs/ta/logscale/logscale.hpp"


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


void tradereder_test() {
    string symbol = "btcusdt";
    TradeReader trade_reader(symbol);
    trade_reader.set_file_cursor(0);
    Trade trade;
    size_t count = trade_reader.count;

    cout << "Number of trades: " << count << endl;

    utils::Timer timer;

    for (size_t i = 0; i < count; ++i) {
        trade_reader.next(trade);
        if (i < 10) {
            cout << "Trade " << i << ": "
                 << "Price: " << trade.p
                 << ", Volume: " << trade.v
                 << ", Quantity: " << trade.q
                 << ", Time: " << trade.t
                 << ", Buyer Maker: " << (trade.is_buyer_maker ? "true" : "false")
                 << endl;
        }
    }
    
    timer.checkpoint("Finished reading trades");
}

class TradeCounter {
    public:
        size_t count;
        TradeCounter() : count(0) {
            PubSub& pubsub = PubSub::getInstance();
            pubsub.subscribe("trade", [this](void* data) { this->new_trade(data); });
            pubsub.subscribe("trade_finished", [this](void* data) { cout << "Trade finished" << endl; });
        }

        void new_trade(void * data) {
            Trade* trade = static_cast<Trade*>(data);
            count++;
        }
};


void test2() {
    string symbol = "btcusdt";
    TradeReader trade_reader(symbol);
    trade_reader.set_file_cursor(0);
    Trade trade;
    TradeCounter trade_counter;

    utils::Timer timer;

    for (size_t i = 0; i < trade_reader.count; ++i) {
        trade_reader.next(trade);
        trade_counter.new_trade(&trade);
    }
    
    timer.checkpoint("Finished reading trades");
    cout << "Total trades counted: " << trade_counter.count << endl;
}


void test3() {
    string symbol = "btcusdt";
    TradeCounter trade_counter;
    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");
    cout << "Total trades counted: " << trade_counter.count << endl;
}

void test4() {
    string symbol = "btcusdt";

    Market * market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    Market * market2 = (new Market("Market2"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    Market * market3 = (new Market("Market3"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    Market * market4 = (new Market("Market4"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    Market * market5 = (new Market("Market5"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();


    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");
}


void test5() {
    string symbol = "btcusdt";

    Market * market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    ZigZag * zigzag = (new ZigZag(0.01, 10))->set_publish_appends("zigzag_append")->subscribe_to_pubsub();

    PubSub& pubsub = PubSub::getInstance();
    pubsub.subscribe("zigzag_append", [zigzag](void* data) { 
        cout << "Market1 received zigzag append: " << zigzag->size() << endl;
    });
    
    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");
}

void test6() {
    string symbol = "btcusdt";

    Market * market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    TradeCache * trade_cache = &TradeCache::getInstance();
    trade_cache->subscribe_to_pubsub();

    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");

    cout << "TradeCache size: " << trade_cache->cache.size() << endl;
    cout << "TradeCache capacity: " << trade_cache->cache.capacity() << endl;
    cout << "TradeCache first trade: " << trade_cache->cache.front() << endl;
    cout << "TradeCache last trade: " << trade_cache->cache.back() << endl;
    cout << "TradeCache duration: " << (trade_cache->cache.back().t - trade_cache->cache.front().t) / (1000 * 60 * 60) << endl;
    cout << "TradeCache full duration in hours: " << trade_cache->get_full_duration_in_hours() << endl;
    cout << "TradeCache Memory Used: " << (trade_cache->cache.size() * sizeof(Trade)) / (1024 * 1024) << " MB" << endl;
    cout << "TradeCache Memory Used: " << trade_cache->get_memory_used_in_mb() << " MB" << endl;
    cout << "-------------------------" << endl;
    trade_cache->print_average_counts();
}


void test7() {
    string symbol = "btcusdt";

    Market * market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
    TradeCache2 * trade_cache = TradeCache2::getInstanceP();
    // trade_cache->add_index(1000, 3600)->add_index(5000, 12 * 60 * 4)->add_index(10000, 6 * 60 * 8)->add_index(30000, 2 * 60 * 24)->add_index(60000, 60 * 24)->add_index(300000, 12 * 24)->add_index(600000, 6 * 24)->add_index(1800000, 2 * 24)->add_index(3600000, 24);
    trade_cache->add_index(1000, 3600)->add_index(60000, 60 * 24);

    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");

    cout << "TradeCache size: " << trade_cache->cache.size() << endl;
    cout << "TradeCache capacity: " << trade_cache->cache.capacity() << endl;
    cout << "TradeCache first trade: " << trade_cache->cache.front() << endl;
    cout << "TradeCache last trade: " << trade_cache->cache.back() << endl;
    cout << "TradeCache duration: " << (trade_cache->cache.back().t - trade_cache->cache.front().t) / (1000 * 60 * 60) << endl;
    cout << "TradeCache full duration in hours: " << trade_cache->get_full_duration_in_hours() << endl;
    cout << "TradeCache Memory Used: " << (trade_cache->cache.size() * sizeof(Trade)) / (1024 * 1024) << " MB" << endl;
    cout << "TradeCache Memory Used: " << trade_cache->get_memory_used_in_mb() << " MB" << endl;
    cout << "-------------------------" << endl;
    trade_cache->print_average_counts();
}

void test8() {
    string symbol = "btcusdt";

    TradeCache& trade_cache = TradeCache::getInstance();
    trade_cache.subscribe_to_pubsub();
    Frames fh(1000, 3600000);
    Frames fm(60000, 60000);
    Frames fs(60000000, 1000);

    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");

    cout << "FH size: " << fh.size() << endl;
    cout << "FH capacity: " << fh.capacity() << endl;
    cout << "FH first frame: " << fh.front() << endl;
    cout << "FH last frame: " << fh.back() << endl;
    cout << "FH avg vwap: " << std::accumulate(fh.begin(), fh.end(), 0.0, [](double sum, const Frame& frame) { return sum + frame.vwap; }) / fh.size() << endl;
    cout << "FH Max H: " << std::max_element(fh.begin(), fh.end(), [](const Frame& a, const Frame& b) { return a.h < b.h; })->h << endl;
    cout << "FH Min L: " << std::min_element(fh.begin(), fh.end(), [](const Frame& a, const Frame& b) { return a.l < b.l; })->l << endl;
    cout << "--------------------------" << endl;
    cout << "FM size: " << fm.size() << endl;
    cout << "FM capacity: " << fm.capacity() << endl;
    cout << "FM first frame: " << fm.front() << endl;
    cout << "FM last frame: " << fm.back() << endl;
    cout << "FM avg vwap: " << std::accumulate(fm.begin(), fm.end(), 0.0, [](double sum, const Frame& frame) { return sum + frame.vwap; }) / fm.size() << endl;
    cout << "FM Max H: " << std::max_element(fm.begin(), fm.end(), [](const Frame& a, const Frame& b) { return a.h < b.h; })->h << endl;
    cout << "FM Min L: " << std::min_element(fm.begin(), fm.end(), [](const Frame& a, const Frame& b) { return a.l < b.l; })->l << endl;
    cout << "--------------------------" << endl;
    cout << "FS size: " << fs.size() << endl;
    cout << "FS capacity: " << fs.capacity() << endl;
    cout << "FS first frame: " << fs.front() << endl;
    cout << "FS last frame: " << fs.back() << endl;
    cout << "FS avg vwap: " << std::accumulate(fs.begin(), fs.end(), 0.0, [](double sum, const Frame& frame) { return sum + frame.vwap; }) / fs.size() << endl;
    cout << "FS Max H: " << std::max_element(fs.begin(), fs.end(), [](const Frame& a, const Frame& b) { return a.h < b.h; })->h << endl;
    cout << "FS Min L: " << std::min_element(fs.begin(), fs.end(), [](const Frame& a, const Frame& b) { return a.l < b.l; })->l << endl;
    cout << "FS Count n==0 : " << std::count_if(fs.begin(), fs.end(), [](const Frame& frame) { return frame.n == 0; }) << endl;
    

}

void test9() {
    string symbol = "btcusdt";

    TradeCache& trade_cache = TradeCache::getInstance();
    trade_cache.subscribe_to_pubsub();
    Frames fh(1000, 3600000);
    Frames fm(60000, 60000);
    ZigZag * zigzag_h = (new ZigZag(0.01, 100000))->set_publish_appends("zigzag_append")->subscribe_to_pubsub_frames_vwap(3600000);
    ZigZag * zigzag_m = (new ZigZag(0.01, 100000))->set_publish_appends("zigzag_append")->subscribe_to_pubsub_frames_vwap(60000);

    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");

    cout << "zigzag_h size: " << zigzag_h->size() << endl;
    cout << "zigzag_h capacity: " << zigzag_h->capacity() << endl;
    cout << "zigzag_h first zig: " << zigzag_h->front() << endl;
    cout << "zigzag_h last zig: " << zigzag_h->back() << endl;

    cout << "---------------------------" << endl;

    cout << "zigzag_m size: " << zigzag_m->size() << endl;
    cout << "zigzag_m capacity: " << zigzag_m->capacity() << endl;
    cout << "zigzag_m first zig: " << zigzag_m->front() << endl;
    cout << "zigzag_m last zig: " << zigzag_m->back() << endl;
    

 }

void test10() {
    string symbol = "btcusdt";

    TradeCache& trade_cache = TradeCache::getInstance();
    trade_cache.subscribe_to_pubsub();
    Frames fh(1000, 3600000);
    Frames fm(60000, 60000);
    ZigZag * zigzag_h = (new ZigZag(0.01, 100000))->set_publish_appends("zigzag_append")->subscribe_to_pubsub_frames_vwap(3600000);
    ZigZag * zigzag_m = (new ZigZag(0.01, 100000))->set_publish_appends("zigzag_append")->subscribe_to_pubsub_frames_vwap(60000);
    Stepper * stepper_h = (new Stepper(0.01, 100000))->set_publish_appends("stepper_append")->subscribe_to_pubsub_frames_vwap(3600000);
    Stepper * stepper_m = (new Stepper(0.01, 100000))->set_publish_appends("stepper_append")->subscribe_to_pubsub_frames_vwap(60000);

    TradeReader trade_reader(symbol);
    utils::Timer timer;
    trade_reader.pubsub_trades();
    timer.checkpoint("Finished reading trades");

    cout << "zigzag_h size: " << zigzag_h->size() << endl;
    cout << "zigzag_h capacity: " << zigzag_h->capacity() << endl;
    cout << "zigzag_h first zig: " << zigzag_h->front() << endl;
    cout << "zigzag_h last zig: " << zigzag_h->back() << endl;

    cout << "---------------------------" << endl;

    cout << "zigzag_m size: " << zigzag_m->size() << endl;
    cout << "zigzag_m capacity: " << zigzag_m->capacity() << endl;
    cout << "zigzag_m first zig: " << zigzag_m->front() << endl;
    cout << "zigzag_m last zig: " << zigzag_m->back() << endl;
    
    cout << "---------------------------" << endl;

    cout << "stepper_h size: " << stepper_h->size() << endl;
    cout << "stepper_h capacity: " << stepper_h->capacity() << endl;
    cout << "stepper_h first step: " << stepper_h->front() << endl;
    cout << "stepper_h last step: " << stepper_h->back() << endl;
    cout << "---------------------------" << endl;
    cout << "stepper_m size: " << stepper_m->size() << endl;
    cout << "stepper_m capacity: " << stepper_m->capacity() << endl;
    cout << "stepper_m first step: " << stepper_m->front() << endl;
    cout << "stepper_m last step: " << stepper_m->back() << endl;


}


int main() {
    // cout << "Running config test..." << endl;
    // config_test();

    // cout << "Running PubSub test..." << endl;
    // pubsub_test();

    // cout << "Running Timer test..." << endl;
    // timer_test();

    // cout << "Running TradeReader test..." << endl;
    // tradereder_test();

    // cout << "Running TradeCounter test..." << endl;
    // test2();

    // cout << "Running PubSub TradeReader test..." << endl;
    // test3();

    // cout << "Running Market PubSub TradeReader test..." << endl;
    // test4();

    // cout << "Running Market ZigZag PubSub TradeReader test..." << endl;
    // test5();

    // cout << "Running Market TradeCache PubSub TradeReader test..." << endl;
    // test6();

    // cout << "Running Market TradeCache2 PubSub TradeReader test..." << endl;
    // test7();

    // cout << "Running Frames PubSub TradeReader test..." << endl;
    // test8();

    // cout << "Running ZigZag PubSub Frames TradeReader test..." << endl;
    // test9();

    cout << "Running ZigZag and Stepper PubSub Frames TradeReader test..." << endl;
    test10();

    cout << "All tests completed." << endl;
    return 0;
}


