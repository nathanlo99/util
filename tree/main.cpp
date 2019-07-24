
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

void benchmark_stdsortstream(const std::vector<point> template_points) {
    using milli = std::chrono::milliseconds;
    const size_t max_points = template_points.size();

    for (size_t num_points : {10e3, 25e3, 10e4, 25e4, 10e5, 25e5, 10e6, 25e6}) {
	const std::vector<point> points(template_points.begin(), template_points.begin() + num_points);
	const auto& start = std::chrono::high_resolution_clock::now();
	const bytestream& stream = stdSortStream(points);
	const auto& finish = std::chrono::high_resolution_clock::now();
	std::cout << "stdSortStream() with " << num_points << " points took "
		  << std::chrono::duration_cast<milli>(finish - start).count()
		  << " milliseconds\n";
	continue;
        assert(validByteStream(stream));
    }
}

void benchmark_radixsortstream(const std::vector<point> template_points) {
    using milli = std::chrono::milliseconds;
    const size_t max_points = template_points.size();
    for (size_t num_points : {10e3, 25e3, 10e4, 25e4, 10e5, 25e5, 10e6, 25e6}) {
	assert(num_points <= max_points);
	const std::vector<point> points(template_points.begin(), template_points.begin() + num_points);
	const auto& start = std::chrono::high_resolution_clock::now();
	const bytestream& radix_stream = radixSortStream(points);	
	const auto& finish = std::chrono::high_resolution_clock::now();
	std::cout << "radixSortStream() with " << num_points << " points took "
		  << std::chrono::duration_cast<milli>(finish - start).count()
		  << " milliseconds\n";
	continue;
        const bytestream& std_stream = stdSortStream(points);
	assert(validByteStream(std_stream));
        assert(validByteStream(radix_stream));
        const auto &[std_it, radix_it] = std::mismatch(std_stream.begin(), std_stream.end(), radix_stream.begin());
        const size_t first_mismatch = std_it - std_stream.begin();
	if (radix_stream != std_stream) {
	    std::cout << "First mismatch: " << first_mismatch << '\n';
	    if (first_mismatch != std_stream.size()) {
		std::cout << "(std)" << std::bitset<8>(*std_it) << " != " << "(rad)" << std::bitset<8>(*radix_it) << '\n';
	    }
	    std::cout << radix_stream.size() << ", " << std_stream.size() << '\n';
	    assert(radix_stream == std_stream);
	}
    }
}

int main() {
    _morton<255>::add_values(mkeys);
    const size_t max_points = 10e7;
    std::vector<point> template_points(max_points, {0, 0, 0});
    for (size_t i = 128; i < template_points.size(); ++i)
        template_points[i] = {rnd(), rnd(), rnd()};

    benchmark_stdsortstream(template_points);
    benchmark_radixsortstream(template_points);
}
