
#include "veb.h"

#include <iostream>

template <typename T>
std::ostream & operator<<(std::ostream &os, std::vector<T> vals)
{
    os << "{";
    for (int i = 0, n = vals.size(); i < n; ++i)
    {  
        os << vals[i] << ((i < n - 1) ? ", " : "");
    }
    return os << "}";
}

VEBtree<unsigned, 4> tree;

void i(unsigned val)
{
    tree.insert(val);
    std::cout << tree.traverse() << std::endl;
}

void p(unsigned val)
{
    const unsigned p = tree.predecessor(val);
    std::cout << "pred of " << val << " is " << p << std::endl;
}

int main()
{
    i(0);
    i(3);
    i(7);
    i(9);
    i(13);
    i(4);

    p(1);
    p(5);
}
