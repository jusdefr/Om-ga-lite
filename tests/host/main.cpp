#include "test_framework.h"

int main() {
  for (const TestCase & testCase : testCases()) {
    testCase.function();
  }
  const int failures = testFailureCount();
  std::printf("\n%zu test functions, %d checks, %d failure(s)\n",
              testCases().size(),
              testCheckCount(),
              failures);
  return failures == 0 ? 0 : 1;
}
