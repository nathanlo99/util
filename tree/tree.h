
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

bytestream stdSortStream(const std::vector<point> &points) {
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


template <typename T>
bytestream getOctreeStreamNaive(std::vector<point> points) {
    // Naive implementation: Insert points into an octree and BFS
    // - Used for benchmarking
    const T octree{points};
    return octree.bfs();
}

