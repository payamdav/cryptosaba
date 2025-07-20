#pragma once
#include <string>
#include <tuple>
#include <vector>
using namespace std;


namespace utils {
    template <typename T>
    string vector_to_json(vector<T> v) {
        string json = "[";
        for (size_t i = 0; i < v.size(); i++) {
            json += to_string(v[i]);
            if (i < v.size() - 1) {
                json += ", ";
            }
        }
        json += "]";
        return json;
    }
}

