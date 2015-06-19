#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/serialize.h"

struct SerializeTest {
  int a;
  float b;
  std::string c;

  EMP_SETUP_SERIALIZE(a,c);
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  SerializeTest st = { 7, 2.34, "my test string" };
  st.EMP_StoreText(std::cout);

  std::cout << std::endl << "Hello!" << std::endl;
}

// class apple {
// public:
//   int a;
//   bool isTasty;
//   float unimportantData;
  
//   SET_SAVED_MEMBERS(a, isTasty);
// };


// int main() {
//   apple a = {7, false, 2.34};
//   a.isTasty=true;
//   a.serialize("Test: ");

// }

