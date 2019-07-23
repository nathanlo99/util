
#pragma once

#include <cstdint>
#include <array>

#define MAX_MORTON_KEY 1317624576693539401

std::array<uint32_t, 256> mkeys;

template <uint32_t i>
struct _morton {
    enum { value = (_morton<i-1>::value - MAX_MORTON_KEY) & MAX_MORTON_KEY };
    static void add_values(std::array<uint32_t, 256> &v) {
        _morton<i-1>::add_values(v);
        v[i] = value;
    }
};

template <>
struct _morton<0> {
    enum { value = 0 };
    static void add_values(std::array<uint32_t, 256> &v) {
        v[0] = value;
    }
};

inline uint64_t _interleave(uint32_t x, uint32_t y, uint32_t z) {
    uint64_t result = 0;
    result |= mkeys[(z >> 16) & 0xFF] << 2 | mkeys[(y >> 16) & 0xFF] << 1 | mkeys[(x >> 16) & 0xFF];
    result <<= 24;
    result |= mkeys[(z >> 8) & 0xFF] << 2 | mkeys[(y >> 8) & 0xFF] << 1 | mkeys[(x >> 8) & 0xFF];
    result <<= 24;
    result |= mkeys[(z >> 0) & 0xFF] << 2 | mkeys[(y >> 0) & 0xFF] << 1 | mkeys[(x >> 0) & 0xFF];
    return result;
}

