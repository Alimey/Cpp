#include <iostream>
#include <random>
#include <time.h>

std::string generateRandomString(size_t length, const std::string& charset) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }

    return result;
}

int main() {
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<> distribution_upper(20'000, 30'000);
  std::uniform_int_distribution<> distribution_lower(1, 20'000);
  for (size_t i = 0; i < 1000; ++i) {
    time_t start, end;
    time_t loc_start, loc_end;
    time(&start);
    std::string charset = "123456789";
    std::string s = generateRandomString(30000, charset);
    std::string ss = generateRandomString(15000, charset);

    BigInteger a = s;
    BigInteger b = ss;
    time(&loc_start);
    BigInteger c = a / b;
    time(&loc_end);
    std::cout << difftime(loc_end, loc_start) << " ";
    time(&loc_start);
    BigInteger cc = a * b;
    time(&loc_end);
    std::cout << difftime(loc_end, loc_start) << " ";
    time(&end);
    double sec = difftime(end, start);
    std::cout << sec << "\n";
    std::cout << a.digits.size() << " " << b.digits.size() << "\n";
  }
}