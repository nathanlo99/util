
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

uint64_t interleave(const point &arr) {
    return _interleave(arr[0], arr[1], arr[2]);
}

struct OctreeNode {
    byte occupancy = 0;
    std::unique_ptr<OctreeNode> children[8] = {nullptr};
};

bool validByteStream(const bytestream &stream) {
    OctreeNode root;
    std::queue<OctreeNode*> pq;
    pq.push(&root);
    auto stream_it = stream.cbegin();
    while (!pq.empty()) {
        OctreeNode* node = pq.front(); pq.pop();
        if (stream_it == stream.cend()) { 
            std::cout << "stream too short" << '\n';
            return 0;
        }
        node->occupancy = *stream_it++;
        for (size_t i = 0; i < 8; ++i) {
            if ((node->occupancy >> i) & 1) {
                node->children[i] = std::make_unique<OctreeNode>();
                pq.push(node->children[i].get());
            }
        }
    }
    return stream_it == stream.cend();
}

bytestream stdSortStream(const std::vector<point> &points) noexcept {
    bytestream bfs_stream;
    bfs_stream.reserve(2 * points.size()); // An upper bound for the memory usage.
    // Interleave the bits
    std::vector<uint64_t> interleaved(points.size(), 0);
    std::transform(points.begin(), points.end(), interleaved.begin(), &interleave);
    // Sort and remove duplicates: now the elements, interpreted as octal literals, are in leaf-DFS order.
    std::sort(interleaved.begin(), interleaved.end()); 
    interleaved.erase(std::unique(interleaved.begin(), interleaved.end()), interleaved.end());
    // We instantiate a queue, holding elements of the form [lower, upper) and a bit depth to start looking for the octant
    // In particular, we've concluded that the bits [depth + 3 .. 45] are identical for all elements in [lower, upper).
    std::queue<std::tuple<size_t, size_t, size_t>> pq;
    pq.emplace(0, interleaved.size(), 45);
    while (!pq.empty()) {
        const auto [lower, upper, depth] = pq.front(); pq.pop();
        // If lower == upper - 1, our range contains a single point, so here's where we encode the leaf (currently just appending (byte)0)
        if (lower == upper - 1) {
            bfs_stream.push_back(0);
            continue;
        }
        // Here's the occupancy mask: the i-th bit corresponds to the existence of the i-th child.
        byte mask = 0;
        size_t last_start = lower;
        for (size_t i = lower; i < upper; ++i) {
            const size_t oct = (interleaved[i] >> depth) & 7;
            // If the current octant changed the occupancy mask, and our interval is non-trivial, then recurse.
            const byte new_mask = mask | (1 << oct);
            if (mask != new_mask && last_start != i) {
                pq.emplace(last_start, i, depth - 3);
                last_start = i;
            }
            mask = new_mask;
        }
        // Don't forget to add the final interval to the pq!
        pq.emplace(last_start, upper, depth - 3);
        // Finally, add the occupancy mask to the bytestream.
        bfs_stream.push_back(mask);
    }
    return bfs_stream;
}

bytestream radixSortStream(const std::vector<point> &points) noexcept {
    // Prepare the stream
    bytestream stream;
    stream.reserve(2 * points.size()); // An upper bound for the memory usage
    const size_t n = points.size();
    // Interleave the bits
    std::vector<uint64_t> data[2] = {std::vector<uint64_t>(n, 0), std::vector<uint64_t>(n, 0)};
    std::transform(points.begin(), points.end(), data[0].begin(), &interleave);
    
    // Because our implementation requires some scratch space, we instantiate two arrays of length n:
    // one of which is the 'data'(interleaved) array, and the other is the scratch array.
    // The `which` boolean indicates which array provides the data.
    // The queue contains tuples (lower, upper, depth) with the same meanings as in stdSortStream.
    std::queue<std::tuple<bool, size_t, size_t, size_t> > pq;
    // These are counter arrays which are encountered in bucket sorting. 
    // We have 8 buckets and thus need 8 counters.
    size_t cnts[8], offsets[8];
    pq.emplace(0, 0, n, 45);
    while (!pq.empty()) {
        // Extract information from the queue
        const auto [which, lower, upper, depth] = pq.front(); pq.pop();
        if (lower == upper - 1) {
            stream.push_back(0);
            continue;
        }

        const std::vector<uint64_t> &interleaved = data[which];
        std::vector<uint64_t> &scratch = data[1-which];
        
        for (size_t i = 0; i < 8; ++i) cnts[i] = 0;
        uint64_t representative = interleaved[lower];
        bool all_same = 1;
        
        // Count the number of elements in each bucket
        for (size_t i = lower; i < upper; ++i) {
            all_same &= (interleaved[i] == representative);
            const size_t oct = (interleaved[i] >> depth) & 7;
            ++cnts[oct];
        }
    
        // If all the elements in our set are the same, this is a leaf.
        if (all_same) {
            stream.push_back(0);
            continue;
        }
        
        // Calculate the bucket offsets and the occupancy byte
        offsets[0] = lower;
        byte mask = !!cnts[0];
        for (size_t i = 1; i < 8; ++i) { 
            offsets[i] = offsets[i-1] + cnts[i-1];
            mask |= (!!cnts[i]) << i;
        }
        stream.push_back(mask);

        // NOTE: This will need to be changed to accomodate a different formatting of leaves.
        if (depth == 0) {
            for (int i = 0; i < 8; ++i) {
                if (cnts[i] > 0)
                    pq.emplace(1-which, 0, 1, 0); // Placeholder for a leaf.
            }
        } else {
            // Using the bucket offsets, place items into the appropriate slots in the scratch array
            for (size_t i = lower; i < upper; ++i) {
                const size_t oct = (interleaved[i] >> depth) & 7;
                const size_t idx = offsets[oct]++;
                scratch[idx] = interleaved[i];
            }
            // Recurse on the buckets **switching the roles of scratch and data**
            if (lower != offsets[0])
                pq.emplace(1-which, lower, offsets[0], depth - 3);
            for (size_t i = 1; i < 8; ++i)
                if (offsets[i-1] != offsets[i])
                    pq.emplace(1-which, offsets[i-1], offsets[i], depth - 3);
        }
    }
    // Once the queue is exhausted, the traversal is complete, so the bytestream is finished
    return stream;
}       

