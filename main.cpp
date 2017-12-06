
#include "string.h"
#include <iostream>

int main() {
    util::string a = "A", b = "B";
    util::string tmp;
    std::cout << "a = " << (void*)&a << std::endl;
    std::cout << "b = " << (void*)&b << std::endl;
    std::cout << "tmp = " << (void*)&tmp << std::endl;
    tmp = a;
    a = b;
    b = tmp;
    std::cout << a << " " << b << std::endl;
    if (a == b) {
        std::cout << b << " == " << a << std::endl;
    } else {
        std::cout << b << " != " << a << std::endl;
    }
}
