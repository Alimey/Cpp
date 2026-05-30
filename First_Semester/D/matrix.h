#include <iostream>
#include <iomanip>
#include <vector>
#include <compare>
#include <cstring>
#include <algorithm>
#include <complex>
#include <time.h>
#include <assert.h>
#include <array>

#define CPP23


/// DECLARATIONS ///

// Class for the integer numbers of any size
class BigInteger;

BigInteger operator*(const BigInteger& a, const BigInteger& b);
BigInteger operator-(const BigInteger& a, const BigInteger& b);
BigInteger operator+(const BigInteger& a, const BigInteger& b);
BigInteger operator/(const BigInteger& a, const BigInteger& b);
BigInteger operator%(const BigInteger& a, const BigInteger& b);

bool operator==(const BigInteger& a, const BigInteger& b);

std::istream& operator>>(std::istream& in, const BigInteger& bi);
std::ostream& operator<<(std::ostream& out, const BigInteger& bi);

BigInteger operator""_bi(const char* str_bi, size_t);

// Get the greatest common divisor of two numbers
BigInteger gcd(const BigInteger& a, const BigInteger& b);

// Class for the rational numbers of any precision
class Rational;

Rational operator*(const Rational& a, const Rational& b);
Rational operator-(const Rational& a, const Rational& b);
Rational operator+(const Rational& a, const Rational& b);
Rational operator/(const Rational& a, const Rational& b);

/// Fft ///

// A set of functions - realizations of Fast Fourier transform
// for multiplying vectors of digits of biginteger numbers

namespace Fft {
// main recursive fft function
void fft(std::vector<std::complex<double>>& P, size_t start, size_t end, int forward) {
  size_t n = end - start + 1;

  if (n == 1) {
    return;
  }

  fft(P, start,  start + n / 2 - 1, forward);
  fft(P, start + n / 2, end, forward);

  std::complex<double> w = std::polar(1., 2 * acos(-1) / static_cast<double>(n) * forward);
  std::complex<double> w_n = 1;

  for (size_t i = start; i < start + n / 2; ++i) {
    auto remember = P[i];
    P[i] = P[i] + w_n * P[i + n / 2];
    P[i + n / 2] = remember - w_n * P[i + n / 2];
    w_n *= w;
  }
}

// add elements to the vector to reach the size of the pow of 2
void addTo2thPow(std::vector<std::complex<double>>& P) {
  while (P.size() & (P.size() - 1)) {
    P.push_back(0);
  }
}

// place elements on the correct positions in advance
void place(std::vector<std::complex<double>>& c) {
  size_t n = c.size();
  long long size_of_code = static_cast<long long>(std::log2(n));

  for (size_t i = 0; i < n; ++i) {
    size_t k = 0;
    long long copy_size_of_code = size_of_code - 1;
    size_t copy_i = i;

    while (copy_size_of_code >= 0) {
      if (copy_i % 2 != 0) {
        k += (1 << copy_size_of_code);
      }

      --copy_size_of_code;
      copy_i >>= 1;
    }

    if (i < k) {
      std::swap(c[i], c[k]);
    }
  }
}

// function for multiplying vectors using fast fourier transform
std::vector<long long> MultiplyVectors(const std::vector<long long>& first, const std::vector<long long>& second) {
  std::vector<std::complex<double>> cfirst(first.size() + second.size() - 1);
  std::vector<std::complex<double>> csecond(second.size() + first.size() - 1);

  for (size_t i = 0; i < first.size(); ++i) {
    cfirst[i] = std::complex<double>(static_cast<double>(first[i]));
  }

  for (size_t i = 0; i < second.size(); ++i) {
    csecond[i] = std::complex<double>(static_cast<double>(second[i]));
  }

  cfirst.reserve(2*cfirst.size());
  csecond.reserve(2*csecond.size());
  addTo2thPow(cfirst);
  addTo2thPow(csecond);
  place(cfirst);
  place(csecond);
  fft(cfirst, 0, cfirst.size() - 1, 1);
  fft(csecond, 0, cfirst.size() - 1, 1);

  std::vector<std::complex<double>> cfinal(cfirst.size());
  
  for (size_t i = 0; i < cfinal.size(); ++i) {
    cfinal[i] = cfirst[i] * csecond[i];
  }

  place(cfinal);
  fft(cfinal, 0, cfinal.size() - 1, -1);

  std::vector<long long> ans(first.size() + second.size() - 1);

  for (size_t i = 0; i < ans.size(); ++i) {
    ans[i] = static_cast<long long>(round(std::real(cfinal[i]) / static_cast<double>(cfinal.size())));
  }

  return ans;
}
};

/// BIG INTEGER ///


class BigInteger {
 private:
  std::vector<long long> digits_;
  const long long base_ = 1e3;
  bool is_negative_ = false;

  std::string findNumberPrefix(const std::string& str_bi) {
    std::string pref;

    if (str_bi.size() == 1 && (str_bi[0] == '+' || str_bi[0] == '-')) {
      pref += str_bi[0];
      return pref;
    }

    for (size_t i = 0; i < str_bi.size(); ++i) {
      if (i == 0 && (str_bi[i] == '+' || str_bi[i] == '-')) {
        continue;
      }

      if (str_bi[i] > '9' || str_bi[i] < '0') {
        pref = str_bi.substr(0, i);
        return pref;
      }
    }
    
    pref = str_bi;
    return pref;
  }

  void fixNumber() {
    for (size_t i = 0; i < digits_.size() - 1; ++i) {
      if (digits_[i] < 0) {
        --digits_[i + 1];
        digits_[i] += base_;
      }
    }

    for (size_t i = 0; i < digits_.size() - 1; ++i) {
      long long carry = digits_[i];
      digits_[i] %= base_;
      digits_[i + 1] += carry / base_;
    }

    if (digits_[digits_.size() - 1] >= base_) {
      long long carry = digits_[digits_.size() - 1];
      digits_[digits_.size() - 1] %= base_;
      digits_.push_back(carry / base_);
    }
  }

  void deleteZeros() {
    while (digits_.size() > 1 && digits_[digits_.size() - 1] == 0) {
      digits_.pop_back();
    }
  }

  void checkZero() {
    if (digits_.size() == 1 && digits_[0] == 0) {
      is_negative_ = false;
    }
  }

  void constructFromSizeT(size_t x) {
    is_negative_ = false;

    if (x == 0) {
      digits_.push_back(0);
      return;
    }

    long long long_x = static_cast<long long>(x);
    while (long_x > 0) {
      digits_.push_back(long_x % base_);
      long_x /= base_;
    }
  }

 public:
  BigInteger(): digits_({0}) {}

  BigInteger(long long x) {
    constructFromSizeT(static_cast<size_t>(std::abs(x)));
    is_negative_ = x < 0;
  }

  BigInteger(const BigInteger& other) = default;

  BigInteger(const std::string& input) {
    std::string s = findNumberPrefix(input);

    size_t start = 0;

    if (s.front() == '-') {
      is_negative_ = true;
      start = 1;
    } else if (s.front() == '+') {
      start = 1;
    }

    std::string digit;
    size_t digit_size = std::to_string(base_).size() - 1;

    for (size_t i = 0; i < s.size() - start; ++i) {
      if (digit.size() < digit_size) {
        digit += s[s.size() - 1 - i];
        continue;
      }

      std::reverse(digit.begin() ,digit.end());
      digits_.push_back(std::stoi(digit));
      digit = s[s.size() - 1 - i];
    }

    std::reverse(digit.begin(), digit.end());
    digits_.push_back(std::stoi(digit));
    deleteZeros();

    checkZero();
  }

  std::string toString() const {
    std::stringstream ss;
    const int kDigitLength = 3;

    if (is_negative_) ss << "-";

    ss << digits_[digits_.size() - 1];

    if (digits_.size() > 1) {
      for (size_t i = digits_.size() - 1; i > 0; --i) {
        ss << std::setw(kDigitLength) << std::setfill('0') << digits_[i - 1];
      }
    }

    return ss.str();
  }

  BigInteger operator-() const {
    BigInteger copy = *this;

    if (!isZero()) {
      copy.is_negative_ = !copy.is_negative_;
    }

    return copy;
  }

  BigInteger& operator--() {
    *this -= 1;
    return *this;
  }

  BigInteger& operator++() {
    *this += 1;
    return *this;
  }

  BigInteger operator--(int) {
    BigInteger ans = *this;
    --(*this);
    return ans;
  }

  BigInteger operator++(int) {
    BigInteger ans = *this;
    ++(*this);
    return ans;
  }

  explicit operator bool() const {
    return !isZero();
  }

  BigInteger& operator=(const BigInteger& other) {
    if (this == &other) {
      return *this;
    }

    assert(base_ == other.base_ && "Base of numbers differs");

    digits_ = other.digits_;
    is_negative_ = other.is_negative_;
    return *this;
  }

  BigInteger& operator+=(const BigInteger& other) {
    if (is_negative_ != other.is_negative_) {
      is_negative_ = !is_negative_;
      *this -= other;
      is_negative_ = !is_negative_;
      checkZero();
      return *this;
    }

    for (size_t i = 0; i < std::min(other.digits_.size(), digits_.size()); ++i) {
      digits_[i] += other.digits_[i];
    }

    for (size_t i = digits_.size(); i < other.digits_.size(); ++i) {
      digits_.push_back(other.digits_[i]);
    }

    fixNumber();
    deleteZeros();
    checkZero();
    return *this;
  }

  BigInteger& operator-=(const BigInteger& other) {
    if (is_negative_ != other.is_negative_) {
      is_negative_ = !is_negative_;
      *this += other;
      is_negative_ = !is_negative_;
      checkZero();
      return *this;
    }

    for (size_t i = 0; i < std::min(other.digits_.size(), digits_.size()); ++i) {
      digits_[i] -= other.digits_[i];
    }

    for (size_t i = digits_.size(); i < other.digits_.size(); ++i) {
      digits_.push_back(-other.digits_[i]);
    }

    deleteZeros();

    if (digits_.back() < 0) {
      for (size_t i = 0; i < digits_.size(); ++i) {
        digits_[i] *= -1;
      }

      is_negative_ = !is_negative_;
    }

    fixNumber();
    deleteZeros();
    checkZero();
    return *this;
  }

  BigInteger& operator*=(const BigInteger& other) {
    if (isZero()) {
      return *this;
    }

    if (other.isZero()) {
      *this = 0;
      return *this;
    }

    is_negative_ ^= other.is_negative_;

    if (other.digits_.size() == 1) {
      for (size_t i = 0; i < digits_.size(); ++i) {
        digits_[i] *= other.digits_[0];
      }

      fixNumber();
      return *this;
    }

    digits_ = Fft::MultiplyVectors(digits_, other.digits_);
    fixNumber();
    deleteZeros();
    return *this;
  }
  

  BigInteger& operator/=(const BigInteger& other) {
    assert(!other.isZero() && "Division by zero");

    if (isZero()) {
      return *this;
    }

    BigInteger to_divide;
    BigInteger ans;
    BigInteger divisor = other;

    to_divide.is_negative_ = false;
    ans.is_negative_ = false;
    divisor.is_negative_ = false;

    bool remember = is_negative_;
    is_negative_ = false;

    if (*this < divisor) {
      *this = 0;
      return *this;
    }

    is_negative_ = remember;
    ans.digits_.resize(digits_.size());
    to_divide.digits_.reserve(2 * digits_.size());

    for (size_t i = 0; i < digits_.size(); ++i) {
      to_divide.digits_[0] = digits_[digits_.size() - i - 1];
      long long digit = 0;
      long long l = 0; long long r = base_;
      bool is_cicle_needed = to_divide >= divisor;

      while (is_cicle_needed && l + 1 < r) {
        long long m = (l + r) / 2;

        if (divisor * m <= to_divide) {
          l = m ;
        } else {
          r = m;
        }
      }

      digit = l;
      ans.digits_[ans.digits_.size() - 1 - i] = digit;

      if (digit > 0) {
        to_divide -= divisor * digit;
      }

      if (to_divide.isZero()) {
        continue;
      }

      to_divide.digits_.insert(to_divide.digits_.begin(), 0);
    }

    ans.deleteZeros();
    ans.fixNumber();

    if (!ans.isZero()) {
      is_negative_ ^= other.is_negative_;
    }

    digits_ = ans.digits_;
    return *this;
  }

  BigInteger& operator%=(const BigInteger& other) {
    *this -= (*this / other) * other;
    return *this;
  }

  std::strong_ordering operator<=>(const BigInteger& other) const {
    if (!is_negative_ && other.is_negative_) {
      return std::strong_ordering::greater;
    }
    if (is_negative_ && !other.is_negative_) {
      return std::strong_ordering::less;
    }
    if (digits_.size() < other.digits_.size()) {
      return is_negative_ ? std::strong_ordering::greater : std::strong_ordering::less;
    }
    if (digits_.size() > other.digits_.size()) {
      return is_negative_ ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    for (size_t i = 0; i < digits_.size(); ++i) {
      if (digits_[digits_.size() - i - 1] < other.digits_[digits_.size() - i - 1]) {
        return is_negative_ ? std::strong_ordering::greater : std::strong_ordering::less;
      }
      if (digits_[digits_.size() - i - 1] > other.digits_[digits_.size() - i - 1]) {
        return is_negative_ ? std::strong_ordering::less : std::strong_ordering::greater;
      }
    }
    return std::strong_ordering::equivalent;
  }

  bool isZero() const {
    return digits_.size() == 1 && digits_[0] == 0;
  }

  friend class Rational;
  friend std::istream& operator>>(std::istream& in, BigInteger& bi);
  friend bool operator==(const BigInteger& a, const BigInteger& b);
  friend BigInteger operator""_bi(unsigned long long x);

  ~BigInteger() = default;
};

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
  BigInteger c = a;
  c += b;
  return c;
}

BigInteger operator-(const BigInteger& a, const BigInteger& b) {
  BigInteger c = a;
  c -= b;
  return c;
}

BigInteger operator*(const BigInteger& a, const BigInteger& b) {
  BigInteger ans = a;
  ans *= b;
  return ans;
}

BigInteger operator/(const BigInteger& a, const BigInteger& b) {
  BigInteger ans = a;
  ans /= b;
  return ans;
}

BigInteger operator%(const BigInteger& a, const BigInteger& b) {
  BigInteger ans = a;
  ans %= b;
  return ans;
}

bool operator==(const BigInteger& a, const BigInteger& b) {
  if (a.digits_.size() != b.digits_.size() || a.is_negative_ != b.is_negative_) {
    return false;
  }

  for (size_t i = 0; i < a.digits_.size(); ++i) {
    if (a.digits_[i] != b.digits_[i]) {
      return false;
    }
  }

  return true;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& bi) {
  out << bi.toString();
  return out;
}

std::istream& operator>>(std::istream& in, BigInteger& bi) {
  std::string str_bi;
  in >> str_bi;
  bi = BigInteger(str_bi);
  return in;
}

BigInteger operator""_bi(const char* str_bi, size_t) {
    BigInteger bi;
    std::stringstream stream(str_bi);
    stream >> bi;
    return bi;
  }

BigInteger operator""_bi(unsigned long long x) {
  BigInteger big_x;
  big_x.digits_.pop_back();
  big_x.constructFromSizeT(x);
  return big_x;
}

BigInteger gcd(const BigInteger& a, const BigInteger& b) {
  return b == 0 ? a : gcd(b, a % b);
}

/// RATIONAL ///

class Rational {
 private:
  BigInteger numerator_;
  BigInteger denominator_;

  void reduce() {
    bool z_negative = numerator_.is_negative_;
    bool n_negative = denominator_.is_negative_;
    numerator_.is_negative_ = false;
    denominator_.is_negative_ = false;
    BigInteger divisor = gcd(numerator_, denominator_);
    numerator_.is_negative_ = z_negative;
    denominator_.is_negative_ = n_negative;
    numerator_ /= divisor;
    denominator_ /= divisor;
  }

 public:
  Rational(): numerator_(0), denominator_(1) {}

  Rational(const BigInteger& bi): numerator_(bi), denominator_(1) {}

  Rational(long long x): numerator_(x), denominator_(1) {}

  Rational(const Rational& other): numerator_(other.numerator_), denominator_(other.denominator_) {}

  Rational& operator=(const Rational& other) = default;

  Rational& operator+=(const Rational& other) {
    numerator_ = numerator_ * other.denominator_ + other.numerator_ * denominator_;
    denominator_ *= other.denominator_;
    reduce();
    return *this;
  }

  Rational& operator-=(const Rational& other) {
    numerator_ = numerator_ * other.denominator_ - other.numerator_ * denominator_;
    denominator_ *= other.denominator_;
    reduce();
    return *this;
  }

  Rational& operator*=(const Rational& other) {
    numerator_ *= other.numerator_;
    denominator_ *= other.denominator_;
    reduce();
    return *this;
  }

  Rational& operator/=(const Rational& other) {
    assert(!other.numerator_.isZero() && "Division by zero");

    if (numerator_.isZero()) {
      return *this;
    }

    numerator_ *= other.denominator_;
    denominator_ *= other.numerator_;
    denominator_.is_negative_ = false;
    numerator_.is_negative_ = numerator_.is_negative_ != other.numerator_.is_negative_;
    reduce();
    return *this;
  }

  Rational operator-() const {
    Rational copy = *this;

    if (!numerator_.isZero()) {
      copy.numerator_.is_negative_ = !copy.numerator_.is_negative_;
    }

    return copy;
  }

  std::strong_ordering operator<=>(const Rational& other) const {
    return numerator_ * other.denominator_ <=> denominator_ * other.numerator_;
  }

  std::string toString() const {
    std::string string_view = numerator_.toString();

    if (denominator_ == 1) {
      return string_view;
    }

    string_view += "/" + denominator_.toString();
    return string_view;
  }

  std::string asDecimal(size_t precision = 0) {
    std::string fraction;

    // Add sign if needed
    if (numerator_.is_negative_) {
      fraction += '-';
    }

    auto carry = numerator_;
    carry.is_negative_ = false;

    // Add the main part of the number
    auto first_part = (carry / denominator_).toString();
    fraction += first_part;

    if (precision == 0) {
      return fraction;
    }

    fraction += '.';

    std::string zeros; // zeros right behind the point
    carry %= denominator_; // remainder to divide on denumerator

    if (carry == 0) {
      zeros.resize(precision);
      std::fill(zeros.begin(), zeros.end(), '0');
      fraction += zeros;
      return fraction;
    }

    // Count zeros right behind the point
    while (carry < denominator_) {
      carry *= 10;
      zeros += '0';
    }

    zeros.pop_back();

    // If zeros are enough for the given precision
    if (zeros.size() >= precision) {
      fraction += zeros.substr(0, precision);
      return fraction;
    }

    precision -= zeros.size();
    fraction += zeros;

    // Counting the number to divide on denumerator and get the rest of digits
    for (size_t i = 0; i < precision + 1; ++i) {
      carry *= numerator_.base_;
    }

    // Counting the rest digits
    auto second_part = (carry / denominator_).toString();
    fraction += second_part.substr(0, precision + 1);

    // Checking if rounding is needed
    if (fraction.back() < '5') {
      fraction.pop_back();
      return fraction;
    }

    fraction.pop_back();
    size_t i = 0;
    size_t fsize = fraction.size();

    // Rounding numbers behind point
    while (fraction[fsize - i - 1] == '9' && fraction[fsize - i - 1] != '.') {
      fraction[fsize - i - 1] = '0';
      ++i;
    }

    if (fraction[fsize - i - 1] != '.') {
      ++fraction[fsize - i - 1];
      return fraction;
    }

    ++i;
    
    // Rounding numbers before point
    while (fsize >= i + 1 && fraction[fsize - i - 1] == '9') {
      fraction[fsize - i - 1] = '0';
      ++i;
    }

    if (fsize >= i + 1) {
      ++fraction[fsize - i - 1];
      return fraction;
    }

    // Add 1 in the begining if needed
    fraction = '1' + fraction;
    return fraction;
  }

  explicit operator double() {
    const int kPrecision = 40;
    double d = std::stod(asDecimal(kPrecision));
    return d;
  }

  friend bool operator==(const Rational& a, const Rational& b) = default;

  ~Rational() = default;
};

Rational operator+(const Rational& a, const Rational& b) {
  Rational c = a;
  c += b;
  return c;
}

Rational operator-(const Rational& a, const Rational& b) {
  Rational c = a;
  c -= b;
  return c;
}

Rational operator*(const Rational& a, const Rational& b) {
  Rational ans = a;
  ans *= b;
  return ans;
}

Rational operator/(const Rational& a, const Rational& b) {
  Rational ans = a;
  ans /= b;
  return ans;
}

std::istream& operator>>(std::istream& in, Rational& rat) {
  BigInteger b;
  in >> b;
  rat = Rational(b);
  return in;
}

std::ostream& operator<<(std::ostream& out, const Rational& rat) {
  out << rat.toString();
  return out;
}

/// METAFUNCTIONS ///

// IS_PRIME //

// Metafunctions check if N is a prime number

// constexpr function for counting the sqrt of a number n
constexpr size_t sqrt_n(size_t n, size_t curr = 1) {
  while (curr * curr < n) {
    ++curr;
  }
  if (curr * curr > n) {
    --curr;
  }
  return curr;
}

template <size_t n, size_t div>
struct is_prime_recursion {
  static constexpr bool value = (n % div != 0) && is_prime_recursion<n, div - 1>::value;
};

template <size_t n>
struct is_prime_recursion<n, 1> {
  static constexpr bool value = true;
};

// Wrapper-function
template <size_t n>
struct is_prime {
  static constexpr bool value = is_prime_recursion<n, sqrt_n(n)>::value;
};

/// EXPRESSION TEMPLATES ///

// Expression templates for counting the sum, subtraction,
// product of matrices and the product of matrices by a number

template <typename Derived>
class MatrixExpression {
 public:
  const Derived& exprValue() const {
    return static_cast<const Derived&>(*this);
  }

  bool operator==(const MatrixExpression& other) const = default;
};

template <typename E1, typename E2>
class MatrixSum: public MatrixExpression<MatrixSum<E1, E2>> {
 public:
  using value_type = typename E1::value_type; // value_type of values in base matrices

  static constexpr bool is_leaf = false;
  std::conditional_t<E1::is_leaf, E1, const E1&> e1;
  std::conditional_t<E2::is_leaf, E2, const E2&> e2;
  static constexpr size_t kCols = E1::kCols; // the number of columns in the sum matrix

  MatrixSum(const E1& e1, const E2& e2)
      : e1(e1), e2(e2) {}

  decltype(auto) operator[](size_t row, size_t col) const {
    return e1.exprValue()[row, col] + e2.exprValue()[row, col];
  }
};

template <typename E1, typename E2>
class MatrixSubtraction: public MatrixExpression<MatrixSubtraction<E1, E2>> {
 public:
  using value_type = typename E1::value_type; // value_type of values in base matrices

  static constexpr bool is_leaf = false;
  std::conditional_t<E1::is_leaf, E1, const E1&> e1;
  std::conditional_t<E2::is_leaf, E2, const E2&> e2;
  static constexpr size_t kCols = E1::kCols; // the number of columns in the subtraction matrix

  MatrixSubtraction(const E1& e1, const E2& e2)
      : e1(e1), e2(e2) {}

  decltype(auto) operator[](size_t row, size_t col) const {
    return e1.exprValue()[row, col] - e2.exprValue()[row, col];
  }
};

template <typename E1, typename E2>
class MatrixProduct: public MatrixExpression<MatrixProduct<E1, E2>> {
 public:
  using value_type = typename E1::value_type; // value_type of values in base matrices

  static constexpr bool is_leaf = false;
  std::conditional_t<E1::is_leaf, E1, const E1&> e1;
  std::conditional_t<E2::is_leaf, E2, const E2&> e2;
  static constexpr size_t kCols = E2::kCols; // the number of columns in the product matrix

  MatrixProduct(const E1& e1, const E2& e2)
      : e1(e1), e2(e2) {}

  decltype(auto) operator[](size_t row, size_t col) const {
    decltype(e1.exprValue()[row, 0] * e2.exprValue()[0, col]) ans = 0;

    for (size_t i = 0; i < e1.exprValue().kCols; ++i) {
      ans += e1.exprValue()[row, i] * e2.exprValue()[i, col];
    }

    return ans;
  }
};

template <typename E, typename Scalar>
class MatrixNumberMultiplication: public MatrixExpression<MatrixNumberMultiplication<E, Scalar>> {
 public:
  using value_type = Scalar; // value_type of values in base matrices

  static constexpr bool is_leaf = false;
  std::conditional_t<E::is_leaf, E, const E&> e;
  Scalar scalar;
  static constexpr size_t kCols = E::kCols; // the number of columns in the product matrix bu a scalar

  MatrixNumberMultiplication(const E& e, Scalar scalar)
      : e(e), scalar(scalar) {}

  decltype(auto) operator[](size_t row, size_t col) const {
    return e.exprValue()[row, col] * scalar;
  }
};

/// DECLARATIONS ///

// RESIDUE //

// The class of residuals modulo Mod
template <size_t Mod>
class Residue {
 public:
  size_t rem = 0;

  Residue() = default;

  Residue(long long x);

  explicit operator int() const;

  // Get the pow of residue in log time
  Residue binpow(size_t pow) const;

  Residue inversed() const;

  Residue& operator+=(const Residue& other);
  Residue& operator-=(const Residue& other);
  Residue& operator*=(const Residue& other);
  Residue& operator/=(const Residue& other);

  std::strong_ordering operator<=>(const Residue& other) const = default;
};

template <size_t Mod>
Residue<Mod> operator+(const Residue<Mod>& first, const Residue<Mod>& second);

template <size_t Mod>
Residue<Mod> operator-(const Residue<Mod>& first, const Residue<Mod>& second);

template <size_t Mod>
Residue<Mod> operator*(const Residue<Mod>& first, const Residue<Mod>& second);

template <size_t Mod>
Residue<Mod> operator/(const Residue<Mod>& first, const Residue<Mod>& second);

// MATRIX //

// The class of the matrices Row x Col, that contain elements of value_type Field.
template <size_t Row, size_t Col, typename Field=Rational>
class Matrix: public MatrixExpression<Matrix<Row, Col, Field>> {
  using value_type = Field;

  friend class MatrixExpression<Matrix<Row, Col, Field>>;

  template <typename E1, typename E2>
  friend class MatrixSum;

  template <typename E1, typename E2>
  friend class MatrixSubtraction;

  template <typename E1, typename E2>
  friend class MatrixProduct;

  template <typename E, typename Scalar>
  friend class MatrixNumberMultiplication;

  template <size_t AnotherN, size_t Another_M, typename Another_Field>
  friend class Matrix;

 private:
  static constexpr bool is_leaf = true;

  std::array<std::array<Field, Col>, Row> matrix;
  static constexpr size_t kCols = Col; // the number of columns in matrix

 public:
  Matrix() = default;
  Matrix(const Matrix& other) = default;
  Matrix(std::initializer_list<std::initializer_list<Field>> input);
 
  // Matrix constructor from matrix expression
  template <typename Derived>
  Matrix(const MatrixExpression<Derived>& expr);

  Matrix& operator=(const Matrix& other) = default;

  Matrix& operator+=(const Matrix& other);

  Matrix& operator-=(const Matrix& other);

  Matrix& operator*=(const Field& number);

  Matrix& operator*=(const Matrix<Col, Col, Field>& other);
 
 private:
  // Make matrix triangular and get the determinant
  Field triangulate();

  // Make matrix simplified and get the determinant
  Field simplify();
 
 public:
  Matrix<Col, Row, Field> transposed() const;

  size_t rank() const;

  std::array<Field, Col> getRow(size_t row) const;

  std::array<Field, Row> getColumn(size_t col) const;

  Field& operator[](size_t row, size_t col);

  const Field& operator[](size_t row, size_t col) const;

  bool operator==(const Matrix& other) const = default;

  // Get the unity matrix of the same size
  static Matrix unityMatrix();

  Field det() const;

  Matrix inverted() const;

  Matrix& invert();

  Field trace() const;
};

template <typename E1, typename E2>
MatrixSum<E1, E2> operator+(const MatrixExpression<E1>& first, const MatrixExpression<E2>& second);

template <typename E1, typename E2>
MatrixSubtraction<E1, E2> operator-(const MatrixExpression<E1>& first, const MatrixExpression<E2>& second);

template <typename E1, typename E2>
MatrixProduct<E1, E2> operator+(const MatrixExpression<E1>& first, const MatrixExpression<E2>& second);

/// DEFINITIONS ///

// RESIDUE //

template <size_t Mod>
Residue<Mod> Residue<Mod>::binpow(size_t pow) const {
  Residue<Mod> res = 1;
  Residue<Mod> rem_pow = *this;

  while (pow > 0) {
    if ((pow & 1) == 1) {
      res *= rem_pow;
    }

    pow >>= 1;
    rem_pow *= rem_pow;
  }

  return res;
}

template <size_t Mod>
Residue<Mod> Residue<Mod>::inversed() const {
  static_assert(is_prime<Mod>::value);
  Residue<Mod> res = binpow(Mod - 2);

  return res;
}

template <size_t Mod>
Residue<Mod>::Residue(long long x) {
  if (x < 0) {
    long long long_n = static_cast<long long>(Mod);
    x += (abs(x) / long_n + 1) * long_n;
  }

  rem = static_cast<size_t>(x) % Mod;
}

template <size_t Mod>
Residue<Mod>::operator int() const {
  return rem;
}

template <size_t Mod>
Residue<Mod>& Residue<Mod>::operator+=(const Residue<Mod>& other) {
  *this = Residue<Mod>(rem + other.rem);
  return *this;
}

template <size_t Mod>
Residue<Mod>& Residue<Mod>::operator-=(const Residue<Mod>& other) {
  if (rem < other.rem) {
    rem += Mod;
  }
  
  rem -= other.rem;
  return *this;
}

template <size_t Mod>
Residue<Mod>& Residue<Mod>::operator*=(const Residue<Mod>& other) {
  *this = Residue<Mod>(rem * other.rem);
  return *this;
}

template <size_t Mod>
Residue<Mod>& Residue<Mod>::operator/=(const Residue<Mod>& other) {
  operator*=(other.inversed());
  return *this;
}

template <size_t Mod>
Residue<Mod> operator+(const Residue<Mod>& first, const Residue<Mod>& second) {
  Residue<Mod> ans = first;
  ans += second;
  return ans;
}

template <size_t Mod>
Residue<Mod> operator-(const Residue<Mod>& first, const Residue<Mod>& second) {
  Residue<Mod> ans = first;
  ans -= second;
  return ans;
}

template <size_t Mod>
Residue<Mod> operator*(const Residue<Mod>& first, const Residue<Mod>& second) {
  Residue<Mod> ans = first;
  ans *= second;
  return ans;
}

template <size_t Mod>
Residue<Mod> operator/(const Residue<Mod>& first, const Residue<Mod>& second) {
  Residue<Mod> ans = first;
  ans /= second;
  return ans;
}

template <size_t Mod>
std::ostream& operator<<(std::ostream& out, const Residue<Mod>& res) {
  out << res.rem;
  return out;
}

template <size_t Mod>
std::istream& operator>>(std::istream& in, Residue<Mod>& res) {
  long long value;
  in >> value;
  res = Residue<Mod>(value);
  return in;
}

// MATRIX->USUAL //

// Definitions for the common functions of both non-square and square matrices

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field>::Matrix(std::initializer_list<std::initializer_list<Field>> input) {
  size_t row = 0;

  for (auto list : input) {
    std::copy(list.begin(), list.end(), matrix[row].begin());
    ++row;
  }
}

template <size_t Row, size_t Col, typename Field>
template <typename Derived>
Matrix<Row, Col, Field>::Matrix(const MatrixExpression<Derived>& expr) {
  const auto& generator = expr.exprValue();

  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Col; ++j) {
      matrix[i][j] = generator[i, j];
    }
  }
}

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field>& Matrix<Row, Col, Field>::operator+=(const Matrix<Row, Col, Field>& other) {
  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Col; ++j) {
      matrix[i][j] += other.matrix[i][j];
    }
  }

  return *this;
}

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field>& Matrix<Row, Col, Field>::operator-=(const Matrix<Row, Col, Field>& other) {
  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Col; ++j) {
      matrix[i][j] -= other.matrix[i][j];
    }
  }

  return *this;
}

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field>& Matrix<Row, Col, Field>::operator*=(const Field& number) {
  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Col; ++j) {
      matrix[i][j] *= number;
    }
  }

  return *this;
}

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field>& Matrix<Row, Col, Field>::operator*=(const Matrix<Col, Col, Field>& other) {
  *this = *this * other;
  return *this;
}

template <size_t Row, size_t Col, typename Field>
Matrix<Col, Row, Field> Matrix<Row, Col, Field>::transposed() const {
  Matrix<Col, Row, Field> ans;
  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Col; ++j) {
      ans.matrix[j][i] = matrix[i][j];
    }
  }

  return ans;
}

template <size_t Row, size_t Col, typename Field=Rational>
std::ostream& operator<<(std::ostream& out, const Matrix<Row, Col, Field>& matrix) {
  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Col; ++j) {
      out << matrix[i, j] << " ";
    }
    out << "\n";
  }

  return out;
}

template <size_t Row, size_t Col, typename Field>
Field Matrix<Row, Col, Field>::triangulate() {
  Field determinant = 1;
  int sign = 1;

  for (size_t i = 0; i < Row; ++i) {
    size_t not_zero = i;

    for (size_t k = i + 1; k < Row; ++k) {
      if (matrix[k][i] != Field(0)) {
        not_zero = k;
        break;
      }
    }

    if (not_zero != i) {
      std::swap(matrix[i], matrix[not_zero]);
      sign *= -1;
    }

    Field divisor = matrix[i][i];

    if (divisor == 0) {
      determinant = 0;
      return determinant;
    }

    determinant *= divisor;

    for (size_t j = i; j < Col; ++j) {
      matrix[i][j] /= divisor;
    }

    for (size_t k = i + 1; k < Row; ++k) {
      Field multiplier = matrix[k][i];

      for (size_t j = i; j < Col; ++j) {
        matrix[k][j] -= multiplier * matrix[i][j];
      }
    }
  }

  determinant *= sign;
  return determinant;
}

template <size_t Row, size_t Col, typename Field>
Field Matrix<Row, Col, Field>::simplify() {
  Field determinant = triangulate();
  
  for (size_t i = Row - 1; i != SIZE_MAX; --i) {
    for (size_t k = i - 1; k != SIZE_MAX; --k) {
      Field multiplier = matrix[k][i];

      for (size_t j = i; j < Col; ++j) {
        matrix[k][j] -= multiplier * matrix[i][j];
      }
    }
  }

  return determinant;
}

template <size_t Row,  size_t Col, typename Field>
size_t Matrix<Row, Col, Field>::rank() const {
  Matrix<Row, Col, Field> simplified = *this;
  simplified.triangulate();
  size_t rank_value = 0;

  for (size_t i = 0; i < Row; ++i) {
    bool is_essential = false;

    for (size_t j = 0; j < simplified.matrix[i].size(); ++j) {
      if (simplified.matrix[i][j] != 0) {
        is_essential = true;
        break;
      }
    }

    if (!is_essential) {
      return rank_value;
    }

    ++rank_value;
  }

  return rank_value;
}

template <size_t Row, size_t Col, typename Field>
std::array<Field, Col> Matrix<Row, Col, Field>::getRow(size_t row) const {
  std::array<Field, Col> arr_row = matrix[row];
  return arr_row;
}

template <size_t Row, size_t Col, typename Field>
std::array<Field, Row> Matrix<Row, Col, Field>::getColumn(size_t col) const {
  std::array<Field, Row> arr_col;

  for (size_t i = 0; i < Row; ++i) {
    arr_col[i] = matrix[i][col];
  }

  return arr_col;
}

template <size_t Row, size_t Col, typename Field>
Field& Matrix<Row, Col, Field>::operator[](size_t row, size_t col) {
  return matrix[row][col];
}

template <size_t Row, size_t Col, typename Field>
const Field& Matrix<Row, Col, Field>::operator[](size_t row, size_t col) const {
  return matrix[row][col];
}

template <typename E1, typename E2>
MatrixSum<E1, E2> operator+(const MatrixExpression<E1>& first, const MatrixExpression<E2>& second) {
  MatrixSum<E1, E2> ans(first.exprValue(), second.exprValue());
  return ans;
}

template <typename E1, typename E2>
MatrixSubtraction<E1, E2> operator-(const MatrixExpression<E1>& first, const MatrixExpression<E2>& second) {
  MatrixSubtraction<E1, E2> ans(first.exprValue(), second.exprValue());
  return ans;
}

template <typename E1, typename E2>
MatrixProduct<E1, E2> operator*(const MatrixExpression<E1>& first, const MatrixExpression<E2>& second) {
  MatrixProduct<E1, E2> ans(first.exprValue(), second.exprValue());
  return ans;
}

template <typename E>
MatrixNumberMultiplication<E, typename E::value_type> operator*(const MatrixExpression<E>& first, const typename E::value_type& second) {
  MatrixNumberMultiplication<E, typename E::value_type> ans(first.exprValue(), second);
  return ans;
}

template <typename E>
MatrixNumberMultiplication<E, typename E::value_type> operator*(const typename E::value_type& first, const MatrixExpression<E>& second) {
  MatrixNumberMultiplication<E, typename E::value_type> ans(second.exprValue(), first);
  return ans;
}

// MATRIX->SQUARE //

// A set of functions that can be only used with square matrices

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field> Matrix<Row, Col, Field>::unityMatrix() {
  static_assert(Row == Col);

  Matrix<Row, Col, Field> ans;

  for (size_t i = 0; i < Row; ++i) {
    ans[i, i] = Field(1);
  }

  return ans;
}

template <size_t Row, size_t Col, typename Field>
Field Matrix<Row, Col, Field>::det() const {
  static_assert(Row == Col);

  auto copy = *this;
  Field ans = copy.triangulate();

  return ans;
}

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field>& Matrix<Row, Col, Field>::invert() {
  static_assert(Row == Col);

  Matrix<Row, 2 * Row, Field> doubled;

  for (size_t i = 0; i < Row; ++i) {
    doubled.matrix[i][Row + i] = 1;

    for (size_t j = 0; j < Row; ++j) {
      doubled.matrix[i][j] = matrix[i][j];
    }
  }

  Field determinant = doubled.simplify();

  if (determinant == 0) {
    throw std::runtime_error("Matrix is not invertible");
  }

  for (size_t i = 0; i < Row; ++i) {
    for (size_t j = 0; j < Row; ++j) {
      matrix[i][j] = doubled.matrix[i][Row + j];
    }
  }

  return *this;
}

template <size_t Row, size_t Col, typename Field>
Matrix<Row, Col, Field> Matrix<Row, Col, Field>::inverted() const {
  auto copy = *this;
  copy.invert();
  return copy;
}

template <size_t Row, size_t Col, typename Field>
Field Matrix<Row, Col, Field>::trace() const {
  static_assert(Row == Col);

  Field ans = 0;

  for (size_t i = 0; i < Row; ++i) {
    ans += matrix[i][i];
  }

  return ans;
}

template <size_t Row, typename Field=Rational>
using SquareMatrix = Matrix<Row, Row, Field>;