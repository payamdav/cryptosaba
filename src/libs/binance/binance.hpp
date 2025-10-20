#pragma once

#include <string>


using namespace std;


string binance_trade_futures_url(const string& symbol, int year, int month, int day=0);
string download_binance_trade_data(const string& symbol, int year, int month, int day=0);
string publish_trades_from_csv_file(const string& symbol, int year, int month, int day=0);
