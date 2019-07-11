
#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <memory>

static int debug_depth = 0;

static std::string debug_pad() { return std::string(debug_depth, '\t'); }
static void increase_depth() { debug_depth++; }
static void decrease_depth() { debug_depth--; }
static std::ostream& debug() { return std::cout << debug_pad(); }

template <typename T, unsigned int w>
class VEBtree
{
    static constexpr unsigned int half_w = w >> 1;
    static constexpr T mask = (1 << half_w) - 1;
    static constexpr T INFINITY = -1;

    T min = INFINITY, max = 0;
    std::unique_ptr<VEBtree<T, half_w> > summary;
    std::unordered_map<T, VEBtree<T, half_w> > clusters;

public:
    VEBtree() 
    {
	static_assert((w & (w - 1)) == 0, "w given to VEBtree was not power of 2");
    }

    VEBtree(T val): min{val}, max{val} {}
    
    std::vector<T> traverse()
    {
        std::vector<T> result;
        if (min == INFINITY) 
            return result;
        result.push_back(min);
        if (!summary) 
            return result;
        std::vector<T> summary_traversal = summary->traverse();
        for (T cluster : summary_traversal)
        {
            std::vector<T> cluster_traversal = clusters[cluster].traverse();
            for (T num : cluster_traversal)
                result.push_back((cluster << half_w) | num);
        }
        return result;
    }

    void insert(T val)
    {
        increase_depth();
        debug() << "Inserting " << val << std::endl;

	if (val > max)
	    max = val;
	if (empty()) 
	{
            debug() << "Inserting into empty" << std::endl;
	    min = val;
            decrease_depth();
	    return;
	}

	if (val < min) std::swap(val, min);
	const T cluster = val >> half_w, idx = val & mask;
        debug() << "Cluster = " << cluster << ", Idx = " << idx << std::endl;
	if (clusters.count(cluster))
	{
            debug() << "Recursing into cluster" << std::endl;
	    clusters[cluster].insert(idx); 
	}
	else
	{
            debug() << "Recursing into summary" << std::endl;
	    clusters[cluster] = VEBtree<T, half_w>{idx};
            if (summary)
	        summary->insert(cluster); // Recursive call
            else
                summary = std::make_unique<VEBtree<T, half_w>>(cluster);
	}
        decrease_depth();
    }

    T predecessor(T val)
    {
        increase_depth();
        debug() << "Predecessor " << val << std::endl;
	if (val <= min)
	    return (decrease_depth(), INFINITY);
	if (val > max)
	    return (decrease_depth(), max);
	const T cluster = val >> half_w, idx = val & mask;
        debug() << "Not a trivial case..." << std::endl;
	if (clusters.count(cluster) == 0 || clusters[cluster].getMin() >= val)
	{
            debug() << "First case" << std::endl;
            if (!summary)
                return min;
	    const T new_cluster = summary->predecessor(cluster);
            debug() << "new_cluster = " << new_cluster << std::endl;
            if (new_cluster == INFINITY)
            {
                decrease_depth();
                return (min < val) ? min : INFINITY;
            }
            decrease_depth();
	    return (new_cluster << half_w) | clusters[new_cluster].getMax();
	}
	else
	{      
            debug() << "Second case" << std::endl;
            decrease_depth();
	    return (cluster << half_w) | clusters[cluster].predecessor(idx);
	}
    }

    bool empty()
    { return min == INFINITY; }

    T getMin()
    { return min; }

    T getMax()
    { return max; }
};

// Base case implementation
template <typename T>
class VEBtree<T, 1>
{
    static constexpr T INFINITY = -1;
    bool hasZero = 0, hasOne = 0;

public:
    VEBtree() 
    {}

    VEBtree(T val): hasZero{!val}, hasOne{!!val}
    {}

    std::vector<T> traverse()
    { return (hasZero && hasOne) ? std::vector<T>{0, 1} : (hasZero ? std::vector<T>{0} : std::vector<T>{1}); }

    void insert(const T val) 
    { val ? (hasOne = 1) : (hasZero = 1); }

    T predecessor(T val) 
    { return val && hasOne; }

    bool empty()
    { return !hasZero && !hasOne; }

    T getMin() 
    { return hasZero ? 0 : (hasOne ? 1 : INFINITY); }

    T getMax()
    { return hasOne; }
};
