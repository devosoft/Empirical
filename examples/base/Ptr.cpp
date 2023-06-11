/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file Ptr.cpp
 *  @brief An example file for using emp::Ptr.
 */

#include <iostream>
#include <string>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

// Example class that can be pointed to and derived from.
struct TestBase {
  int x;
  TestBase(int _x) : x(_x) { ; }
  virtual ~TestBase() { ; }
  virtual int Val() { return x; }
};

// Example class that is derived from the TestBase class; used to make sure virtual
// function calls work correctly.
struct TestDerived : public TestBase {
  TestDerived(int _x) : TestBase(_x) { ; }
  int Val(){return 2*x;}
};

int main()
{
  std::string test_str = "Test String!";
  std::cout << test_str << std::endl;

  // Passing an existing pointer into test_ptr; should not be tracked.
  emp::Ptr<std::string> test_ptr(&test_str);
  std::cout << "From Ptr: " << *test_ptr << std::endl;
  std::cout << "  with size = " << test_ptr->size() << std::endl;

  {
    emp::Ptr<std::string> test_ptr2(&test_str);
  }

  // Creating new int Ptr, which should be tracked.
  emp::Ptr<int> int_ptr;
  int_ptr.New(123456);
  std::cout << "*int_ptr = " << *int_ptr << std::endl;
  int_ptr.Delete();
  std::cout << "Deleted int_ptr." << std::endl;


  // Examples with base and derived classes.
  emp::Ptr<TestBase> ptr_base = new TestDerived(5);
  emp::Ptr<TestDerived> ptr_derived(ptr_base.Cast<TestDerived>());



  // Examples with arrays!
  int_ptr.NewArray(20);                    // Reuse int_ptr, this time as an array.
  for (size_t i = 0; i < 20; i++) {
    int_ptr[i] = (int) (i+100);
  }
  std::cout << "Array contents:";
  for (size_t i = 0; i < 20; i++) {
    std::cout << " " << int_ptr[i];
  }
  std::cout << std::endl;
  int_ptr.DeleteArray();


  // Creat a vector of pointers; set just the first 26, and delete them.
  emp::vector< emp::Ptr<char> > ptr_v(26);
  for (size_t i = 0; i < 26; i++) {
    ptr_v[i] = emp::NewPtr<char>((char)('A' + (char) i));
  }
  ptr_v.resize(100);
  for (size_t i = 0; i < 26; i++) {
    ptr_v[i].Delete();
  }


  // -- Interactions between Ptr and emp::vector --

  // create a vector of pointers
  emp::vector<emp::Ptr<char>> v_ptr(26);
  for (size_t i = 0; i < v_ptr.size(); i++) {
    v_ptr[i] = emp::NewPtr<char>((char)('A' + i));
  }

  // print the pointer contents.
  std::cout << "Chars: ";
  for (size_t i = 0; i < v_ptr.size(); i++) {
    std::cout << *v_ptr[i];
  }
  std::cout << std::endl;

  // Juggle the pointers with another vector.
  emp::vector<emp::Ptr<char>> v_ptr2;
  std::swap(v_ptr, v_ptr2);

  // and delete the pointers.
  for (size_t i = 0; i < v_ptr2.size(); i++) {
    v_ptr2[i].Delete();
  }

  // Signal that we are done.
  std::cout << "End of main()." << std::endl;
}
