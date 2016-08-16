//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE

#include "../../web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << UI::Text("test1") << "Testing 1.<br>";
  doc << UI::Text("test2").SetCSS("color", "green") << "Testing 2.<br>";
  doc << UI::Text("test3") << "Testing 3.<br>";
  doc << UI::Text("test4").SetAttr("class", "make_red") << "Testing 4.<br>";
  doc << UI::Text("test5") << "Testing 5.<br>";

  // CSS settings can also be changed after being added to a doc.
  doc.Text("test3").SetCSS("color", "blue");
  doc.Text("test5").SetAttr("class", "make_red make_large");
}
