#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include "../../libs/utils/datetime_utils.hpp"
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
#include "../../libs/ta/volumebox/volumebox.hpp"


using namespace std;


class Simulator {
    private:
        size_t start_ts = 0;
        size_t end_ts = 1900000000000;

    public:
        string symbol;
        utils::Timer timer;
        PubSub& pubsub = PubSub::getInstance();
        TradeCache * trade_cache = &TradeCache::getInstance();
        Market * market1;
        Frames * fs;
        Frames * fh;
        Frames * fm;
        ZigZag * zigzag_vwap_h;
        ZigZag * zigzag_vwap_m;
        Stepper * stepper;
        VBox * vbox;
        VBox * vbox2;

        
        size_t anal_point_count = 0;

        double average_price_movement = 100000000000;

        ofstream trade_file;
        ofstream zigzag_vwap_h_file;
        ofstream zigzag_vwap_m_file;
        ofstream stepper_file;
        ofstream vbox_file;
        ofstream vbox2_file;

        Simulator(string symbol, size_t start_ts=0, size_t end_ts=1900000000000) {
            this->start_ts = start_ts;
            this->end_ts = end_ts;
            pubsub.reset();
            // this->trade_cache->reset();
            this->trade_cache = &TradeCache::getInstance();
            this->trade_cache->subscribe_to_pubsub();
            

            this->symbol = symbol;
            this->market1 = (new Market("Market1"))->set_price_multiplier_to_handle_orders(0.0001)->set_commision(10)->subscribe_to_pubsub();
            this->fs = new Frames(5000, 1000);
            this->fh = new Frames(1000, 3600000);
            this->fm = new Frames(1000, 60000);
            this->zigzag_vwap_h = (new ZigZag(0.01, 100))->set_publish_appends("zigzag_vwap_h_append")->subscribe_to_pubsub_frames_vwap(3600000);
            this->zigzag_vwap_m = (new ZigZag(0.01, 100))->set_publish_appends("zigzag_vwap_m_append")->subscribe_to_pubsub_frames_vwap(60000);
            this->stepper = (new Stepper(0.0001, 100))->set_publish_appends("stepper")->subscribe_to_pubsub();
            // this->vbox = (new VBox(this->fs, 3600, 0.0200, 0.0200, "uniform"))->set_publish_appends("volumebox");
            // this->vbox = (new VBox(this->fs, 600, 0.0020, 0.0020, "ramp"))->set_publish_appends("volumebox");
            // this->vbox2 = (new VBox(this->fs, 1800, 0.0020, 0.0020, "ramp"))->set_publish_appends("volumebox2");

            this->vbox = (new VBox(this->fs, 600, 0.0005, 0.0005, "ramp"))->set_publish_appends("volumebox");
            this->vbox2 = (new VBox(this->fs, 3600, 0.0020, 0.0020, "ramp"))->set_publish_appends("volumebox2");

            pubsub.subscribe("stepper", [this](void* data) { if(this->fh->size() > 24) this->anal_point(); });

            // open files for writing in binary mode
            string trade_file_name = Config::getInstance().files_path + symbol + "_trades.bin";
            string zigzag_vwap_h_file_name = Config::getInstance().files_path + symbol + "_zigzag_vwap_h.bin";
            string zigzag_vwap_m_file_name = Config::getInstance().files_path + symbol + "_zigzag_vwap_m.bin";
            string stepper_file_name = Config::getInstance().files_path + symbol + "_stepper.bin";
            string vbox_file_name = Config::getInstance().files_path + symbol + "_vbox.bin";
            string vbox2_file_name = Config::getInstance().files_path + symbol + "_vbox2.bin";
            trade_file.open(trade_file_name, ios::out | ios::binary);
            zigzag_vwap_h_file.open(zigzag_vwap_h_file_name, ios::out | ios::binary);
            zigzag_vwap_m_file.open(zigzag_vwap_m_file_name, ios::out | ios::binary);
            stepper_file.open(stepper_file_name, ios::out | ios::binary);
            vbox_file.open(vbox_file_name, ios::out | ios::binary);
            vbox2_file.open(vbox2_file_name, ios::out | ios::binary);

            pubsub.subscribe("frame_60000", [this](void* data) {
                if (this->fm->size() < 121) return;
                double sum = 0;
                for (auto it = this->fm->end() - 120; it != this->fm->end(); ++it) {
                    sum += (abs(it->vwap - (it - 1)->vwap) / (it - 1)->vwap);
                }
                this->average_price_movement = sum / 120;
                // cout << "Average price movement: " << this->average_price_movement << endl;
                this->zigzag_vwap_m->d = this->average_price_movement * 3;
                // cout << "Average price movement: " << this->average_price_movement * 3 << endl;
            });

            pubsub.subscribe("trade", [this](void* data) {
                Trade trade = *(Trade*)data;
                this->trade_file.write((char*)&trade.t, sizeof(trade.t));
                this->trade_file.write((char*)&trade.p, sizeof(trade.p));
            });

            pubsub.subscribe("zigzag_vwap_h_append", [this](void* data) {
                if(this->zigzag_vwap_h->size() < 2) return;
                Zig zig = *(this->zigzag_vwap_h->end() - 2);
                this->zigzag_vwap_h_file.write((char*)&zig.t, sizeof(zig.t));
                this->zigzag_vwap_h_file.write((char*)&zig.p, sizeof(zig.p));
            });

            pubsub.subscribe("zigzag_vwap_m_append", [this](void* data) {
                if(this->zigzag_vwap_m->size() < 2) return;
                Zig zig = *(this->zigzag_vwap_m->end() - 2);
                this->zigzag_vwap_m_file.write((char*)&zig.t, sizeof(zig.t));
                this->zigzag_vwap_m_file.write((char*)&zig.p, sizeof(zig.p));
            });

            pubsub.subscribe("stepper", [this](void* data) {
                Step step = *(Step*)data;
                this->stepper_file.write((char*)&step.t, sizeof(step.t));
                this->stepper_file.write((char*)&step.p, sizeof(step.p));
            });

            pubsub.subscribe("trade_finished", [this](void* data) {
                // close all files
                this->trade_file.close();
                this->zigzag_vwap_h_file.close();
                this->zigzag_vwap_m_file.close();
                this->stepper_file.close();
            });

            pubsub.subscribe("volumebox", [this](void* data) {
                Vols vols = *(Vols*)data;
                size_t t = this->fs->back().t;
                this->vbox_file.write((char*)&t, sizeof(t));
                this->vbox_file.write((char*)&vols.v, sizeof(vols.v));
                this->vbox_file.write((char*)&vols.vs, sizeof(vols.vs));
                this->vbox_file.write((char*)&vols.vb, sizeof(vols.vb));
                this->vbox_file.write((char*)&vols.vd, sizeof(vols.vd));
            });

            pubsub.subscribe("volumebox2", [this](void* data) {
                Vols vols = *(Vols*)data;
                size_t t = this->fs->back().t;
                this->vbox2_file.write((char*)&t, sizeof(t));
                this->vbox2_file.write((char*)&vols.v, sizeof(vols.v));
                this->vbox2_file.write((char*)&vols.vs, sizeof(vols.vs));
                this->vbox2_file.write((char*)&vols.vb, sizeof(vols.vb));
                this->vbox2_file.write((char*)&vols.vd, sizeof(vols.vd));
            });

        }

        void run() {
            TradeReader trade_reader(symbol);
            trade_reader.pubsub_trades(start_ts, end_ts);
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

    // for (const auto &symbol : SYMBOLS) {
    //     cout << "Symbol: " << symbol << endl;
    //     Simulator simulator(symbol);
    //     simulator.run();
    //     cout << "-------------------------" << endl;
    // }

    Simulator simulator("adausdt", utils::get_timestamp("2025-03-12 00:00:00"), utils::get_timestamp("2025-03-13 02:00:00"));
    simulator.run();

    return 0;
}


