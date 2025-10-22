#include <cstddef>
#include <iostream>
#include <string>
#include "../../libs/utils//datetime_utils.hpp"
#include "../../libs/utils//file_utils.hpp"
#include "../../libs/binance//binance.hpp"
#include "../../libs/ta/candles//candles.hpp"
#include "../../libs/core/pubsub//pubsub.hpp"

using namespace std;



void test_candles_binary_file(string symbol) {
    // cehck file size
    cout << "Reading candles binary file for symbol: " << symbol << " - file size: " << utils::get_file_size(candle_file_path_name(symbol, 1)) << " bytes" << " - count of candles: " << utils::get_file_size(candle_file_path_name(symbol, 1)) / Candle::buffer_size() << endl;

    CandlesVector candles_vector(1);
    candles_vector.read_from_binary_file_by_symbol(symbol);
    std::cout << "Total candles read from binary file: " << candles_vector.size() << std::endl;
    cout << "First candle: " << candles_vector.front() << std::endl;
    cout << "Last candle: " << candles_vector.back() << std::endl;

}

void create_candle_file_by_monthly_files(string symbol, int y1, int m1, int y2, int m2) {
    utils::YearMonth start_ym(y1, m1);
    utils::YearMonth end_ym(y2, m2);

    CandleWriter candle_writer(symbol, 1);
    PubSub::getInstance().subscribe("trade", [&candle_writer](const void* data) {
        Trade* trade_pointer = (Trade*)data;
        candle_writer.push(*trade_pointer);
    });

    for (utils::YearMonth ym = start_ym; ym <= end_ym; ym = ym + 1) {
        string csv_file_path = publish_trades_from_csv_file(symbol, ym.year, ym.month);
        utils::remove_file(csv_file_path);
    }

    candle_writer.write_current_candle_to_file();
}

void create_candle_file_by_daily_files(string symbol, int y1, int m1, int d1, int y2, int m2, int d2) {
    utils::YearMonthDay start_ymd(y1, m1, d1);
    utils::YearMonthDay end_ymd(y2, m2, d2);

    CandleWriter candle_writer(symbol, 1);
    PubSub::getInstance().subscribe("trade", [&candle_writer](const void* data) {
        Trade* trade_pointer = (Trade*)data;
        candle_writer.push(*trade_pointer);
    });

    for (utils::YearMonthDay ymd = start_ymd; ymd <= end_ymd; ymd = ymd + 1) {
        string csv_file_path = publish_trades_from_csv_file(symbol, ymd.year, ymd.month, ymd.day);
        utils::remove_file(csv_file_path);
    }

    candle_writer.write_current_candle_to_file();
}

int main(int argc, char* argv[]) {
    if (argc == 6) { // <symbol> <start_year> <start_month> <end_year> <end_month>
        string symbol = argv[1];
        int start_year = stoi(argv[2]);
        int start_month = stoi(argv[3]);
        int end_year = stoi(argv[4]);
        int end_month = stoi(argv[5]);
        create_candle_file_by_monthly_files(symbol, start_year, start_month, end_year, end_month);
        test_candles_binary_file(symbol);
    }
    else if (argc == 8) { // <symbol> <start_year> <start_month> <start_day> <end_year> <end_month> <end_day>
        string symbol = argv[1];
        int start_year = stoi(argv[2]);
        int start_month = stoi(argv[3]);
        int start_day = stoi(argv[4]);
        int end_year = stoi(argv[5]);
        int end_month = stoi(argv[6]);
        int end_day = stoi(argv[7]);
        create_candle_file_by_daily_files(symbol, start_year, start_month, start_day, end_year, end_month, end_day);
        test_candles_binary_file(symbol);
    }
    else {
        std::cout << "Usage: " << argv[0] << " <symbol> <start_year> <start_month> <end_year> <end_month>" << std::endl;
        std::cout << "Usage: " << argv[0] << " <symbol> <start_year> <start_month> <start_day> <end_year> <end_month> <end_day>" << std::endl;
    }



    return 0;
}

