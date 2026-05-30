#include <iostream>
#include <iomanip>
#include <vector>
#include <compare>
#include <cstring>
#include <algorithm>
#include <complex>
#include <time.h>
#include <assert.h>


/// DECLARATIONS ///

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
BigInteger gcd(const BigInteger& a, const BigInteger& b);

class Rational;

Rational operator*(const Rational& a, const Rational& b);
Rational operator-(const Rational& a, const Rational& b);
Rational operator+(const Rational& a, const Rational& b);
Rational operator/(const Rational& a, const Rational& b);

/// Fft ///

namespace Fft {
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

void addTo2thPow(std::vector<std::complex<double>>& P) {
  while (P.size() & (P.size() - 1)) {
    P.push_back(0);
  }
}

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

    assert(base_ == other.base_);

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
    assert(!other.isZero());

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
    assert(!other.numerator_.isZero());

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

    // Добавляем знак, если надо
    if (numerator_.is_negative_) {
      fraction += '-';
    }

    auto carry = numerator_;
    carry.is_negative_ = false;

    // Добавляем целую часть числа
    auto first_part = (carry / denominator_).toString();
    fraction += first_part;

    // Если точность ноль знаков после запятой, то завершаемся
    if (precision == 0) {
      return fraction;
    }

    // Иначе добавляем точку
    fraction += '.';

    std::string zeros; // нули сразу после запятой
    carry %= denominator_; // остаток от деления целой части на делитель

    // Если число разделилось нацело, то просто надо добавить нулей
    if (carry == 0) {
      zeros.resize(precision);
      std::fill(zeros.begin(), zeros.end(), '0');
      fraction += zeros;
      return fraction;
    }

    // Высчитываем нули сразу после запятой
    // И дополняем остаток от целой части до минимального числа, 
    // которое можно делить на знаменатель
    while (carry < denominator_) {
      carry *= 10;
      zeros += '0';
    }

    // Был добавлен 1 лишний ноль
    zeros.pop_back();

    // Если у нас количества нулей уже столько, какова наша точность,
    // то завершаемся
    if (zeros.size() >= precision) {
      fraction += zeros.substr(0, precision);
      return fraction;
    }

    // Рассчитываем, сколько знаков после запятой осталось добрать
    // И приписываем к ответу нули, которые получили ранее
    precision -= zeros.size();
    fraction += zeros;

    // Вместо того, чтобы precision раз делить carry на знаменатель и
    // сносить цифры, умножаем carry на основание precision раз,
    // чтобы поделить единожны на знаменатель и сразу получить
    // precision цифр
    for (size_t i = 0; i < precision + 1; ++i) {
      carry *= numerator_.base_;
    }

    // Делим carry на знаменатель и приписываем к итоговой строке ответ
    auto second_part = (carry / denominator_).toString();
    fraction += second_part.substr(0, precision + 1);

    // Если на конце стоит цифра < 5, то не надо делать округление вверх,
    // можно просто завершиться
    if (fraction.back() < '5') {
      fraction.pop_back(); // precision + 1 цифра после запятой
      return fraction;
    }

    // Иначе удаляем precision + 1 цифру после запятой и округляем
    fraction.pop_back();
    size_t i = 0;
    size_t fsize = fraction.size();

    // Если цифра == 9, то она превращается в ноль и нам надо округлять дальше
    while (fraction[fsize - i - 1] == '9' && fraction[fsize - i - 1] != '.') {
      fraction[fsize - i - 1] = '0';
      ++i;
    }

    // Если мы не дошли до точки, то нужно просто увеличить последнюю цифру
    if (fraction[fsize - i - 1] != '.') {
      ++fraction[fsize - i - 1];
      return fraction;
    }

    // Иначе надо округлять и целую часть
    ++i;
    
    // Пока мы не дошли до начала строки или не встретили цифру < 9
    // Превращаем 9 в 0
    while (fsize >= i + 1 && fraction[fsize - i - 1] == '9') {
      fraction[fsize - i - 1] = '0';
      ++i;
    }

    // Если встретили цифру < 9, увеличиваем ее, завершаемся
    if (fsize >= i + 1) {
      ++fraction[fsize - i - 1];
      return fraction;
    }

    // Иначе приписываем 1 в начале дроби
    fraction = '1' + fraction;
    return fraction;
  }

  explicit operator double() {
    const int kPrecision = 40; // с запасом
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