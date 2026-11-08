/*!*****************************************************************************
 * @file    CppUnitTest.h
 * @brief   Tiny replacement of Microsoft's CppUnitTest.h to run the unit tests of
 *          Tests/UnitTest as a console program (MSVC, GCC and Clang)
 * @details Only what the unit tests need is implemented: Assert::AreEqual,
 *          Assert::IsTrue, Assert::Fail, Logger::WriteMessage, TEST_CLASS, TEST_METHOD
 *          and TEST_CLASS_INITIALIZE. A failed assertion is counted and printed but does
 *          not stop the test method, so a run lists all the differences at once.
 *          C++17 is needed (inline static data members)
 ******************************************************************************/
#ifndef CPPUNITTEST_SHIM_H_INC
#define CPPUNITTEST_SHIM_H_INC
//-----------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>
//-----------------------------------------------------------------------------

#define RETURN_WIDE_STRING(value)  std::wostringstream _s; _s << value; return _s.str()

namespace ut
{
  typedef void (*TestFunc)();

  struct TestInfo { const char* File; const char* Name; TestFunc Func; };

  struct State
  {
    std::vector<TestInfo> Tests;    // All the TEST_METHOD
    std::vector<TestFunc> Inits;    // All the TEST_CLASS_INITIALIZE
    std::vector<TestFunc> Cleanups; // All the TEST_CLASS_CLEANUP
    const char* Current = "";       // Name of the running test
    unsigned Assertions = 0;        // Count of assertions of the running test
    unsigned Failures   = 0;        // Count of failed assertions of the running test
    unsigned Printed    = 0;        // Count of failure messages printed for the running test
    unsigned MaxPrinted = 3;        // Max failure messages printed per test (0 = all)
  };
  inline State& S() { static State s; return s; }

  inline bool reg(const char* file, const char* name, TestFunc func) { S().Tests.push_back({ file, name, func }); return true; }
  inline bool reg_init(TestFunc func)    { S().Inits.push_back(func);    return true; }
  inline bool reg_cleanup(TestFunc func) { S().Cleanups.push_back(func); return true; }

  template<class T> struct TestBase { typedef T TestClass; };

  inline std::string narrow(const wchar_t* w)
  {
    std::string out;
    if (w != nullptr) for (; *w != 0; ++w) out += ((unsigned)*w < 128u) ? (char)*w : '?';
    return out;
  }
  inline void failed(const std::string& what, const wchar_t* msg)
  {
    State& s = S();
    s.Failures++;
    if ((s.MaxPrinted == 0) || (s.Printed < s.MaxPrinted)) { s.Printed++; std::printf("    FAIL [%s] %s | %s\n", s.Current, what.c_str(), narrow(msg).c_str()); }
  }
}

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework
{
  struct __LineInfo;

  template<typename T> inline std::wstring ToString(const T& t)
  {
    std::wostringstream os;
    if constexpr (std::is_enum<T>::value)            os << static_cast<long long>(t);
    else if constexpr (std::is_arithmetic<T>::value) { if constexpr (sizeof(T) == 1) os << static_cast<int>(t); else os << t; }
    else os << L"<?>";
    return os.str();
  }

  class Assert
  {
  public:
    template<typename T> static void AreEqual(const T& expected, const T& actual, const wchar_t* message = nullptr, const __LineInfo* = nullptr)
    {
      ut::S().Assertions++;
      if (!(expected == actual))
      {
        std::wstring e = ToString(expected), a = ToString(actual);
        ut::failed("expected <" + ut::narrow(e.c_str()) + "> actual <" + ut::narrow(a.c_str()) + ">", message);
      }
    }
    // Microsoft's framework refuses two different types, here two different integer types are compared by value (the unit tests were written for 16-bit fields)
    template<typename T1, typename T2, typename = typename std::enable_if<!std::is_same<T1, T2>::value && (std::is_arithmetic<T1>::value || std::is_enum<T1>::value) && (std::is_arithmetic<T2>::value || std::is_enum<T2>::value)>::type>
    static void AreEqual(const T1& expected, const T2& actual, const wchar_t* message = nullptr, const __LineInfo* = nullptr)
    {
      ut::S().Assertions++;
      if (!(static_cast<long long>(expected) == static_cast<long long>(actual)))
      {
        std::wstring e = ToString(expected), a = ToString(actual);
        ut::failed("expected <" + ut::narrow(e.c_str()) + "> actual <" + ut::narrow(a.c_str()) + ">", message);
      }
    }
    template<typename T> static void AreNotEqual(const T& notExpected, const T& actual, const wchar_t* message = nullptr, const __LineInfo* = nullptr)
    {
      ut::S().Assertions++;
      if (notExpected == actual) ut::failed("values are equal", message);
    }
    static void IsTrue(bool condition, const wchar_t* message = nullptr, const __LineInfo* = nullptr)
    {
      ut::S().Assertions++;
      if (!condition) ut::failed("condition is false", message);
    }
    static void IsFalse(bool condition, const wchar_t* message = nullptr, const __LineInfo* = nullptr)
    {
      ut::S().Assertions++;
      if (condition) ut::failed("condition is true", message);
    }
    static void Fail(const wchar_t* message = nullptr, const __LineInfo* = nullptr)
    {
      ut::S().Assertions++;
      ut::failed("Assert::Fail", message);
    }
  };

  class Logger
  {
  public:
    static void WriteMessage(const wchar_t* message) { std::printf("    log: %s\n", ut::narrow(message).c_str()); }
    static void WriteMessage(const char* message)    { std::printf("    log: %s\n", message); }
  };
} } }

//-----------------------------------------------------------------------------
#define TEST_CLASS(cls)  class cls : public ::ut::TestBase<cls>

#define TEST_METHOD(name)                                                      \
  static void name##__call() { TestClass instance; instance.name(); }          \
  inline static const bool name##__reg = ::ut::reg(__FILE__, #name, &name##__call); \
  void name()

#define TEST_CLASS_INITIALIZE(name)                                            \
  static void name##__call() { name(); }                                       \
  inline static const bool name##__reg = ::ut::reg_init(&name##__call);        \
  static void name()

#define TEST_CLASS_CLEANUP(name)                                               \
  static void name##__call() { name(); }                                       \
  inline static const bool name##__reg = ::ut::reg_cleanup(&name##__call);     \
  static void name()
//-----------------------------------------------------------------------------
#endif /* CPPUNITTEST_SHIM_H_INC */
