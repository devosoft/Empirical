/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2020
*/
/**
 *  @file
 */

#include <iostream>

#include "emp/prefab/CommentBox.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  emp::prefab::CommentBox box("comment_box");
  doc << box;
  box.AddContent("<h1>Comment Box Title</h1>");
  UI::Div body("body");
  box.AddContent(body);
  body << "More content for comment box";
}
