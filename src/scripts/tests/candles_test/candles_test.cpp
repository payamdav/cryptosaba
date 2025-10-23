#include <string>
#include <iostream>
// #include "../../../libs/utils/timer.hpp"
#include "../../../libs/core/config/config.hpp"
#include "../../../libs/core/pubsub/pubsub.hpp"
#include "../../../libs/trade/trade.hpp"
#include "../../../libs/ta/candles/candles.hpp"
#include "../../../libs/utils/datetime_utils.hpp"


using namespace std;

void test1() {
    Config & config = Config::getInstance();
    string symbol = config.get("symbol");
    TradeReader trade_reader(symbol);
    Candles * candles_1 = (new Candles(1, 1000))->subscribe_to_pubsub_trades();
    Candles * candles_60 = (new Candles(60, 1000))->subscribe_to_pubsub_candles();

    trade_reader.pubsub_trades(config.get_timestamp("datetime1"), config.get_timestamp("datetime2"));

    cout << "All finished reading trades." << endl;
    cout << "Candles_1 size: " << candles_1->size() << endl;
    cout << "Candles_60 size: " << candles_60->size() << endl;
    cout << "Candles_1 first candle: " << candles_1->front() << endl;
    cout << "Candles_60 first candle: " << candles_60->front() << endl;
    cout << "Candles_1 last candle: " << candles_1->back() << endl;
    cout << "Candles_60 last candle: " << candles_60->back() << endl;
    cout << "Candles_1 duration: " << (candles_1->back().t - candles_1->front().t) / (1000 * 60 * 60) << " hours" << endl;
    cout << "Candles_60 duration: " << (candles_60->back().t - candles_60->front().t) / (1000 * 60 * 60) << " hours" << endl;
    cout << "Candles_1 Memory Used: " << (candles_1->size() * sizeof(Candle)) / (1024) << " KB" << endl;
    cout << "Candles_60 Memory Used: " << (candles_60->size() * sizeof(Candle)) / (1024) << " KB" << endl;
    cout << "Candles_1 capacity: " << candles_1->capacity() << endl;
    cout << "Candles_60 capacity: " << candles_60->capacity() << endl;
}


void write_candles_trades_to_binary_file() {
    Config & config = Config::getInstance();
    string symbol = config.get("symbol");
    TradeReader trade_reader(symbol);
    Candles * candles_1 = (new Candles(1, 1000))->subscribe_to_pubsub_trades();
    Candles * candles_60 = (new Candles(60, 1000))->subscribe_to_pubsub_candles();
    Candles * candles_3600 = (new Candles(3600, 1000))->subscribe_to_pubsub_candles();

    ofstream trade_file;
    ofstream candles_1_file;
    ofstream candles_60_file;
    ofstream candles_3600_file;

    trade_file.open(config.files_path + "trades.bin", ios::out | ios::binary);
    candles_1_file.open(config.files_path + "candles_1.bin", ios::out | ios::binary);
    candles_60_file.open(config.files_path + "candles_60.bin", ios::out | ios::binary);
    candles_3600_file.open(config.files_path + "candles_3600.bin", ios::out | ios::binary);

    PubSub& pubsub = PubSub::getInstance();
    pubsub.subscribe("trade", [&trade_file](void* data) {
        Trade* trade = static_cast<Trade*>(data);
        trade_file.write((char*)&trade->t, sizeof(trade->t));
        trade_file.write((char*)&trade->p, sizeof(trade->p));
    });

    pubsub.subscribe("candle_1", [&candles_1_file](void* data) {
        Candle* candle = static_cast<Candle*>(data);
        candles_1_file.write((char*)&candle->t, sizeof(candle->t));
        candles_1_file.write((char*)&candle->o, sizeof(candle->o));
        candles_1_file.write((char*)&candle->h, sizeof(candle->h));
        candles_1_file.write((char*)&candle->l, sizeof(candle->l));
        candles_1_file.write((char*)&candle->c, sizeof(candle->c));
    });

    pubsub.subscribe("candle_60", [&candles_60_file](void* data) {
        Candle* candle = static_cast<Candle*>(data);
        candles_60_file.write((char*)&candle->t, sizeof(candle->t));
        candles_60_file.write((char*)&candle->o, sizeof(candle->o));
        candles_60_file.write((char*)&candle->h, sizeof(candle->h));
        candles_60_file.write((char*)&candle->l, sizeof(candle->l));
        candles_60_file.write((char*)&candle->c, sizeof(candle->c));
    });

    pubsub.subscribe("candle_3600", [&candles_3600_file](void* data) {
        Candle* candle = static_cast<Candle*>(data);
        candles_3600_file.write((char*)&candle->t, sizeof(candle->t));
        candles_3600_file.write((char*)&candle->o, sizeof(candle->o));
        candles_3600_file.write((char*)&candle->h, sizeof(candle->h));
        candles_3600_file.write((char*)&candle->l, sizeof(candle->l));
        candles_3600_file.write((char*)&candle->c, sizeof(candle->c));
    });

    trade_reader.pubsub_trades(config.get_timestamp("datetime1"), config.get_timestamp("datetime2"));

    trade_file.close();
    candles_1_file.close();
    candles_60_file.close();
    candles_3600_file.close();

    cout << "All finished reading trades." << endl;
    cout << "Candles_1 size: " << candles_1->size() << endl;
    cout << "Candles_60 size: " << candles_60->size() << endl;
    cout << "Candles_3600 size: " << candles_3600->size() << endl;


}

void read_candles_from_binary_file(string symbol, size_t start_ts = 0, size_t end_ts = 0) {
    cout << "Reading candles binary file for symbol: " << symbol << " - from " << start_ts << " " << utils::get_utc_datetime_string(start_ts) << " to " << end_ts << " " << utils::get_utc_datetime_string(end_ts) << endl;

    CandlesVector candles(symbol, start_ts, end_ts);
    cout << "Read " << candles.size() << " candles from binary file." << endl;
    cout << "First Candle: " << candles.front() << endl;
    cout << "Last Candle: " << candles.back() << endl;
}

void test_candles_integrity_check(string symbol, size_t start_ts = 0, size_t end_ts = 0) {
    CandlesVector candles(1);
    candles.report_candles_integrity(symbol, start_ts, end_ts);
}

int main() {
    // test1();
    // write_candles_trades_to_binary_file();
    // read_candles_from_binary_file("btcusdt", utils::get_timestamp("2025-03-20 00:00:10"), utils::get_timestamp("2025-03-20 00:00:15"));

    test_candles_integrity_check("btcusdt");
    test_candles_integrity_check("ethusdt");
    test_candles_integrity_check("xrpusdt");
    test_candles_integrity_check("adausdt");
    test_candles_integrity_check("dogeusdt");
    test_candles_integrity_check("trumpusdt");
    test_candles_integrity_check("vineusdt");
    

    return 0;
}


