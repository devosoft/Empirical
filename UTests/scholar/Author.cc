//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../config/command_line.h"
#include "../../tools/assert.h"
#include "../../scholar/Author.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_flag(args, "-v");

  emp::Author fml("First", "Middle", "Last");
  emp_assert(fml.GetFullName() == "First Middle Last");

  emp::Author fl("No", "Mid");
  emp_assert(fl.GetFullName() == "No Mid");

  emp::Author l("JustLast");
  emp_assert(l.GetFullName() == "JustLast");

  emp::Author fml2("First", "Middiff", "Last");
  emp_assert(fml2.GetFullName() == "First Middiff Last");
  emp_assert(fml2 < fml);

  emp::Author auth1("aaa", "bbb", "ccc");
  emp::Author auth2("aaa", "bbb", "ccd");
  emp::Author auth3("aaa", "bbd", "ccc");
  emp::Author auth4("aad", "bbb", "ccc");
  emp::Author auth5("aaa", "bbb", "ccc");

  emp_assert(auth1 < auth2);
  emp_assert(auth1 < auth3);
  emp_assert(auth1 < auth4);
  emp_assert(auth1 == auth5);

  emp_assert(auth2 > auth3);
  emp_assert(auth2 > auth4);
  emp_assert(auth2 > auth5);

  emp_assert(auth3 < auth4);
  emp_assert(auth3 > auth5);

  emp_assert(auth4 > auth5);

  // Try out <= operator
  emp_assert(auth1 <= auth2);
  emp_assert(auth1 <= auth3);
  emp_assert(auth1 <= auth4);
  emp_assert(auth1 <= auth5);
}
