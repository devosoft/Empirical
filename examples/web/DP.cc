//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <string>

#include "web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

void TextCB(const std::string & str) {
  auto result = doc.Slate("result");
  result.Clear();
  result << "<br>" << str << "<br>";
}

int main()
{
  doc << "<h2>Choose Your Words!</h2>";
  auto ta = doc.AddTextArea(TextCB, "text_area");
  ta.SetSize(400, 80);

  auto result = doc.AddSlate("result");
}
