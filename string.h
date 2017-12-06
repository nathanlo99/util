#ifndef UTIL_STRING
#define UTIL_STRING

#include <cstring>
#include <iostream>

// Defines a heap-allocated c-string
namespace util {
class string {
private:
  char *s = NULL;
  size_t size = 0;

public:
  inline string() : s(NULL), size(0) {}

  inline string(const char *str) {
    if (str == NULL) {
      s = NULL;
      size = 0;
    } else {
      size = strlen(str);
      s = (char *)malloc(size * sizeof(char));
      strcpy(s, str);
    }
  }

  inline string(const string &str) { string(str.c_str()); }

  inline string(const char c) {
    size = 1;
    s = (char *)malloc(size * sizeof(char));
    *s = c;
  }

  inline ~string() {
    if (s != NULL)
      free(s);
  }

  inline string &operator=(const char *str) {
    if (str == NULL) {
      if (s == NULL)
        free(s);
      size = 0;
    } else {
      size = strlen(str);
      s = (char *)realloc(s, size * sizeof(char));
      strcpy(s, str);
    }
    return *this;
  }

  inline const string operator+(const char *str) const {
    const int new_size = size + strlen(str);
    char *ss = (char *)malloc(new_size * sizeof(char));
    strcpy(ss, s);
    strcat(ss, str);
    const string res(ss);
    return res;
  }

  inline string &operator=(const string &rhs) {
    if (this != &rhs)
      *this = rhs.c_str();
    return *this;
  }

  inline string &operator+=(const string &rhs) {
    *this = *this + rhs;
    return *this;
  }

  inline string &operator+=(const char *str) {
    *this = *this + str;
    return *this;
  }

  inline const string operator+(const string &other) const {
    return *this + other.c_str();
  }

  inline bool operator==(const string &other) const {
    if (s == NULL || other.c_str() == NULL)
      return 0;
    if (size != other.length())
      return 0;
    return strcmp(s, other.c_str()) == 0;
  }

  inline bool operator!=(const string &other) const {
    return !(*this == other);
  }

  inline int length() const { return size; }
  inline char *c_str() const { return s; }

  friend std::ostream &operator<<(std::ostream &os, const string &str) {
    return os << str.s;
  }

}; // class string

} // namespace util

#endif // #ifndef UTIL_STRING
