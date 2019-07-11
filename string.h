#ifndef UTIL_STRING
#define UTIL_STRING

#include <cassert>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

constexpr unsigned int roundup(unsigned int v) {
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  return v + 1;
}

// Defines a heap-allocated c-string
namespace util {
class string {
  static constexpr size_t npos = -1;
  static constexpr size_t max_size_t = std::numeric_limits<size_t>::max();
  using iterator = char *;
  using const_iterator = char *;
  using reverse_iterator = std::reverse_iterator<char *>;
  using const_reverse_iterator = std::reverse_iterator<char *>;
  size_t m_size, m_cap;
  char *m_data;

  string(size_t size, size_t cap, char *data)
      : m_size{size}, m_cap{cap}, m_data{data} {}

public:
  // Default constructor
  string() {
    m_size = 0;
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
  }

  // Copy constructor
  string(const string &other) {
    m_size = other.m_size;
    m_cap = other.m_cap;
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    for (size_t i = 0; i < m_size; ++i)
      m_data[i] = other.m_data[i];
  }

  // Substring constructor
  string(const string &str, size_t pos, size_t len = npos) {
    if (pos >= str.size())
      throw std::out_of_range{"pos out of range"};
    if (len == npos || pos + len >= str.m_size)
      len = str.m_size - pos;
    m_size = len;
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    for (size_t i = 0; i < m_size; ++i)
      m_data[i] = str.m_data[i + pos];
  }

  // From C-string constructor
  string(const char *str) {
    m_size = strlen(str);
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    for (size_t i = 0; i < m_size; ++i)
      m_data[i] = str[i];
  }

  // Buffer constructor
  string(const char *s, size_t n) {
    m_size = n;
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    for (size_t i = 0; i < m_size; ++i)
      m_data[i] = s[i];
  }

  // Fill constructor
  string(size_t n, char c) {
    m_size = n;
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    for (size_t i = 0; i < m_size; ++i)
      m_data[i] = c;
  }

  // Range constructor
  template <class InputIterator>
  string(InputIterator first, InputIterator last) {
    m_size = std::distance(first, last);
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    char *data_it = m_data;
    for (InputIterator it = first; it != last; ++it)
      *data_it = *it;
  }

  // Initializer list constructor
  string(std::initializer_list<char> il) {
    m_size = il.size();
    m_cap = roundup(m_size + 1);
    m_data = (char *)malloc(m_cap);
    if (!m_data)
      throw std::bad_alloc{};
    char *data_it = m_data;
    for (auto c : il)
      *data_it = c;
  }

  // Move constructor
  string(string &&other) noexcept {
    m_size = other.m_size;
    m_cap = other.m_cap;
    m_data = other.m_data;
    other.m_data =
        nullptr; // Under any other circumstances, m_data is not nullptr.
                 // This is to make sure this->m_data is not freed.
  }

  // Destructor
  ~string() { free(m_data); }

  // string assign
  string &operator=(const string &rhs) {
    if (this != &rhs)
      *this = rhs.c_str();
    return *this;
  }

  // c-string assign
  string &operator=(const char *str) {
    assert(str && "Tried to assign nullptr!");
    m_size = strlen(str);
    m_cap = roundup(m_size + 1);
    m_data = (char *)realloc(m_data, m_cap);
    for (size_t i = 0; i < m_size; ++i)
      m_data[i] = str[i];
    return *this;
  }

  // character assign
  string &operator=(char c) {
    if (m_cap < 4) {
      m_cap = 4;
      m_data = (char *)realloc(m_data, m_cap);
    }
    m_size = 1;
    m_data[0] = c;
    return *this;
  }

  // initializer_list assign
  string &operator=(std::initializer_list<char> il) {
    m_size = il.size();
    m_cap = roundup(m_size + 1);
    m_data = (char *)realloc(m_data, m_cap);
    char *data_it = m_data;
    for (auto c : il)
      *data_it = c;
    return *this;
  }

  // move assign
  string &operator=(string &&str) noexcept {
    free(m_data);
    m_size = str.m_size;
    m_cap = str.m_cap;
    m_data = str.m_data;
    str.m_data = nullptr; // See move constructor
    return *this;
  }

  iterator begin() noexcept { return m_data; }
  const_iterator begin() const noexcept { return m_data; }
  iterator end() noexcept { return m_data + m_size; }
  const_iterator end() const noexcept { return m_data + m_size; }
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(m_data + (m_size - 1));
  }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(m_data + (m_size - 1));
  }
  reverse_iterator rend() noexcept { return reverse_iterator(m_data - 1); }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(m_data - 1);
  }

  const_iterator cbegin() const noexcept { return m_data; }
  const_iterator cend() const noexcept { return m_data + m_size; }
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(m_data + (m_size - 1));
  }
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(m_data - 1);
  }

  size_t size() const noexcept { return m_size; }
  size_t length() const noexcept { return m_size; }
  size_t max_size() const noexcept { return max_size_t; }

  void resize(size_t n, char c = '\0') {
    const size_t old_size = m_size;
    if (n < m_size)
      m_size = n;
    else {
      m_size = n;
      m_cap = roundup(m_size + 1);
      const char *new_data = m_data = (char *)realloc(m_data, m_cap);
      for (size_t i = old_size; i < n; ++i)
        m_data[i] = c;
    }
  }
  size_t capacity() const noexcept { return m_cap; }
  void reserve(size_t n = 0) {
    if (n > m_cap) {
      m_cap = roundup(n);
      m_data = (char *)realloc(m_data, m_cap);
    }
  }
  void clear() noexcept {
    m_size = 0;
    m_cap = roundup(m_size + 1);
    m_data = (char *)realloc(m_data, m_cap);
  }
  bool empty() const noexcept { return !m_size; }
  void shrink_to_fit() {
    m_cap = roundup(m_size + 1);
    m_data = (char *)realloc(m_data, m_cap);
  }

  char &operator[](size_t pos) {
    m_data[m_size] = 0;
    return m_data[pos];
  }
  const char &operator[](size_t pos) const {
    m_data[m_size] = 0;
    return m_data[pos];
  }
  char &at(size_t pos) {
    if (pos >= m_size)
      throw std::out_of_range{"pos out of range"};
    return m_data[pos];
  }
  const char &at(size_t pos) const {
    if (pos >= m_size)
      throw std::out_of_range{"pos out of range"};
    return m_data[pos];
  }
  char &back() { return m_data[m_size - 1]; }
  const char &back() const { return m_data[m_size - 1]; }
  char &front() { return m_data[0]; }
  const char &front() const { return m_data[0]; }

  const string operator+(const char *str) const {
    const size_t new_size = m_size + strlen(str);
    size_t new_cap = roundup(new_size + 1);
    char *new_data = (char *)malloc(new_cap);
    // NOTE: not realloc as we want a new version of the string
    for (size_t i = 0; i < m_size; ++i)
      new_data[i] = m_data[i];
    for (size_t i = m_size; i < new_size; ++i)
      new_data[i] = str[i - m_size];
    return string{new_size, new_cap, new_data};
  }

  // TODO: Verify that no optimizations are possible
  string &operator+=(const string &rhs) { return *this = *this + rhs; }
  string &operator+=(const char *str) { return *this = *this + str; }

  const string operator+(const string &other) const {
    return *this + other.m_data;
  }

  bool operator==(const char *other) const {
    if (!other || !m_data)
      return 0;
    for (size_t i = 0; i < m_size; ++i)
      if (m_data[i] != other[i])
        return 0;
    if (other[m_size] != 0)
      return 0;
    return 1;
  }

  bool operator==(const string &other) const {
    if (m_size != other.m_size)
      return 0;
    for (size_t i = 0; i < m_size; ++i)
      if (m_data[i] != other.m_data[i])
        return 0;
    return 1;
  }

  bool operator!=(const string &other) const { return !(*this == other); }

  const char *c_str() const {
    assert(m_size + 1 < m_cap);
    m_data[m_size] = 0;
    return m_data;
  }

  friend std::ostream &operator<<(std::ostream &os, const string &str) {
    return os << str.c_str();
  }
}; // class string
} // namespace util
#endif // #ifndef UTIL_STRING
