
#pragma once

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
        if (stream_it == stream.cend()) 
            return 0;
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
    bfs_stream.reserve(3 * points.size()); // A lower bound for the memory usage.
    const size_t n = points.size();
    // Interleave the bits
    std::vector<uint64_t> interleaved(n, 0);
    std::transform(points.begin(), points.end(), interleaved.begin(), &interleave);
    // Sort and remove duplicates: now the elements, interpreted as octal literals, are in leaf-DFS order.
    std::sort(interleaved.begin(), interleaved.end()); 
    interleaved.erase(std::unique(interleaved.begin(), interleaved.end()), interleaved.end());
    // We instantiate a queue, holding elements of the form [lower, upper) and a bit depth to start looking for the octant
    // In particular, we've concluded that the bits [depth + 3 .. 45] are identical for all elements in [lower, upper).
    std::queue<std::tuple<size_t, size_t, size_t>> pq;
    pq.push({0, interleaved.size(), 45});
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
            const size_t new_mask = mask | (1 << oct);
            // If the current octant changed the occupancy mask, and our interval is non-trivial, then recurse.
            if (mask != new_mask && last_start != i) {
                pq.push({last_start, i, depth - 3});
                last_start = i;
            }
            mask = new_mask;
        }
        // Don't forget to add the final interval to the pq!
        pq.push({last_start, upper, depth - 3});
        // Finally, add the occupancy mask to the bytestream.
        bfs_stream.push_back(mask);
    }
    return bfs_stream;
}

bytestream radixSortStream(const std::vector<point> &points) noexcept {
    // Prepare the stream
    bytestream stream;
    stream.reserve(3 * points.size());
    const size_t n = points.size();
    // Interleave the bits
    std::vector<uint64_t> data[2] = {std::vector<uint64_t>(n, 0), std::vector<uint64_t>(n, 0)};
    std::transform(points.begin(), points.end(), data[0].begin(), &interleave);
    
    // Because our implementation requires some scratch space, we instantiate two arrays of length n:
    // one of which is the 'data'(interleaved) array, and the other is the scratch array.
    // The `which` boolean indicates which array provides the data.
    // The queue contains tuples (which, lower, upper, depth) with the same meanings as in stdSortStream.
    std::queue<std::tuple<bool, size_t, size_t, size_t> > pq;
    // These are counter arrays which are encountered in bucket sorting. 
    // We have 8 buckets and thus need 8 counters.
    size_t cnts[8], offsets[8];
    pq.push({0, 0, n, 45});
    while (!pq.empty()) {
        // Extract information from the queue
        const auto [which, lower, upper, depth] = pq.front(); pq.pop();
        std::vector<uint64_t> &interleaved = data[which], &scratch = data[1-which];
        if (lower == upper) continue;
        // If the interval contains a single item, it is a leaf.
        if (lower == upper - 1) {
            stream.push_back(0);
            continue;
        }
        for (size_t i = 0; i < 8; ++i) cnts[i] = 0;
        // Count the number of elements in each bucket, and also update the occupancy mask
        for (size_t i = lower; i < upper; ++i) {
            const size_t oct = (interleaved[i] >> depth) & 7;
            ++cnts[oct];
        }
        
        // Calculate the bucket offsets
        offsets[0] = lower;
        for (size_t i = 1; i < 8; ++i) { 
            offsets[i] = offsets[i-1] + cnts[i-1];
        }
        
        // Calculate and push the occupancy byte
        byte mask = 0;
        for (size_t i = 0; i < 8; ++i) {
            mask |= (!!cnts[i]) << i;
        }
        stream.push_back(mask);

        // Using the bucket offsets, place items into the appropriate slots in the scratch array
        for (size_t i = lower; i < upper; ++i) {
            const size_t oct = (interleaved[i] >> depth) & 7;
            const size_t idx = offsets[oct]++;
            scratch[idx] = interleaved[i];
        }
        // Recurse on the buckets **switching the roles of scratch and data** (which' = 1-which)
        pq.push({1-which, lower, offsets[0], depth - 3});
        for (size_t i = 1; i < 8; ++i)
            pq.push({1-which, offsets[i-1], offsets[i], depth - 3});
    }
    // Once the queue is exhausted, the traversal is complete, so the bytestream is finished
    return stream;
}       

