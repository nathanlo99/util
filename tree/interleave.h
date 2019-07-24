
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

inline uint64_t _interleave(const uint16_t x, const uint16_t y, const uint16_t z) {
    uint64_t result = 0;
    result = (result << 24) | mkeys[(z >> 8) & 0xFF] << 2 | mkeys[(y >> 8) & 0xFF] << 1 | mkeys[(x >> 8) & 0xFF];
    result = (result << 24) | mkeys[(z >> 0) & 0xFF] << 2 | mkeys[(y >> 0) & 0xFF] << 1 | mkeys[(x >> 0) & 0xFF];
    return result;
}

inline void _unpack(uint64_t val, uint16_t &x, uint16_t &y, uint16_t &z) {
    const uint64_t x_mask = 0b001001001001001001001001'001001001001001001001001;
    const uint64_t y_mask = 0b010010010010010010010010'010010010010010010010010;
    const uint64_t z_mask = 0b100100100100100100100100'100100100100100100100100;
    x = y = z = 0;
    for (int i = 15; i-->0; ) {
        x = 2 * x + ((val >> (3 * i + 0)) & 1);
        y = 2 * y + ((val >> (3 * i + 1)) & 1);
        z = 2 * z + ((val >> (3 * i + 2)) & 1);
    }
}

