#include <iostream>
#include <string>
#include <sstream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/serialize.h"

struct SerializeTest {
  int a;
  float b;
  std::string c;
  
  SerializeTest(int _a, float _b, std::string _c) : a(_a), b(_b), c(_c) { ; }
  EMP_SETUP_SERIALIZE(SerializeTest, a,c);
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::stringstream ss;
  SerializeTest st(7, 2.34, "my test string");
  st.EMP_StoreText(ss);

  if (verbose) {
    std::cout << ss.str() << std::endl;
  }

  SerializeTest st2(ss);

  emp_assert(st2.a == 7);

  if (verbose) {
    std::cout << st2.a << std::endl;
  }
}
