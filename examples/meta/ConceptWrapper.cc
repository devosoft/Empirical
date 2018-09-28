//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ConceptWrapper template

#include <iostream>

#include "../../source/meta/ConceptWrapper.h"

EMP_BUILD_CONCEPT( TestConcept,
                   PUBLIC( int x=5; ),
                   REQUIRED_FUN(RequiredFun1, "Missing RequiredFun function 1!", void),
                   REQUIRED_FUN(RequiredFun2, "Missing RequiredFun function 2!", void),
                   OPTIONAL_FUN(OptionalFun1, { std::cout << "Default version of OptionalFun1()" << std::endl; x++; }, void)
                 );


struct MinimalClass {
  void RequiredFun1() { std::cout << "In MinimalClass::RequiredFun1()" << std::endl; }
  void RequiredFun2() { std::cout << "In MinimalClass::RequiredFun2()" << std::endl; }
};

struct FullClass {
  void RequiredFun1() { std::cout << "In FullClass::RequiredFun1()" << std::endl; }
  void RequiredFun2() { std::cout << "In FullClass::RequiredFun2()" << std::endl; }
  void OptionalFun1() { std::cout << "In FullClass::OptionalFun2()" << std::endl; }
};

int main() {
  TestConcept<MinimalClass> min_class;
  min_class.RequiredFun1();
  min_class.RequiredFun2();
  min_class.OptionalFun1();

  TestConcept<FullClass> full_class;
  full_class.RequiredFun1();
  full_class.RequiredFun2();
  full_class.OptionalFun1();

  std::cout << "min_class.x = " << min_class.x << std::endl;
  std::cout << "full_class.x = " << full_class.x << std::endl;
  std::cout << "Done!" << std::endl;
}