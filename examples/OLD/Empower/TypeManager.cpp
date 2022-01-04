/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file TypeManager.cpp
 *  @brief Some examples code for using emp::TypeManager
 */

#include <iostream>

#include "../../include/emp/in_progress/Empower/Type.h"
#include "../../include/emp/in_progress/Empower/TypeManager.h"

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
