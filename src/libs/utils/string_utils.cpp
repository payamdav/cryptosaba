#include "string_utils.hpp"


namespace utils {

string ltrim(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    return (start == string::npos) ? "" : str.substr(start);
}

string rtrim(const string& str) {
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (end == string::npos) ? "" : str.substr(0, end + 1);
}

string trim(const string& str) {
    return rtrim(ltrim(str));
}

tuple<string, string> split(const string& str, const string& delimiter) {
    size_t pos = str.find(delimiter);
    if (pos == string::npos) {
        return make_tuple(str, "");
    }
    return make_tuple(str.substr(0, pos), str.substr(pos + delimiter.size()));
}

}

vector<string> utils::split_to_vector(const string& str, const string& delimiter) {
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.size();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

string utils::toLowerCase(const string& str) {
    string result = str;
    for (char& c : result) {
        c = tolower(c);
    }
    return result;
}
string utils::toUpperCase(const string& str) {
    string result = str;
    for (char& c : result) {
        c = toupper(c);
    }
    return result;
}

string utils::lpad(const string& str, char pad_char, size_t length) {
    if (str.length() >= length) {
        return str;
    }
    return string(length - str.length(), pad_char) + str;
}
string utils::rpad(const string& str, char pad_char, size_t length) {
    if (str.length() >= length) {
        return str;
    }
    return str + string(length - str.length(), pad_char);
}