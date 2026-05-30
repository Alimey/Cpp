#include <iostream>
#include <cstring>
#include <algorithm>

class CowString {
 public:
  char* buffer;
  size_t sz = 0;
  size_t cap = 1;
  static constexpr size_t kShift = 8;

 private:
  static size_t& userCount(char* buf) {
    return *reinterpret_cast<size_t*>(buf);
  }

  void make_copy() {
    if (userCount(buffer) <= 1) {
      return;
    }
    userCount(buffer) -= 1;
    char* new_buffer = new char[kShift + cap];
    userCount(new_buffer) = 1;
    std::copy(buffer + kShift, buffer + kShift + cap, new_buffer + kShift);
    buffer = new_buffer;
  }

  void resize_buffer(size_t add) {
    char* new_buffer = new char[kShift + add + 1];
    userCount(new_buffer) = 1;
    std::copy(buffer + kShift, buffer + kShift + sz, new_buffer + kShift);
    if (userCount(buffer) > 1) {
      userCount(buffer) -= 1;
    } else {
      delete[] buffer;
    }
    buffer = new_buffer;
    cap = add + 1;
  }

  CowString(size_t n):
      buffer(new char[kShift + n + 1])
      , sz(n)
      , cap(n + 1) {
    userCount(buffer) = 1;
  }

 public:
  CowString():
      buffer(new char[kShift + 1])
      , sz(0)
      , cap(1) {
    userCount(buffer) = 1;
    buffer[kShift] = '\0';
  }

  CowString(size_t n, char c): 
      buffer(new char[kShift + n + 1])
      , sz(n)
      , cap(n + 1) {
    userCount(buffer) = 1;
    std::fill(buffer + kShift, buffer + kShift + sz, c);
    buffer[sz + kShift] = '\0';
  }

  CowString(const char* c_style): 
      buffer(new char[kShift + strlen(c_style) + 1])
      , sz(strlen(c_style))
      , cap(strlen(c_style) + 1) {
    userCount(buffer) = 1;
    std::copy(c_style, c_style + sz, buffer + kShift);
    buffer[sz + kShift] = '\0';
  }

  CowString(const CowString& other):
      buffer(other.buffer) 
      , sz(other.sz)
      , cap(other.cap) {
    userCount(buffer) += 1;
  }

  CowString& operator=(const CowString& other) {
    if (this == &other) {
      return *this;
    }
    auto copy = other;
    swap(copy);
    return *this;
  }

  void swap(CowString& other) {
    std::swap(buffer, other.buffer);
    std::swap(sz, other.sz);
    std::swap(cap, other.cap);
  }

  CowString& operator+=(const CowString& other) {
    if (other.sz + sz > cap - 1) {
      resize_buffer(sz + other.sz);
    } else {
      make_copy();
    }
    std::copy(other.buffer + kShift, other.buffer + kShift + other.sz + 1, buffer + kShift + sz);
    sz += other.sz;
    return *this;
  }

  CowString& operator+=(char c) {
    if (sz + 1 > cap - 1) {
      resize_buffer(2 * cap);
    } else {
      make_copy();
    }
    buffer[sz + kShift] = c;
    ++sz;
    buffer[sz + kShift] = '\0';
    return *this;
  }

  char& operator[](size_t i) {
    make_copy();
    return buffer[kShift + i];
  }

  const char& operator[](size_t i) const {
    return buffer[kShift + i];
  }

  size_t length() const {
    return sz;
  }

  size_t size() const {
    return sz;
  }

  size_t capacity() const {
    return cap - 1;
  }

  void push_back(char c) {
    operator+=(c);
  }

  void pop_back() {
    make_copy();
    --sz;
    buffer[sz + kShift] = '\0';
  }

  char& front() {
    make_copy();
    return buffer[kShift];
  }

  const char& front() const {
    return buffer[kShift];
  }

  char& back() {
    make_copy();
    return buffer[sz + kShift - 1];
  }

  const char& back() const {
    return buffer[sz + kShift - 1];
  }

  size_t find(const CowString& substr) const {
    if (substr.sz > sz) {
      return length();
    }
    for (size_t i = 0; i < sz - substr.sz + 1; ++i) {
      if (memcmp(&buffer[kShift + i], substr.buffer + substr.kShift, substr.sz) == 0) {
        return i;
      }
    }
    return length();
  }

  size_t rfind(const CowString& substr) const {
    if (substr.sz > sz) {
      return length();
    }
    for (size_t i = sz - substr.sz; i--; ) {
      if (memcmp(&buffer[kShift + i], substr.buffer + substr.kShift, substr.sz) == 0) {
        return i;
      }
    }
    return length();
  }

  CowString substr(size_t start, size_t count) const {
    if (count > sz) {
      count = sz - start;
    }
    CowString sub(count);
    std::copy(buffer + kShift + start, buffer + kShift + start + count, sub.buffer + kShift);
    sub.buffer[kShift + count] = '\0';
    return sub;
  }

  bool empty() const {
    return sz == 0;
  }

  void clear() {
    if (userCount(buffer) > 1) {
      userCount(buffer) -= 1;
      buffer = new char[kShift + 1];
      userCount(buffer) = 1;
      cap = 1;
    }
    sz = 0;
    buffer[kShift] = '\0';
  }

  void shrink_to_fit() {
    if (sz == cap - 1) {
      return;
    }
    resize_buffer(sz);
  }

  char* data() {
    make_copy();
    return buffer + kShift;
  }

  const char* data() const {
    return buffer + kShift;
  }

  ~CowString() {
    if (userCount(buffer) > 1) {
      userCount(buffer) -= 1;
      return;
    }
    delete[] buffer;
  }
};

CowString operator+(const CowString& a, const CowString& b) {
  CowString s = a;
  s += b;
  return s;
}

CowString operator+(char a, const CowString& b) {
  CowString s(1, a);
  s += b;
  return s;
}

CowString operator+(const CowString& b, char a) {
  CowString s = b;
  s += a;
  return s;
}

std::ostream& operator<<(std::ostream& out, const CowString& s) {
  for (size_t i = 0; i < s.size(); ++i) {
    out << s.buffer[CowString::kShift + i];
  }
  return out;
}

std::istream& operator>>(std::istream& in, CowString& s) {
  s.clear();
  char ch = in.get();
  while (isspace(ch)) {
    ch = in.get();
  }
  while (!isspace(ch) && ch != EOF) {
    s += ch;
    ch = in.get();
  }
  return in;
}

bool operator<(const CowString& first, const CowString& other) {
    int result_of_comparing = memcmp(first.buffer + CowString::kShift, other.buffer + CowString::kShift, std::min(first.sz, other.sz));
    return (result_of_comparing == 0 && first.sz < other.sz) || result_of_comparing < 0;
  }

bool operator>(const CowString& first, const CowString& other) {
    return other < first;
  }

bool operator==(const CowString& first, const CowString& other) {
    return memcmp(first.buffer + CowString::kShift, other.buffer + CowString::kShift, first.sz) == 0 && first.sz == other.sz;
  }

bool operator<=(const CowString& first, const CowString& other) {
    return !(first > other);
  }

bool operator>=(const CowString& first, const CowString& other) {
    return !(first < other);
  }

bool operator!=(const CowString& first, const CowString& other) {
  return !(first == other);
}