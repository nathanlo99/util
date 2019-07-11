
#include "../string.h"

static unsigned int passed_tests = 0, num_tests = 0;
#define ASSERT(exp)                                                            \
  ((num_tests++, exp) ? (void)(passed_tests++)                                 \
                      : (void)printf("Assertion failed: %s:%d (%s)\n",         \
                                     __FILE__, __LINE__, #exp))

using string = util::string;

int main() {
  { // Default constructor
    string s;
    ASSERT(s.length() == 0);      // Should be empty
    ASSERT(s.c_str() != nullptr); // Should never be nullptr
  }

  { // Copy constructor
    string s{"Hello world"};
    string t{s};
    ASSERT(s == "Hello world");     // string (const char*) sanity check
    ASSERT(t == "Hello world");     // String is copied
    ASSERT(s == t);                 // Equality sanity check
    ASSERT(s.c_str() != t.c_str()); // Should be deep copy
  }

  { // Substring constructor
    string s{"Hello world"};
    { // Normal substring
      string t{s, 2, 5};
      ASSERT(t == "llo w");
    }
    { // Substring with len > str.size - pos
      string t{s, 2, 1000};
      ASSERT(t == "llo world");
    }
    { // Default length defaults to suffix
      string t{s, 2};
      ASSERT(t == "llo world");
    }
  }

  {   // string(const char*) constructor
    { // Constructing on empty string
      string empty{""};
      ASSERT(empty.length() == 0);
      ASSERT(empty.empty());
      ASSERT(empty.c_str() != nullptr);
      ASSERT(empty.c_str()[0] == 0);
    }
    { // Constructing on non-empty string
      string str{"Hello world!"};
      const unsigned int n = strlen("Hello world!");
      ASSERT(str.length() == n);
      ASSERT(str.c_str()[n] == 0);
      ASSERT(strcmp(str.c_str(), "Hello world!") == 0);
    }

    const char *tmp = "Test string";
    string tmp_str{tmp};
    ASSERT(tmp_str.c_str() != tmp); // We want a deep copy, not a shallow one
  }

  { // Buffer constructor
    string s{"Hello", 2};
    ASSERT(s == "He");

    string t{"Hello world", 5};
    ASSERT(t == "Hello");
  }

  {
    string s(10, 'x');
    ASSERT(s == "xxxxxxxxxx");

    string t(10u, 42);
    ASSERT(t == "**********");

    // TODO: Fix compilation error: it tries to run the range constructor
    /*
    string u(10, 42);
    ASSERT(t == "**********");
    */
  }

  { // string (string &&) constructor
    string s{"Testing"};
    string t{std::move(s)};
    ASSERT(t == "Testing");
  }

  printf("%d tests passed out of %d total tests!\n", passed_tests, num_tests);
}
