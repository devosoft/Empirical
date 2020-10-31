//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << "<h1>Font Examples...</h1>";

  doc << UI::Font(100, "green").SetUnderlined().SetOverlined().SetWavyLine().SetLineColor("red")
      << "Testing!<br>"
      << UI::Font(15, "black")
      << "<br>Back to normal?"
      << UI::Font().SetSmallcaps()
      << "<br>And now THIS should be SmallCaps....";
}
