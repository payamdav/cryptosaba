#pragma once
#include <time.h>
#include <string>
#include <vector>
using namespace std;

namespace utils {

struct YearMonthDay {
    int year;
    int month;
    int day;
};

time_t get_date(int year, int month, int day);
time_t get_datetime(int year, int month, int day, int hour, int minute, int second);
time_t epoch_date();
long long get_timestamp(int year, int month, int day, int hour, int minute, int second, int millisecond);
long long get_timestamp(const string & date);
string get_utc_datetime_string(long long ts);
vector<YearMonthDay> get_year_month_days(long long start_ts, long long end_ts);
}
