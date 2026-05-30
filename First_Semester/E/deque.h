#include <iostream>
#include <compare>
#include <type_traits>

template <typename T>
class Deque {
  public:
  static constexpr size_t cBucketSize = 24;

  size_t ext_start = 1;
  size_t int_start = 0;
  size_t sz = 0;
  size_t cap = sz / cBucketSize + 3; // размер внешнего массива дека
  T** data = new T*[cap];

  Deque() {
    for (size_t i = 0; i < cap; ++i) {
      data[i] = createBucket();
    }
  }

  explicit Deque(int size): sz(size) {
    static_assert(std::is_default_constructible_v<T>);
    for (size_t i = 0; i < cap; ++i) {
      data[i] = createBucket();

      if (i == 0 || i == cap - 1) {
        continue;
      }

      for (size_t j = 0; j < cBucketSize && size > 0; ++j, --size) {
        try {
          new (data[i] + j) T();
        } catch (...) {
          destroy(i, j);
          clear(i);
          delete[] data;
          throw;
        }
      }
    }
  }

  Deque(int size, const T& element): sz(size) {
    size_t it_begin = int_start;

    for (size_t i = 0; i < cap; ++i) {
      data[i] = createBucket();

      if (i < ext_start || size == 0) {
        continue;
      }

      for (size_t j = it_begin; j < cBucketSize && size > 0; ++j, --size) {
        putElement(element, i, j);
      }

      it_begin = 0;
    }
  }

  Deque(const Deque& other)
      : ext_start(other.ext_start)
      , int_start(other.int_start)
      , sz(other.sz)
      , cap(other.cap)
      , data(new T*[cap]) {
    size_t size = sz;
    size_t it_begin = int_start;

    for (size_t i = 0; i < cap; ++i) {
      data[i] = createBucket();

      if (i < ext_start || size == 0) {
        continue;
      }

      for (size_t j = it_begin; j <= cBucketSize && size > 0; ++j, --size) {
        putElement(*(other.data[i] + j), i, j);
      }

      it_begin = 0;
    }
  }

  ~Deque() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (auto it = begin(); it != end(); ++it) {
        it->~T();
      }
    }

    for (size_t i = 0; i < cap; ++i) {
      ::operator delete(data[i], static_cast<std::align_val_t>(alignof(T)));
    }

    delete[] data;
  }

  Deque& operator=(const Deque& other) {
    if (this == &other) {
      return *this;
    }

    Deque copy = other;
    swap(copy);

    return *this;
  }

  void swap(Deque<T>& other) {
    std::swap(data, other.data);
    std::swap(ext_start, other.ext_start);
    std::swap(int_start, other.int_start);
    std::swap(sz, other.sz);
    std::swap(cap, other.cap);
  }

  size_t size() const {
    return sz;
  }

  T& operator[](size_t index) {
    return *(begin() + index);
  }

  const T& operator[](size_t index) const {
    return *(begin() + index);
  }

  T& at(size_t index) {
    if (get_real_ext_index(index) >= get_ext_end() 
        && get_real_int_index(index) >= get_int_end()) {
      throw std::out_of_range("index is out of bound");
    }

    return operator[](index);
  }

  const T& at(size_t index) const {
    if (get_real_ext_index(index) >= get_ext_end() 
        && get_real_int_index(index) >= get_int_end()) {
      throw std::out_of_range("index is out of bound");
    }

    return operator[](index);
  }

  void push_back(const T& element) {
    T** saving = data;

    if (get_ext_end() == cap - 1 && get_int_end() == cBucketSize - 1) {
      data = reserve();
    }

    try {
      new (data[get_real_ext_index(sz)] + get_real_int_index(sz)) T(element);
    } catch (...) {
      if (data != saving) {
        clearAllocated();
        delete[] data;
      }

      data = saving;
      throw;
    }

    if (data != saving) {
      delete[] saving;
    }

    ++sz;
  }

  void pop_back() {
    rbegin()->~T();
    --sz;
  }


  void push_front(const T& element) {
    T** saving = data;

    if (data == nullptr || (ext_start == 0 && int_start == 0)) {
      data = reserve();
    }

    if (int_start == 0) {
      --ext_start;
      int_start = cBucketSize - 1;
    } else {
      --int_start;
    }

    try {
      new (data[ext_start] + int_start) T(element);
    } catch (...) {
      if (data != saving) {
        clearAllocated();
        delete[] data;
      }

      data = saving;
      throw;
    }

    if (data != saving) {
      delete[] saving;
    }

    ++sz;
  }

  void pop_front() {
    begin()->~T();
    --sz;

    if (int_start == cBucketSize - 1) {
      int_start = 0;
      ++ext_start;
    } else {
      ++int_start;
    }
  }

  template <bool is_const>
  struct basic_iterator {
    using value_type = std::conditional_t<is_const, const T, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;

    T** deque_bucket;
    size_t cell_pos;

    basic_iterator(T** deque_bucket, size_t cell_pos)
        : deque_bucket(deque_bucket), cell_pos(cell_pos) {}

    operator basic_iterator<true>() const {
      return basic_iterator<true>(deque_bucket, cell_pos);
    }

    basic_iterator& operator++() {
      if (cell_pos == Deque<T>::cBucketSize - 1) {
        cell_pos = 0;
        ++deque_bucket;
      } else {
        ++cell_pos;
      }

      return *this;
    }

    basic_iterator operator++(int) {
      auto copy = *this;
      operator++();
      return copy;
    }

    basic_iterator& operator--() {
      if (cell_pos == 0) {
        cell_pos = Deque<T>::cBucketSize - 1;
        --deque_bucket;
      } else {
        --cell_pos;
      }

      return *this;
    }

    basic_iterator operator--(int) {
      auto copy = *this;
      operator--();
      return copy;
    }

    basic_iterator& operator+=(ssize_t val) {
      ssize_t total_pos = static_cast<ssize_t>(cell_pos) + val;
      ssize_t long_bucketSize = static_cast<ssize_t>(cBucketSize);

      if (total_pos < 0) {
          deque_bucket += (total_pos - (long_bucketSize - 1)) / long_bucketSize; // Округление вниз
      } else {
          deque_bucket += total_pos / long_bucketSize;
      }

      cell_pos = (total_pos % long_bucketSize + long_bucketSize) % long_bucketSize; // Безопасный модуль
      return *this;
  }

    basic_iterator& operator-=(ssize_t val) {
      return *this += -val;
    }

    basic_iterator operator+(ssize_t val) const {
      auto ans = *this;
      ans += val;
      return ans;
    }

    basic_iterator operator-(ssize_t val) const {
      auto ans = *this;
      ans -= val;
      return ans;
    }

    difference_type operator-(const basic_iterator& second) const {
      ssize_t long_bucketSize = static_cast<ssize_t>(Deque<T>::cBucketSize);
      ssize_t llfirst = static_cast<ssize_t>(cell_pos);
      ssize_t llsecond = static_cast<ssize_t>(second.cell_pos);

      return (deque_bucket - second.deque_bucket) * long_bucketSize + (llfirst - llsecond);
    }

    std::strong_ordering operator<=>(const basic_iterator& other) const = default;

    bool operator==(const basic_iterator& other) const = default;

    reference operator*() const {
      return *operator->();
    }

    pointer operator->() const {
      return (*deque_bucket) + cell_pos;
    }
  };

  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() {
    return iterator(data + ext_start, int_start);
  }

  iterator end() {
    if (sz == 0) {
      return begin();
    }

    size_t int_end = get_int_end();
    size_t ext_end = get_ext_end();

    if (int_end == cBucketSize - 1) {
      return iterator(data + ext_end + 1, 0);
    }

    return iterator(data + ext_end, int_end + 1);
  }

  const_iterator begin() const {
    return const_iterator(data + ext_start, int_start);
  }

  const_iterator end() const {
    if (sz == 0) {
      return begin();
    }

    size_t int_end = get_int_end();
    size_t ext_end = get_ext_end();

    if (int_end == cBucketSize - 1) {
      return const_iterator(data + ext_end + 1, 0);
    }

    return const_iterator(data + ext_end, int_end + 1);
  }

  const_iterator cbegin() const {
    return const_iterator(begin());
  }

  const_iterator cend() const {
    return const_iterator(end());
  }

  reverse_iterator rbegin() {
    return reverse_iterator(end());
  }

  reverse_iterator rend() {
    return reverse_iterator(begin());
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(cbegin());
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(cend());
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  void insert(const_iterator const_it, const T& value) {
    T save = value;
    iterator it{const_it.deque_bucket, const_it.cell_pos};

    if (it == begin()) {
      push_front(value);
      return;
    }

    while (it != end()) {
      std::swap(*it, save);
      ++it;
    }

    push_back(save);
  }

  void erase(const_iterator const_it) {
    iterator it{const_it.deque_bucket, const_it.cell_pos};

    if (it == begin()) {
      pop_front();
      return;
    }

    while (it != end()) {
      std::swap(*it, *(it + 1));
      ++it;
    }

    pop_back();
  }

 private:
  size_t get_ext_end() const {
    if (sz == 0) {
      return ext_start;
    }

    return ext_start + (int_start + sz - 1) / cBucketSize;
  }

  size_t get_int_end() const {
    if (sz == 0) {
      return int_start;
    }

    return (int_start + sz - 1) % cBucketSize;
  }

  size_t get_real_ext_index(size_t index) const {
    return ext_start + (int_start + index) / cBucketSize;
  }

  size_t get_real_int_index(size_t index) const {
    return (int_start + index) % cBucketSize;
  }

  T** reserve() {
    T** newdata = new T*[cap * 3];
    std::copy(data, data + cap, newdata + cap);

    for (size_t i = 0; i < cap; ++i) {
      newdata[i] = createBucket();
    }

    for (size_t i = 2 * cap; i < 3 * cap; ++i) {
      newdata[i] = createBucket();
    }

    ext_start += cap;
    cap *= 3;

    return newdata;
  }

  T* createBucket() {
    return static_cast<T*>(::operator new(cBucketSize * sizeof(T), static_cast<std::align_val_t>(alignof(T))));
  }

  void putElement(const T& element, size_t bucket, size_t cell) {
    try {
      new (data[bucket] + cell) T(element);
    } catch (...) {
      destroy(bucket, cell);
      clear(bucket);
      delete[] data;
      throw;
    }
  }

  void destroy(size_t ex_stop, size_t in_stop) const {
    for (iterator it = {data + ex_stop, in_stop}; it != begin(); --it) {
      it->~T();
    }
  }

  void clear(size_t stop, size_t start = 0) const {
    for (; stop != start - 1; --stop) {
      ::operator delete(data[stop], static_cast<std::align_val_t>(alignof(T)));
    }
  }

  void clearAllocated() {
    clear(cap / 3 - 1);
    clear(cap - 1, 2 * cap / 3);
  }
};