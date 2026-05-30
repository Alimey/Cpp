#include <iostream>
#include <random>
#include "C:\Users\Alimey\Desktop\programming\C++\Pro\B\ilya.h"

long long int RandomInt(long long int min, long long int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

void RandomIntFill(std::vector<long long int>& to_fill, long long int max, long long int min, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    to_fill.push_back(RandomInt(min, max));
  }
}

std::vector<long long int> MakeIntOperation(std::vector<long long int>& a, std::vector<long long int>& b, char operation) {
  std::vector<long long int> answers;
  if (operation == '+') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] + b[i]);
    }
  }
  if (operation == '-') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] - b[i]);
    }
  }
  if (operation == '*') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] * b[i]);
    }
  }
  if (operation == '/') {
    for (size_t i = 0; i < a.size(); ++i) {
      if (b[i] == 0) {
        answers.push_back(a[i]);
        continue;
      }
      answers.push_back(a[i] / b[i]);
    }
  }
  return answers;
}

void RandomBigintFill(std::vector<BigInteger>& to_fill, std::vector<long long int>& base) {
  for (size_t i = 0; i < base.size(); ++i) {
    to_fill.push_back(BigInteger(base[i]));
  }
}

std::vector<BigInteger> MakeBigintOperation(std::vector<BigInteger>& a, std::vector<BigInteger>& b, char operation) {
  std::vector<BigInteger> answers;
  if (operation == '+') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] + b[i]);
    }
  }
  if (operation == '-') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] - b[i]);
    }
  }
  if (operation == '*') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] * b[i]);
    }
  }
  if (operation == '/') {
    for (size_t i = 0; i < a.size(); ++i) {
      answers.push_back(a[i] / b[i]);
    }
  }
  return answers;
}

int main() {
  size_t quantity = 10000;
  long long int maximum = 100000;
  long long int minimun = -100000;
  char operation = '-';
  /// long long int ///
  std::vector<long long int> int_a;
  std::vector<long long int> int_b;
  RandomIntFill(int_a, maximum, minimun, quantity);
  RandomIntFill(int_b, maximum, minimun, quantity);
  std::vector<long long int> int_ans = MakeIntOperation(int_a, int_b, operation);
  // std::vector<double> int_ans;
  // for (size_t i = 0; i < int_a.size(); ++i) {
  //   int_ans.push_back(double(int_a[i]) / double(int_b[i]));
  // }
  /// BIGlong long int ///
  std::vector<BigInteger> bint_a;
  std::vector<BigInteger> bint_b;
  RandomBigintFill(bint_a, int_a);
  RandomBigintFill(bint_b, int_b);
  std::vector<BigInteger> bint_ans = MakeBigintOperation(bint_a, bint_b, operation);
  // std::vector<Rational> ratio_ans;
  // for (size_t i = 0; i < bint_a.size(); ++i) {
  //   Rational r;
  //   r.z = bint_a[i];
  //   r.n = bint_b[i];
  //   if (r.n.is_negative && !r.z.is_negative) {
  //     r.n.is_negative = false;
  //     r.z.is_negative = true;
  //   }
  //   if (r.z.is_negative && r.n.is_negative) {
  //     r.n.is_negative = false;
  //     r.z.is_negative = false;
  //   }
  //   ratio_ans.push_back(r);
  // }
  /// Comparing ///
  // std::cout << "INT\n";
  // for (size_t i = 0; i < quantity; ++i) {
  //   std::cout << int_a[i] << " + " << int_b[i] << " = " << int_ans[i] << "\n";
  // }
  // std::cout << "BIGINT\n";
  // for (size_t i = 0; i < quantity; ++i) {
  //   std::cout << bint_a[i] << " + " << bint_b[i] << " = " << bint_ans[i] << "\n";
  // }
  /// Asserting ///
  for (size_t i = 0; i < quantity; ++i) {
    if (int_ans[i] != bint_ans[i]) {
      std::cout << "Stress failed on example: " << int_a[i] << " " << operation << " " << int_b[i] << " = " << int_ans[i] << "_long long int / " << bint_ans[i] << "_bint\n";
      // std::cout << "Test: " << int_a[i] << "/" << int_b[i] << "\n";
      // std::cout << "rational: " << double(ratio_ans[i]) << ", double: " << int_ans[i] << "\n";
    }
  }
}