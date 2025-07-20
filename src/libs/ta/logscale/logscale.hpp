#pragma once

#include <string>

using namespace std;


class Logscale {
    private:
        double log_base;
        double log_base_log2;
        double base_value_log2;
    public:
        double base_value;
        double multiplier;

        Logscale(double base_value=1, double multiplier=0.0001);
        Logscale(string symbol);
        double scale(const double & value);
        long scale_int(const double & value);
        double operator () (const double & value);
};
