#include <iostream>
#include <string>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/mem_track.h"

struct TestClass1 {
  TestClass1() {
    EMP_TRACK_CONSTRUCT(TestClass1);
  }
  ~TestClass1() {
    EMP_TRACK_DESTRUCT(TestClass1);
  }
};

struct TestClass2 {
  TestClass2() {
    EMP_TRACK_CONSTRUCT(TestClass2);
  }
  ~TestClass2() {
    EMP_TRACK_DESTRUCT(TestClass2);
  }
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::vector<TestClass1 *> test_v;
  TestClass2 class2_mem;

  if (verbose) {
    std::cout << "(before construct) TestClass1 count = "
              << EMP_TRACK_COUNT(TestClass1) << std::endl;
  }


  for (int i = 0; i < 1000; i++) {
    test_v.push_back( new TestClass1 );
  }

  if (verbose) {
    std::cout << "(after construct) TestClass1 count = "
              << EMP_TRACK_COUNT(TestClass1) << std::endl;
  }


  for (int i = 500; i < 1000; i++) {
    delete test_v[i];
  }

  if (verbose) {
    std::cout << "(after delete) TestClass1 count = "
              << EMP_TRACK_COUNT(TestClass1) << std::endl;
    std::cout << EMP_TRACK_STATUS << std::endl;
  }
}
