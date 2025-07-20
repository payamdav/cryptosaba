#include "data_utils.hpp"


uint64_t utils::big_endian_to_little_endian(uint64_t x) {
    return ((x & 0x00000000000000FF) << 56) |
           ((x & 0x000000000000FF00) << 40) |
           ((x & 0x0000000000FF0000) << 24) |
           ((x & 0x00000000FF000000) << 8)  |
           ((x & 0x000000FF00000000) >> 8)  |
           ((x & 0x0000FF0000000000) >> 24) |
           ((x & 0x00FF000000000000) >> 40) |
           ((x & 0xFF00000000000000) >> 56);
}


uint32_t utils::big_endian_to_little_endian(uint32_t x) {
    return ((x & 0x000000FF) << 24) |
           ((x & 0x0000FF00) << 8) |
           ((x & 0x00FF0000) >> 8) |
           ((x & 0xFF000000) >> 24);
}

