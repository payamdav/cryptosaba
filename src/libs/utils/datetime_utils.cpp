#include "datetime_utils.hpp"

namespace utils {

time_t get_date(int year, int month, int day) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    return mktime(&t);
}

time_t get_datetime(int year, int month, int day, int hour, int minute, int second) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    return mktime(&t);
}

time_t epoch_date() {
    return get_date(1970, 1, 1);
}

long long get_timestamp(int year, int month, int day, int hour=0, int minute=0, int second=0, int millisecond=0) {
    auto t_day = get_datetime(year, month, day, hour, minute, second);
    auto epoch = epoch_date();
    return (t_day - epoch) * 1000 + millisecond;
}

long long get_timestamp(const string & date) {
    struct tm tm;
    strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
    return get_timestamp(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0);
}

string get_utc_datetime_string(long long ts) {
    time_t t = ts / 1000;
    struct tm *tm = gmtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return string(buf);
}


vector<YearMonthDay> get_year_month_days(long long start_ts, long long end_ts) {
    vector<YearMonthDay> result;
    // loop from start of day of start_ts to end_ts - with one day step (in milliseconds)
    for (long long ts = start_ts - (start_ts % 86400000); ts <= end_ts; ts += 86400000) {
        time_t t = ts / 1000;
        struct tm *tm = gmtime(&t);
        result.push_back({tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday});
    }
    return result;
}

}
