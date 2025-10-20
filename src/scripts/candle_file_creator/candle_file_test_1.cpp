#include <iostream>
#include <string>
#include "../../libs//utils//datetime_utils.hpp"
#include "../../libs//utils//file_utils.hpp"
#include "../../libs//binance//binance.hpp"

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
    string csv_file_path = publish_trades_from_csv_file(symbol, year, month, day);
    std::cout << "Published trades from CSV file: " << csv_file_path << std::endl;
    // remove file
    utils::remove_file(csv_file_path);
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
    test_publish_trades_from_csv_file("vineusdt", 2025, 8, 15);

    return 0;
}

