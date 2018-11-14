//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Empower

#include <iostream>

#include "../../source/Empower/Type.h"

int main()
{
  emp::Type * type1 = new emp::TypeInfo<int>;
  std::cout << "type1 is '" << type1->GetName()
	    << "' with id " << type1->GetID()
	    << " and size " << type1->GetSize()
	    << std::endl;
}
