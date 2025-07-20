#pragma once
#include <string>
#include <tuple>
#include <vector>
using namespace std;


namespace utils {
    string ltrim(const string& str);
    string rtrim(const string& str);
    string trim(const string& str);
    tuple<string, string> split(const string& str, const string& delimiter);
    vector<string> split_to_vector(const string& str, const string& delimiter);
    string toLowerCase(const string& str);
    string toUpperCase(const string& str);
    string lpad(const string& str, char pad_char, size_t length);
    string rpad(const string& str, char pad_char, size_t length);
}

