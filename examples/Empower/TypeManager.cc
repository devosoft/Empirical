//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::TypeManage

#include <iostream>

#include "../../source/Empower/Type.h"
#include "../../source/Empower/TypeManager.h"

void PrintType(const emp::Type & type) {
  std::cout << "type '" << type.GetName()
	    << "' had id " << type.GetID()
	    << " and size " << type.GetSize()
	    << std::endl;  
}

int main()
{
  emp::TypeManager type_manager;
  emp::vector<const emp::Type *> type_v;

  type_v.push_back(&(type_manager.GetType<int>()));
  type_v.push_back(&(type_manager.GetType<double>()));
  type_v.push_back(&(type_manager.GetType<std::string>()));
  type_v.push_back(&(type_manager.GetType<bool>()));
  for (const auto * type_ptr : type_v) {
    PrintType(*type_ptr);
  }
  
  
}
