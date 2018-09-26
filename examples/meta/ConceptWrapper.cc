//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ConceptWrapper template

#include <iostream>

#include "../../source/meta/ConceptWrapper.h"

EMP_BUILD_CONCEPT( TestConcept,
                   REQUIRED_FUN(RequiredFun, "Missing RequiredFun function!", void)
                 );


struct MinimalClass {
  void RequiredFun() { std::cout << "In MinimalClass::RequiredFun()" << std::endl; }
};

int main() {
  TestConcept<MinimalClass> test_class;
  test_class.RequiredFun();

  std::cout << "Done!" << std::endl;
}