#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/tuple_struct.h"

struct TestStruct {
  int myvar;

  EMP_BUILD_TUPLE( int, MyInt,
                   int, MyInt2,
                   std::string, MyString )
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::cout << "Hello!" << std::endl;

  TestStruct test;
  test.myvar = 12;
  test.MyInt(15);
  test.MyInt() = 20;

  std::cout << "myvar = " << test.myvar << std::endl;
  std::cout << "MyInt = " << test.MyInt() << std::endl;
}
