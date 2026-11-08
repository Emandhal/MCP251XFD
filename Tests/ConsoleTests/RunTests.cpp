/*!*****************************************************************************
 * @file    RunTests.cpp
 * @brief   Console runner of the unit tests of Tests/UnitTest (see CppUnitTest.h)
 * @details Usage: run_tests [-v] [name]
 *            -v    print all the failed assertions (default: the 3 first of each test)
 *            name  run only the tests whose name contains this text
 ******************************************************************************/
#include "CppUnitTest.h"
#include <cstring>

int main(int argc, char** argv)
{
  const char* filter = nullptr;
  for (int i = 1; i < argc; i++)
  {
    if (std::strcmp(argv[i], "-v") == 0) ut::S().MaxPrinted = 0;
    else filter = argv[i];
  }

  ut::State& s = ut::S();
  for (size_t i = 0; i < s.Inits.size(); i++) s.Inits[i]();           // Class initializers

  unsigned run = 0, failedTests = 0, assertions = 0, failures = 0;
  for (size_t i = 0; i < s.Tests.size(); i++)
  {
    const ut::TestInfo& t = s.Tests[i];
    if ((filter != nullptr) && (std::strstr(t.Name, filter) == nullptr)) continue;
    s.Current = t.Name; s.Assertions = 0; s.Failures = 0; s.Printed = 0;
    t.Func();
    run++; assertions += s.Assertions; failures += s.Failures;
    if (s.Failures > 0) failedTests++;
    std::printf("[%s] %-60s %5u assertions, %u failed\n", (s.Failures == 0 ? " OK " : "FAIL"), t.Name, s.Assertions, s.Failures);
  }
  for (size_t i = 0; i < s.Cleanups.size(); i++) s.Cleanups[i]();      // Class cleanups

  std::printf("\n%u test(s), %u assertion(s): %u test(s) failed, %u assertion(s) failed\n", run, assertions, failedTests, failures);
  return (failedTests == 0 ? 0 : 1);
}
