//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ConceptWrapper template

#include <iostream>

#include "../../source/meta/ConceptWrapper.h"

EMP_BUILD_CONCEPT( TestConcept,
                   REQUIRED_FUN(RequiredFun1, "Missing RequiredFun function 1!", void),
                   REQUIRED_FUN(RequiredFun2, "Missing RequiredFun function 2!", void)
                 );


struct MinimalClass {
  void RequiredFun1() { std::cout << "In MinimalClass::RequiredFun1()" << std::endl; }
  void RequiredFun2() { std::cout << "In MinimalClass::RequiredFun2()" << std::endl; }
};

int main() {
  TestConcept<MinimalClass> test_class;
  test_class.RequiredFun1();
  test_class.RequiredFun2();

  std::cout << "Done!" << std::endl;
}