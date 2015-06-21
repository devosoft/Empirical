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

struct SerializeTest_D :public SerializeTest {
  char d = '$';

  SerializeTest_D(int _a, float _b, std::string _c, char _d)
    : SerializeTest(_a, _b, _c), d(_d) { ; }
  EMP_SETUP_SERIALIZE_D(SerializeTest_D, SerializeTest, d);
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::stringstream ss;
  emp::serialize::TextIO io(ss);
  SerializeTest st(7, 2.34, "my_test_string");
  st.EMP_Store(io);

  if (verbose) {
    std::cout << "Finished save on base class.\nSaved stream: " << ss.str() << std::endl;
  }

  SerializeTest st2(io);

  emp_assert(st2.a == 7);      // Make sure a was reloaded correctly.
  emp_assert(st2.c == "my_test_string");  // Make sure ss was fully emptied!

  if (verbose) {
    std::cout << "a reloaded as: " << st2.a << std::endl;
    std::cout << "c reloaded as: " << st2.c << std::endl;
    std::cout << "b (not) reloaded as: " << st2.b << std::endl;
    std::cout << "After reload, stream: " << ss.str() << std::endl;
  }

  SerializeTest_D stD(10,0.2,"three",'D');
  stD.EMP_Store(io);

  if (verbose) {
    std::cout << "Finished save on derived class.\nSaved stream: " << ss.str() << std::endl;
  }
}
