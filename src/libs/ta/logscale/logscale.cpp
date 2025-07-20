#include "logscale.hpp"
#include <cmath>
#include "../../core/config/config.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

Logscale::Logscale(double base_value, double multiplier) {
    this->base_value = base_value;
    this->multiplier = multiplier;
    this->log_base = 1.0 + multiplier;
    this->log_base_log2 = log2(log_base);
    this->base_value_log2 = log2(base_value);
}

Logscale::Logscale(string symbol) {
    Config & config = Config::getInstance();
    vector<string> symbols = config.get_csv_strings("symbols");
    vector<string> symbols_min_price = config.get_csv_strings("symbols_min_price");
    auto it = find(symbols.begin(), symbols.end(), symbol);
    if (it == symbols.end()) {
        cerr << "Logscale::Logscale: symbol not found: " << symbol << endl;
        throw runtime_error("Logscale::Logscale: symbol not found: " + symbol);
    }
    size_t index = distance(symbols.begin(), it);
    base_value = std::stod(symbols_min_price[index]);
    multiplier = 0.0001;
    log_base = 1.0 + multiplier;
    log_base_log2 = log2(log_base);
    base_value_log2 = log2(base_value);
}

double Logscale::scale(const double & value) {
    return (log2(value) - base_value_log2) / log_base_log2;
}

long Logscale::scale_int(const double & value) {
    return (long) ((log2(value) - base_value_log2) / log_base_log2);
}

double Logscale::operator () (const double & value) {
    return scale(value);
}

