#include <iostream>
#include "biginteger.h"

int main() {
  Rational r;
  r.z = 1;
  r.n = 99999999;
  std::cout << r.asDecimal(5);
}