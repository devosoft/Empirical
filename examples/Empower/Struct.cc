//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Struct

#include <iostream>

#include "../../source/Struct.h"

int main()
{
  emp::TypeManager type_manager;
  emp::StructType address_struct;
  address_struct.AddVar<size_t>("House Number");
  address_struct.AddVar<std::string>("Street");
  address_struct.AddVar<std::string>("City");
  address_struct.AddVar<std::string>("State");
  address_struct.AddVar<size_t>("Zip Code");
  address_struct.AddVar<std::string>("Country");

  emp::Struct home(address_struct);
  emp::Struct work(address_struct);

  home["House Number"] = 123;
  home["Street"] = "Alphabet St.";
  home["City"] = "Springfield";
  home["State"] = "OZ";
  home["Zip Code"] = 0;
  home["Country"] = "Anywhere!";
}
