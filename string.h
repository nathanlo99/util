#ifndef UTIL_STRING
#define UTIL_STRING

#include <cassert>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

// TODO: Rewrite parts of this with C string functions wherever possible.
static constexpr unsigned int roundup(unsigned int v) {
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  return v + 1;
}

// Defines a heap-allocated c-string
namespace util {
template <class charT, class traits = char_traits<charT>,
          class Alloc = allocator<charT>>
class basic_string {
  using value_type = charT;
  using traits_type = std::char_traits<value_type>;
  using allocator_type = std::allocator<value_type>;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = allocator_traits<allocator_type>::pointer;
  using const_pointer = allocator_traits<allocator_type>::const_pointer;
  using iterator = value_type *;
  using const_iterator = const value_type *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<iterator>;
  using difference_type = allocator_traits<allocator_type>::difference_type;
  using size_type = allocator_traits<allocator_type>::size_type;
  static constexpr size_type char_width = sizeof(charT);
  static constexpr size_type npos = -1;

  size_type m_size, m_cap;
  allocator_type m_alloc;
  pointer m_data;

public:
  // Default constructor
  basic_string() : m_size{0}, m_cap{2}, m_alloc{}, m_data{m_alloc(m_cap)} {}
  explicit basic_string(const allocator_type &alloc)
      : m_size{0}, m_cap{2}, m_alloc{alloc}, m_data{m_alloc(m_cap)} {}

  // Copy constructor
  basic_string(const basic_string &str);
  basic_string(const basic_string &str, const allocator_type &alloc);

  // Substring constructor
  string(const string &str, size_t pos, size_t len = npos);

  // From C-string constructor
  string(const char *str);

  // Buffer constructor
  string(const char *s, size_t n);

  // Fill constructor
  string(size_t n, char c);

  // Range constructor
  template <class InputIterator>
  string(InputIterator first, InputIterator last);

  // Initializer list constructor
  string(std::initializer_list<char> il);

  // Move constructor
  string(string &&other) noexcept;

  // Destructor
  ~string() { free(m_data); }

  // string assign
  string &operator=(const string &rhs);

  // c-string assign
  string &operator=(const char *str);

  // character assign
  string &operator=(char c);

  // initializer_list assign
  string &operator=(std::initializer_list<char> il);

  // move assign
  string &operator=(string &&str) noexcept;

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
  size_t max_size() const noexcept { return npos; }

  void resize(size_t n, char c = '\0');
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
  void shrink_to_fit();

  char &operator[](size_t pos) { return m_data[pos]; }
  const char &operator[](size_t pos) const { return m_data[pos]; }
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

  string &operator+=(const string &str);                           // TODO
  string &operator+=(const char *s);                               // TODO
  string &operator+=(char c);                                      // TODO
  string &operator+=(std::initializer_list<char> il);              // TODO
  string &append(const string &str);                               // TODO
  string &append(const string &str, size_t subpos, size_t sublen); // TODO
  string &append(const char *s);                                   // TODO
  string &append(const char *s, size_t n);                         // TODO
  template <class InputIterator>
  string &append(InputIterator first, InputIterator last); // TODO
  string &append(std::initializer_list<char> il);          // TODO
  void push_back(char c);                                  // TODO
  string &assign(const string &str);                       // TODO
  string &assign(const string &str, size_t subpos,
                 size_t sublen = npos);                    // TODO
  string &assign(const char *s);                           // TODO
  string &assign(const char *s, size_t n);                 // TODO
  string &assign(size_t n, char c);                        // TODO
  template <class InputIterator>                           // TODO
  string &assign(InputIterator first, InputIterator last); // TODO
  string &assign(std::initializer_list<char> il);          // TODO
  string &assign(string &&str) noexcept;                   // TODO
  string &insert(size_t pos, const string &str);           // TODO
  string &insert(size_t pos, const string &str, size_t subpos,
                 size_t sublen = npos);                                 // TODO
  string &insert(size_t pos, const char *s);                            // TODO
  string &insert(size_t pos, const char *s, size_t n);                  // TODO
  string &insert(size_t pos, size_t n, char c);                         // TODO
  iterator insert(const_iterator p, size_t n, char c);                  // TODO
  iterator insert(const_iterator p, char c);                            // TODO
  template <class InputIterator>                                        // TODO
  iterator insert(iterator p, InputIterator first, InputIterator last); // TODO
  string &insert(const_iterator p, std::initializer_list<char> il);     // TODO
  string &erase(size_t pos = 0, size_t len = npos);                     // TODO
  iterator erase(const_iterator p);                                     // TODO
  iterator erase(const_iterator first, const_iterator last);            // TODO

  string &replace(size_t pos, size_t len, const string &str);
  string &replace(const_iterator i1, const_iterator i2, const string &str);
  string &replace(size_t pos, size_t len, const string &str, size_t subpos,
                  size_t sublen = npos);
  string &replace(size_t pos, size_t len, const char *s);
  string &replace(const_iterator i1, const_iterator i2, const char *s);
  string &replace(size_t pos, size_t len, const char *s, size_t n);
  string &replace(const_iterator i1, const_iterator i2, const char *s,
                  size_t n);
  string &replace(size_t pos, size_t len, size_t n, char c);
  string &replace(const_iterator i1, const_iterator i2, size_t n, char c);
  template <class InputIterator>
  string &replace(const_iterator i1, const_iterator i2, InputIterator first,
                  InputIterator last);
  string &replace(const_iterator i1, const_iterator i2,
                  std::initializer_list<char> il);
  void swap(string &str);
  void pop_back() { --m_size; }

  const char *c_str() const;
  const char *data() const;

  allocator_type get_allocator() const noexcept;

  size_t copy(char *s, size_t len, size_t pos = 0) const;
  size_t find(const string &str, size_t pos = 0) const noexcept;
  size_t find(const char *s, size_t pos = 0) const;
  size_t find(const char *s, size_t pos, size_type n) const;
  size_t find(char c, size_t pos = 0) const noexcept;

  size_t rfind(const string &str, size_t pos = npos) const noexcept;
  size_t rfind(const char *s, size_t pos = npos) const;
  size_t rfind(const char *s, size_t pos, size_t n) const;
  size_t rfind(char c, size_t pos = npos) const noexcept;

  size_t find_first_of(const string &str, size_t pos = 0) const noexcept;
  size_t find_first_of(const char *s, size_t pos = 0) const;
  size_t find_first_of(const char *s, size_t pos, size_t n) const;
  size_t find_first_of(char c, size_t pos = 0) const noexcept;

  size_t find_last_of(const string &str, size_t pos = npos) const noexcept;
  size_t find_last_of(const char *s, size_t pos = npos) const;
  size_t find_last_of(const char *s, size_t pos, size_t n) const;
  size_t find_last_of(char c, size_t pos = npos) const noexcept;

  size_t find_first_not_of(const string &str, size_t pos = 0) const noexcept;
  size_t find_first_not_of(const char *s, size_t pos = 0) const;
  size_t find_first_not_of(const char *s, size_t pos, size_t n) const;
  size_t find_first_not_of(char c, size_t pos = 0) const noexcept;

  size_t find_last_not_of(const string &str, size_t pos = npos) const noexcept;
  size_t find_last_not_of(const char *s, size_t pos = npos) const;
  size_t find_last_not_of(const char *s, size_t pos, size_t n) const;
  size_t find_last_not_of(char c, size_t pos = npos) const noexcept;

  string substr(size_t pos = 0, size_t len = npos) const;

  int compare(const string &str) const noexcept;
  int compare(size_t pos, size_t len, const string &str) const;
  int compare(size_t pos, size_t len, const string &str, size_t subpos,
              size_t sublen = npos) const;
  int compare(const char *s) const;
  int compare(size_t pos, size_t len, const char *s) const;
  int compare(size_t pos, size_t len, const char *s, size_t n) const;

  friend std::ostream &operator<<(std::ostream &os, const string &str) {
    return os << str.c_str();
  }
}; // class string

string operator+(const string &lhs, const string &rhs);
string operator+(string &&lhs, string &&rhs);
string operator+(string &&lhs, const string &rhs);
string operator+(const string &lhs, string &&rhs);
string operator+(const string &lhs, const char *rhs);
string operator+(string &&lhs, const char *rhs);
string operator+(const char *lhs, const string &rhs);
string operator+(const char *lhs, string &&rhs);
string operator+(const string &lhs, char rhs);
string operator+(string &&lhs, char rhs);
string operator+(char lhs, const string &rhs);
string operator+(char lhs, string &&rhs);

bool operator==(const string &lhs, const string &rhs) noexcept;
bool operator==(const char *lhs, const string &rhs);
bool operator==(const string &lhs, const char *rhs);
bool operator!=(const string &lhs, const string &rhs) noexcept;
bool operator!=(const char *lhs, const string &rhs);
bool operator!=(const string &lhs, const char *rhs);
bool operator<(const string &lhs, const string &rhs) noexcept;
bool operator<(const char *lhs, const string &rhs);
bool operator<(const string &lhs, const char *rhs);
bool operator<=(const string &lhs, const string &rhs) noexcept;
bool operator<=(const char *lhs, const string &rhs);
bool operator<=(const string &lhs, const char *rhs);
bool operator>(const string &lhs, const string &rhs) noexcept;
bool operator>(const char *lhs, const string &rhs);
bool operator>(const string &lhs, const char *rhs);
bool operator>=(const string &lhs, const string &rhs) noexcept;
bool operator>=(const char *lhs, const string &rhs);
bool operator>=(const string &lhs, const char *rhs);

void swap(string &x, string &y);
istream &operator>>(istream &is, string &str);
ostream &operator<<(ostream &os, const string &str);

istream &getline(istream &is, string &str, char delim);
istream &getline(istream &&is, string &str, char delim);
istream &getline(istream &is, string &str);
istream &getline(istream &&is, string &str);

} // namespace util
#endif // #ifndef UTIL_STRING
