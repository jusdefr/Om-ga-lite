#ifndef OMEGA_LITE_TEST_FRAMEWORK_H
#define OMEGA_LITE_TEST_FRAMEWORK_H

#include <cstdio>
#include <vector>

struct TestCase {
  const char * name;
  void (*function)();
};

inline std::vector<TestCase> & testCases() {
  static std::vector<TestCase> cases;
  return cases;
}

inline int & testFailureCount() {
  static int count = 0;
  return count;
}

inline int & testCheckCount() {
  static int count = 0;
  return count;
}

struct TestRegistrar {
  TestRegistrar(const char * name, void (*function)()) {
    testCases().push_back(TestCase{name, function});
  }
};

#define TEST(name)                                        \
  static void name();                                     \
  static TestRegistrar registrar_##name(#name, name);      \
  static void name()

#define CHECK(condition)                                                     \
  do {                                                                       \
    testCheckCount()++;                                                       \
    if (!(condition)) {                                                       \
      std::printf("    FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition);  \
      testFailureCount()++;                                                   \
    }                                                                        \
  } while (0)

#define CHECK_EQ(actual, expected) CHECK((actual) == (expected))

#endif
