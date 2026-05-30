#include "ilya.h"
#include <random>

int main() {
  BigInteger x(-10);
  BigInteger y(2);

  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<> dist(-1'000'000, 1'000'000);

  for (int i = 0; i < 1000; ++i) {
    int x = dist(generator);
    int y = dist(generator);

    BigInteger bx(x);
    BigInteger by(y);

    std::cout << x << " + " << y << " = " << x + y << "\n";
    std::cout << bx << " + " << by << " = " << bx + by << "\n";
  }
}