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
  EMP_SETUP_DATAPOD(SerializeTest, a, c);
};

struct SerializeTest_D : public SerializeTest {
  char d = '$';

  SerializeTest_D(int _a, float _b, std::string _c, char _d)
    : SerializeTest(_a, _b, _c), d(_d) { ; }
  EMP_SETUP_DATAPOD_D(SerializeTest_D, SerializeTest, d);
};

struct ExtraBase {
  double e;

  ExtraBase(double _e) : e(_e) { ; }
  EMP_SETUP_DATAPOD(ExtraBase, e);
};

struct MultiTest : public SerializeTest, public ExtraBase {
  bool f;

  MultiTest(int _a, float _b, std::string _c, double _e, bool _f)
    : SerializeTest(_a, _b, _c), ExtraBase(_e), f(_f) { ; }
  EMP_SETUP_DATAPOD_D2(MultiTest, SerializeTest, ExtraBase, f);
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::stringstream ss;
  emp::serialize::DataPod pod(ss);
  SerializeTest st(7, 2.34, "my_test_string");
  st.EMP_Store(pod);

  if (verbose) {
    std::cout << "Finished storing to DataPod for base class.\n"
              << "Saved stream: " << ss.str() << std::endl;
  }

  SerializeTest st2(pod);

  if (verbose) {
    std::cout << "Reloaded DataPod for base class.  Results:\n  st2.a = " << st2.a
              << "\n  st2.b = " << st2.b << " (uninitialized)\n  st2.c = " << st2.c
              << std::endl;
  }

  emp_assert(st2.a == 7);                 // Make sure a was reloaded correctly.
  emp_assert(st2.c == "my_test_string");  // Make sure c was reloaded correctly.

  if (verbose) {
    std::cout << "After reload, stream: " << ss.str() << std::endl;
  }

  SerializeTest_D stD(10,0.2,"three",'D');
  stD.EMP_Store(pod);

  if (verbose) {
    std::cout << "Finished store on derived class.\nSaved stream: " << ss.str() << std::endl;
  }

  SerializeTest_D stD2(pod);

  if (verbose) {
    std::cout << "Reloaded DataPod for derived class.  Results:\n"
              << "  stD2.a = " << stD2.a << "\n"
              << "  stD2.b = " << stD2.b << " (uninitialized)\n"
              << "  stD2.c = " << stD2.c << "\n"
              << "  stD2.d = " << stD2.d
              << std::endl;
  }

  emp_assert(stD2.a == 10);
  emp_assert(stD2.c == "three");
  emp_assert(stD2.d == 'D');

  if (verbose) {
    std::cout << "Reload successful!" << std::endl;
  }

  MultiTest stM(111,2.22,"ttt",4.5,true);
  stM.EMP_Store(pod);

  if (verbose) {
    std::cout << "Finished save on mulit-derived class.\nSaved stream: " << ss.str() << std::endl;
  }

  MultiTest stM2(pod);

  if (verbose) {
    std::cout << "Reloaded DataPod for multi-derived class.  Results:\n"
              << "  stM2.a = " << stM2.a << "\n"
              << "  stM2.b = " << stM2.b << " (uninitialized)\n"
              << "  stM2.c = " << stM2.c << "\n"
              << "  stM2.e = " << stM2.e << "\n"
              << "  stM2.f = " << stM2.f
              << std::endl;
  }

  emp_assert(stM2.a == 111);
  emp_assert(stM2.c == "ttt");
  emp_assert(stM2.e == 4.5);
  emp_assert(stM2.f == true);


  if (verbose) std::cout << "All reloads successful!!!" << std::endl;

  // int test_int;
  // emp::serialize::SetupLoad<int>(pod, &test_int, true);
  // emp::serialize::SetupLoad(pod, &stM2, true);
}
