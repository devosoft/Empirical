//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Empower

#include <iostream>

#include "../../source/Empower/Type.h"

void PrintType(emp::Type & type) {
  std::cout << "type '" << type.GetName()
	    << "' had id " << type.GetID()
	    << " and size " << type.GetSize()
	    << std::endl;  
}

int main()
{
  emp::vector<emp::Type *> type_v;
  type_v.push_back(new emp::TypeInfo<int>);
  type_v.push_back(new emp::TypeInfo<double>);
  type_v.push_back(new emp::TypeInfo<std::string>);
  type_v.push_back(new emp::TypeInfo<bool>);
  for (auto type_ptr : type_v) {
    PrintType(*type_ptr);
  }
  
  
}
