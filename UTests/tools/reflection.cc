#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/reflection.h"

struct TestTrue {
  int test_member;
  int TestFun(int a, int b) { return a*b; }
};

struct TestFalse {
  int other_stuff;
};

struct TestTrueMethod {
  int test_member() { return 4; }
};

template <typename T>
int TestExternalFun(T & obj, int a, int b) { return a+b; }

EMP_CREATE_MEMBER_DETECTOR(test_member);
EMP_CREATE_METHOD_FALLBACK(DynamicFun, TestFun, TestExternalFun, int);

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  if (verbose) {
    std::cout << "TestTrue --> " << EMP_Detect_test_member<TestTrue>::value << std::endl;
    std::cout << "TestFalse --> " << EMP_Detect_test_member<TestFalse>::value << std::endl;
    std::cout << "TestTrueMethod --> " << EMP_Detect_test_member<TestTrueMethod>::value << std::endl;
  }

  emp_assert(EMP_Detect_test_member<TestTrue>::value == 1);
  emp_assert(EMP_Detect_test_member<TestFalse>::value == 0);
  emp_assert(EMP_Detect_test_member<TestTrueMethod>::value == 1);

  if (verbose) {
    std::cout << "EMP_Detect successful!" << std::endl;
  }

  TestTrue t;
  TestFalse f;
  emp_assert(DynamicFun(t, 20, 20) == 400);
  emp_assert(DynamicFun(f, 20, 20) == 40);

  if (verbose) {
    std::cout << "Dynamic function detection successful!" << std::endl;
  }
}
