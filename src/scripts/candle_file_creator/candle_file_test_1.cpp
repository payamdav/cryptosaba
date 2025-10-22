#include <cstddef>
#include <iostream>
#include <string>
#include "../../libs/utils//datetime_utils.hpp"
#include "../../libs/utils//file_utils.hpp"
#include "../../libs/binance//binance.hpp"
#include "../../libs/ta/candles//candles.hpp"
#include "../../libs/core/pubsub//pubsub.hpp"

using namespace std;


void test_year_month_day() {
    using namespace utils;

    YearMonthDay ymd1(2023, 9, 25);
    YearMonthDay ymd2(2023, 10, 6);

    for (YearMonthDay ymd = ymd1; ymd <= ymd2; ymd = ymd + 1) {
        std::cout << "YearMonthDay: " << ymd.to_string() << std::endl;
    }

}

void test_year_month() {
    using namespace utils;

    YearMonth ym1(2023, 9);
    YearMonth ym2(2024, 3);

    for (YearMonth ym = ym1; ym <= ym2; ym = ym + 1) {
        std::cout << "YearMonth: " << ym.to_string() << std::endl;
    }
}

void test_binance_url(string symbol, int year, int month, int day=0) {
    std::cout << "Binance URL: " << binance_trade_futures_url(symbol, year, month, day) << std::endl;
}

void test_download_binance_trade_data(string symbol, int year, int month, int day=0) {
    std::cout << "\n=== Testing download for " << symbol << " ===" << std::endl;
    download_binance_trade_data(symbol, year, month, day);
}

void test_publish_trades_from_csv_file(string symbol, int year, int month, int day=0) {
    std::cout << "\n=== Testing publish trades from CSV for " << symbol << " ===" << std::endl;
    size_t publish_count = 0;
    // subscrijbe to trade pubsub to count published trades using a lambda function
    PubSub::getInstance().subscribe("trade", [&publish_count](const void* data) {
        publish_count++;
    });
    string csv_file_path = publish_trades_from_csv_file(symbol, year, month, day);
    std::cout << "Published trades from CSV file: " << csv_file_path << std::endl;
    std::cout << "Total published trades: " << publish_count << std::endl;
    // write line count
    size_t line_count = utils::count_lines_in_file(csv_file_path);
    std::cout << "Line count in CSV file: " << line_count << std::endl;
    // remove file
    // utils::remove_file(csv_file_path);
}

void test_publish_trades_from_csv_file_and_write_candles(string symbol, int year, int month, int day=0) {
    std::cout << "\n=== Testing publish trades from CSV for " << symbol << " ===" << std::endl;
    size_t publish_count = 0;
    CandleWriter candle_writer(symbol, 1);

    size_t first_trade_time = 0;
    size_t last_trade_time = 0;

    // subscrijbe to trade pubsub to count published trades using a lambda function
    PubSub::getInstance().subscribe("trade", [&publish_count, &candle_writer, &first_trade_time, &last_trade_time](const void* data) {
        publish_count++;
        Trade* trade_pointer = (Trade*)data;

        if (publish_count == 1) {
            first_trade_time = trade_pointer->t;
        }
        if (trade_pointer->t > last_trade_time) {
            last_trade_time = trade_pointer->t;
        }

        candle_writer.push(*trade_pointer);
    });
    string csv_file_path = publish_trades_from_csv_file(symbol, year, month, day);
    std::cout << "Published trades from CSV file: " << csv_file_path << std::endl;
    std::cout << "Total published trades: " << publish_count << std::endl;
    std::cout << "First trade time: " << first_trade_time << std::endl;
    std::cout << "Last trade time: " << last_trade_time << std::endl;
    // write line count
    size_t line_count = utils::count_lines_in_file(csv_file_path);
    std::cout << "Line count in CSV file: " << line_count << std::endl;
    candle_writer.write_current_candle_to_file();
    // remove file
    // utils::remove_file(csv_file_path);
}

void test_candles_binary_file(string symbol) {
    // cehck file size
    cout << "Reading candles binary file for symbol: " << symbol << " - file size: " << utils::get_file_size(candle_file_path_name(symbol, 1)) << " bytes" << " - count of candles: " << utils::get_file_size(candle_file_path_name(symbol, 1)) / Candle::buffer_size() << endl;

    CandlesVector candles_vector(1);
    candles_vector.read_from_binary_file_by_symbol(symbol);
    std::cout << "Total candles read from binary file: " << candles_vector.size() << std::endl;
    cout << "First candle: " << candles_vector.front() << std::endl;
    cout << "Last candle: " << candles_vector.back() << std::endl;

}


int main() {
    // test_year_month_day();
    // std::cout << std::endl;
    // test_year_month();
    // std::cout << std::endl;
    // test_binance_url("btcusdt", 2023, 9, 25);
    // test_binance_url("ethusdt", 2024, 1);
    // std::cout << std::endl;
    // test_download_binance_trade_data("vineusdt", 2025, 8, 15);
    // test_publish_trades_from_csv_file("vineusdt", 2025, 8, 15);
    test_publish_trades_from_csv_file_and_write_candles("vineusdt", 2025, 8, 18);
    test_candles_binary_file("vineusdt");


    return 0;
}

