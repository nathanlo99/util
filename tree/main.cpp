
#include <iostream>
#include <vector>
#include <bitset>
#include <chrono>
#include <cassert>

#include "interleave.h"
#include "tree.h"

std::ostream& operator<< (std::ostream &os, bytestream stream) {
    os << "stream{ ";
    for (byte b : stream) {
	os << std::bitset<8>((int)b) << ", ";
    }
    os << " }";
    return os;
}

uint16_t rnd() { return rand() & ((1 << 16) - 1); }

void benchmark_stdsortstream() {
    using milli = std::chrono::milliseconds;
    const size_t max_points = 10e6;
    std::vector<point> template_points(max_points, {0, 0, 0});
    for (size_t i = 0; i < template_points.size(); ++i)
        template_points[i] = {rnd(), rnd(), rnd()};

    for (size_t num_points : {10e3, 25e3, 10e4, 25e4, 10e5, 25e5, 10e6}) {
	const std::vector<point> points(template_points.begin(), template_points.begin() + num_points);
	const auto& start = std::chrono::high_resolution_clock::now();
	const bytestream& stream = stdSortStream(points);
	const auto& finish = std::chrono::high_resolution_clock::now();
	std::cout << "stdSortStream() with " << num_points << " points took "
		  << std::chrono::duration_cast<milli>(finish - start).count()
		  << " milliseconds\n";
        // assert(validByteStream(stream));
    }
}

void benchmark_radixsortstream() {
    using milli = std::chrono::milliseconds;
    const size_t max_points = 10e6;
    std::vector<point> template_points(max_points, {0, 0, 0});
    for (size_t i = 0; i < template_points.size(); ++i)
        template_points[i] = {rnd(), rnd(), rnd()};

    for (size_t num_points : {10e3, 25e3, 10e4, 25e4, 10e5, 25e5, 10e6}) {
	const std::vector<point> points(template_points.begin(), template_points.begin() + num_points);
	const auto& start = std::chrono::high_resolution_clock::now();
	const bytestream& stream = radixSortStream(points);	
	const auto& finish = std::chrono::high_resolution_clock::now();
	std::cout << "radixSortStream() with " << num_points << " points took "
		  << std::chrono::duration_cast<milli>(finish - start).count()
		  << " milliseconds\n";
        // assert(validByteStream(stream));
    }
}

int main() {
    _morton<255>::add_values(mkeys);
    benchmark_stdsortstream();
    benchmark_radixsortstream();
}
