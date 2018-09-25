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

int main() {
  std::cout << "Done!" << std::endl;
}