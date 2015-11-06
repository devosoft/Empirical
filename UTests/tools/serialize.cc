// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <string>
#include <sstream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/serialize.h"

struct SerializeTest {
  int a;
  float b;        // unimportant data!
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

struct NestedTest {
  SerializeTest st;
  std::string name;
  SerializeTest_D std;
  MultiTest mt;

  NestedTest(int a1, float b1, std::string c1,
             int a2, float b2, std::string c2, char d2,
             int a3, float b3, std::string c3, double e3, bool f3)
    : st(a1, b1, c1), name("my_class"), std(a2, b2, c2, d2), mt(a3, b3, c3, e3, f3) { ; }

  EMP_SETUP_DATAPOD(NestedTest, st, name, std, mt);
};

struct BuiltInTypesTest {
  const int a;
  std::vector<int> int_v;

  BuiltInTypesTest(int _a, int v_size) : a(_a), int_v(v_size) {
    for (int i = 0; i < v_size; i++) int_v[i] = i*i;
  }

  EMP_SETUP_DATAPOD(BuiltInTypesTest, a, int_v);
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::stringstream ss;
  emp::serialize::DataPod pod(ss);


  // Basic test...

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


  // Derived class Test

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



  // Multiply-derived class Test

  MultiTest stM(111,2.22,"ttt",4.5,true);
  stM.EMP_Store(pod);

  if (verbose) {
    std::cout << "Finished save on multi-derived class.\nSaved stream: " << ss.str() << std::endl;
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


  // Nested objects test...

  NestedTest nt(91, 3.14, "magic numbers",
                100, 0.01, "powers of 10", '1',
                1001, 1.001, "ones and zeros", 0.125, true);
  nt.EMP_Store(pod);

  if (verbose) {
    std::cout << "Finished save on nested class.\nSaved stream: " << ss.str() << std::endl;
  }

  NestedTest nt2(pod);

  if (verbose) {
    std::cout << "Reloaded DataPod for multi-derived class.  Results:\n"
              << "  nt2.st.a = " << nt2.st.a << "\n"
              << "  nt2.st.c = " << nt2.st.c << "\n"
              << "  nt2.name = " << nt2.name << "\n"
              << "  nt2.std.a = " << nt2.std.a << "\n"
              << "  nt2.std.c = " << nt2.std.c << "\n"
              << "  nt2.std.d = " << nt2.std.d << "\n"
              << "  nt2.mt.a = " << nt2.mt.a << "\n"
              << "  nt2.mt.c = " << nt2.mt.c << "\n"
              << "  nt2.mt.e = " << nt2.mt.e << "\n"
              << "  nt2.mt.f = " << nt2.mt.f << "\n"
              << std::endl;
  }

  emp_assert(nt2.st.a == 91);
  emp_assert(nt2.st.c == "magic numbers");
  emp_assert(nt2.name == "my_class");
  emp_assert(nt2.std.a == 100);
  emp_assert(nt2.std.c == "powers of 10");
  emp_assert(nt2.std.d == '1');
  emp_assert(nt2.mt.a == 1001);
  emp_assert(nt2.mt.c == "ones and zeros");
  emp_assert(nt2.mt.e == 0.125);
  emp_assert(nt2.mt.f == true);


  // If we made it this far, everything must have worked!

  if (verbose) std::cout << "All reloads successful!!!" << std::endl;


  const int v_size = 43;
  BuiltInTypesTest bitt(91, v_size);
  bitt.EMP_Store(pod);

  if (verbose) {
    std::cout << "Finishd save on built-in library types.\nSaved stream: " << ss.str() << std::endl;
  }

  BuiltInTypesTest bitt2(pod);
  if (verbose) {
    std::cout << "Reloaded DataPod for built-in library types class.\nResults:\n"
              << "  vector size = " << bitt2.int_v.size() << "\n";
    for (int i = 0; i < v_size; i++) {
      std::cout << "  int_v[" << i << "] = " << bitt2.int_v[i] << "\n";
    }
  }
}
