/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2017
*/
/**
 *  @file
 */

#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << "<h1>Font Examples...</h1>";

  doc << UI::Font(100, emp::Palette::GREEN).SetUnderlined().SetOverlined().SetWavyLine().SetLineColor(emp::Palette::RED)
      << "Testing!<br>"
      << UI::Font(15, emp::Palette::BLACK)
      << "<br>Back to normal?"
      << UI::Font().SetSmallcaps()
      << "<br>And now THIS should be SmallCaps....";
}
