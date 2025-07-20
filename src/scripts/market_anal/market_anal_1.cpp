#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>
#include "../../libs/trade/trade.hpp"
#include "../../libs/utils/timer.hpp"
#include "../../libs/core/config/config.hpp"
#include "../../libs/ta/logscale/logscale.hpp"
#include "../../libs/core/pubsub/pubsub.hpp"
#include "../../libs/market/market.hpp"
#include "../../libs/ta/zigzag/zigzag.hpp"
#include "../../libs/ta/stepper/stepper.hpp"
#include "../../libs/trade/tradecache.hpp"
#include "../../libs/ta/frames/frames.hpp"


using namespace std;


class Simulator {
    private:

    public:
        string symbol;
        utils::Timer timer;
        PubSub& pubsub = PubSub::getInstance();
        TradeCache * trade_cache = &TradeCache::getInstance();
        Market * market1;
        Frames * fh;
        Frames * fm;
        ZigZag * zigzag_vwap_h;
        ZigZag * zigzag_vwap_m;
        Stepper * stepper;
        
        size_t anal_point_count = 0;

        Simulator(string symbol) {
            pubsub.reset();
            trade_cache->reset();
            trade_cache->subscribe_to_pubsub();
            

            this->symbol = symbol;
            this->market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
            this->trade_cache = &TradeCache::getInstance();
            this->fh = new Frames(1000, 3600000);
            this->fm = new Frames(1000, 60000);
            this->zigzag_vwap_h = (new ZigZag(0.01, 100))->set_publish_appends("zigzag_vwap_h_append")->subscribe_to_pubsub_frames_vwap(3600000);
            this->zigzag_vwap_m = (new ZigZag(0.01, 100))->set_publish_appends("zigzag_vwap_m_append")->subscribe_to_pubsub_frames_vwap(60000);
            this->stepper = (new Stepper(0.0001, 100))->set_publish_appends("stepper")->subscribe_to_pubsub();

            pubsub.subscribe("stepper", [this](void* data) { if(this->fh->size() > 24) this->anal_point(); });

        }

        void run() {
            TradeReader trade_reader(symbol);
            trade_reader.pubsub_trades();
            timer.checkpoint("Finished reading trades");
            finish();
        }

        void finish() {
            cout << "Anal point count: " << anal_point_count << endl;
        }

        void anal_point() {
            anal_point_count++;

        }

};

int main() {

    for (const auto &symbol : Config::getInstance().get_csv_strings("symbols")) {
        cout << "Symbol: " << symbol << endl;
        Simulator simulator(symbol);
        simulator.run();
        cout << "-------------------------" << endl;
    }

    return 0;
}


