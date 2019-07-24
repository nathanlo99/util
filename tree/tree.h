
#pragma once

#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)

#include <unordered_set>
#include <set>
#include <iomanip>
#include <memory>
#include <functional>
#include <algorithm>
#include <vector>
#include <queue>
#include "interleave.h"

using byte = uint8_t;
using coord = uint32_t;
using point = std::array<coord, 3>;
using bytestream = std::vector<byte>;
using bitstream = std::vector<byte>;
using stream_istate = std::pair<byte, int>;

uint64_t interleave(const point &arr) {
    return _interleave(arr[0], arr[1], arr[2]);
}

struct OctreeNode {
    byte occupancy = 0;
    std::unique_ptr<OctreeNode> children[8] = {nullptr};
};

bool validByteStream(const bytestream &stream, const bitstream& bitstream) {
    OctreeNode root;
    std::deque<OctreeNode*> pq;
    pq.push_back(&root);
    auto stream_it = stream.cbegin();
    while (!pq.empty()) {
        OctreeNode* node = pq.front(); pq.pop_front();
        if (stream_it == stream.cend()) { 
            std::cout << "stream too short" << '\n';
            return 0;
        }
        node->occupancy = *stream_it++;
        for (size_t i = 0; i < 8; ++i) {
            if ((node->occupancy >> i) & 1) {
                node->children[i] = std::make_unique<OctreeNode>();
                pq.push_back(node->children[i].get());
            }
        }
    }
    return stream_it == stream.cend();
}

inline void bitstream_append(bitstream &bit_stream, stream_istate& state, const uint64_t val, int num_bits) {
    if (state.second + num_bits <= 8) {
        state.first = (state.first << num_bits) | val;
        state.second += num_bits;
        if (state.second == 8) {
            bit_stream.push_back(state.first);
            state.first = 0;
            state.second = 0;
        }
    } else {
        const int first_amt = 8 - state.second;
        byte to_write = (state.first << first_amt) | (val >> (num_bits - first_amt));
        bit_stream.push_back(to_write);
        num_bits -= first_amt;
        while (num_bits >= 8) {
            to_write = val >> (num_bits - 8);
            bit_stream.push_back(to_write);
            num_bits -= 8;
        }
        state.first = val & ((1 << num_bits) - 1);
        state.second = num_bits;
    }
}

inline void encode_leaf(bitstream &bit_stream, stream_istate& state, const int depth, const uint64_t val) {
    uint16_t x, y, z;
    _unpack(val, x, y, z);
    const uint64_t mask = (1LL << depth) - 1;
    uint64_t value = 0;
    value = (value << depth) | (x & mask);
    value = (value << depth) | (y & mask);
    value = (value << depth) | (z & mask);
    bitstream_append(bit_stream, state, value, 3 * depth);
}

std::pair<bytestream, bitstream> stdSortStream(const std::vector<point> &points) noexcept {
    bytestream bfs_stream;
    bfs_stream.reserve(2 * points.size()); // An upper bound for the memory usage.
    bitstream bit_stream;
    stream_istate bit_stream_state{0, 0};

    // Interleave the bits
    uint64_t *interleaved = new uint64_t[points.size()];
    std::transform(points.begin(), points.end(), interleaved, &interleave);
    // Sort and remove duplicates: now the elements, interpreted as octal literals, are in leaf-DFS order.
    std::sort(interleaved, interleaved + points.size()); 
    const int num_unique = std::unique(interleaved, interleaved + points.size()) - interleaved;
    bit_stream.reserve(num_unique * 12);
    
    // We instantiate a queue, holding elements of the form [lower, upper) and a bit depth to start looking for the octant
    // In particular, we've concluded that the bits [depth + 3 .. 45] are identical for all elements in [lower, upper).
    std::deque<std::tuple<int, int, int>> pq;
    pq.emplace_back(0, num_unique, 45);
    while (!pq.empty()) {
        const auto [lower, upper, depth] = pq.front(); pq.pop_front();
        // If lower == upper - 1, our range contains a single point, so here's where we encode the leaf (currently just appending (byte)0)
        if (lower == upper - 1) {
            bfs_stream.push_back(0);
            encode_leaf(bit_stream, bit_stream_state, depth, interleaved[lower]);
            continue;
        }
        // Here's the occupancy mask: the i-th bit corresponds to the existence of the i-th child.
        byte mask = 0;
        int last_start = lower;
        for (int i = lower; i < upper; ++i) {
            const int oct = (interleaved[i] >> depth) & 7;
            // If the current octant changed the occupancy mask, and our interval is non-trivial, then recurse.
            const byte new_mask = mask | (1 << oct);
            if (mask != new_mask && last_start != i) {
                pq.emplace_back(last_start, i, depth - 3);
                last_start = i;
            }
            mask = new_mask;
        }
        // Don't forget to add the final interval to the pq!
        pq.emplace_back(last_start, upper, depth - 3);
        // Finally, add the occupancy mask to the bytestream.
        bfs_stream.push_back(mask);
    }
    delete[] interleaved;
    return {bfs_stream, bit_stream};
}

std::pair<bytestream, bitstream> radixSortStream(const std::vector<point> &points) noexcept {
    // Prepare the stream
    bytestream stream;
    const int num_points = points.size();
    bitstream bit_stream;
    stream_istate bit_stream_state{0, 0};

    stream.reserve(2 * num_points); // An upper bound for the memory usage
    bit_stream.reserve(12 * num_points);

    // Interleave the bits
    uint64_t* data[2] = {new uint64_t[num_points], new uint64_t[num_points]};
    std::transform(points.begin(), points.end(), data[0], &interleave);
    
    
    // Because our implementation requires some scratch space, we instantiate two arrays of length n:
    // one of which is the 'data'(interleaved) array, and the other is the scratch array.
    // The `which` boolean indicates which array provides the data.
    // The queue contains tuples (lower, upper, depth) with the same meanings as in stdSortStream.
    std::deque<std::tuple<int, int, int, int> > pq;
    // These are counter arrays which are encountered in bucket sorting. 
    // We have 8 buckets and thus need 8 counters.
    int cnts[8], offsets[8];
    pq.emplace_back(0, 0, num_points, 45);
    while (!pq.empty()) {
        // Extract information from the queue
        const auto [which, lower, upper, depth] = pq.front(); pq.pop_front();

        const uint64_t* interleaved = data[which];
        uint64_t* scratch = data[1-which];
        
        for (int i = 0; i < 8; ++i) cnts[i] = 0;
        const uint64_t representative = interleaved[lower];
        int all_same = 1;
        
        // Count the number of elements in each bucket
        for (size_t i = lower; i < upper; ++i) {
            all_same &= (interleaved[i] == representative);
            const int oct = (interleaved[i] >> depth) & 7;
            ++cnts[oct];
        }
    
        // If all the elements in our set are the same, this is a leaf.
        if (unlikely(all_same)) {
            stream.push_back(0);
            encode_leaf(bit_stream, bit_stream_state, depth, representative);
            continue;
        }
        
        // Calculate the bucket offsets and the occupancy byte
        offsets[0] = lower;
        byte mask = !!cnts[0];
        for (int i = 1; i < 8; ++i) { 
            offsets[i] = offsets[i-1] + cnts[i-1];
            mask |= (!!cnts[i]) << i;
        }
        stream.push_back(mask);

        // NOTE: This will need to be changed to accomodate a different formatting of leaves.
        if (depth == 0) {
            for (int i = 0; i < 7; ++i) {
                if (cnts[i])
                    pq.emplace_back(1-which, offsets[i], offsets[i+1], 0); // Placeholder for a leaf.
            }
            if (cnts[7])
                pq.emplace_back(1-which, offsets[7], upper, 0);
            continue;
        }

        // Using the bucket offsets, place items into the appropriate slots in the scratch array
        for (int i = lower; i < upper; ++i) {
            const int oct = (interleaved[i] >> depth) & 7;
            const int idx = offsets[oct]++;
            scratch[idx] = interleaved[i];
        }
        // Recurse on the buckets **switching the roles of scratch and data**
        if (lower != offsets[0])
            pq.emplace_back(1-which, lower, offsets[0], depth - 3);
        for (int i = 1; i < 8; ++i)
            if (offsets[i-1] != offsets[i])
                pq.emplace_back(1-which, offsets[i-1], offsets[i], depth - 3);
    
    }
    delete[] data[0];
    delete[] data[1];
    // Once the queue is exhausted, the traversal is complete, so the bytestream is finished
    return {stream, {}};
}       

