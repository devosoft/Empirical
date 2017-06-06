//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::Ptr

#include <iostream>
#include <string>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "../../base/Ptr.h"

struct TestBase {
  int x;
  TestBase(int _x) : x(_x) { ; }
  virtual ~TestBase() { ; }
  virtual int Val() { return x; }
};
struct TestDerived : public TestBase {
  TestDerived(int _x) : TestBase(_x) { ; }
  int Val(){return 2*x;}
};

int main()
{
  emp::Ptr<std::string>::SetVerbose();

  std::string test_str = "Test String!";
  std::cout << test_str << std::endl;

  emp::Ptr<std::string> test_ptr(&test_str);   // false -> don't track!
  std::cout << "From Ptr: " << *test_ptr << std::endl;
  std::cout << "  with size = " << test_ptr->size() << std::endl;

  {
    emp::Ptr<std::string> test_ptr2(&test_str);
  }

  emp::Ptr<int> int_ptr;
  int_ptr.New(123456);
  std::cout << "*int_ptr = " << *int_ptr << std::endl;
  int_ptr.Delete();
  std::cout << "Deleted int_ptr." << std::endl;


  // Examples with base and derived classes.
  emp::Ptr<TestBase> ptr_base = new TestDerived(5);
  emp::Ptr<TestDerived> ptr_derived(ptr_base.Cast<TestDerived>());

  std::cout << "End of main()." << std::endl;
}
