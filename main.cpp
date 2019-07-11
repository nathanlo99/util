#include "string.h"
#include <iostream>

int main() {
  util::string a = util::string{""} + "", b = util::string{""}, tmp;
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
