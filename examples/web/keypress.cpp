//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/KeypressManager.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;
UI::Document doc("emp_base");
UI::KeypressManager keypress_manager;
std::string other_str;

int x = 0;

bool OtherKey(const UI::KeyboardEvent & evt)
{
  x=0;
  other_str = "Why would you press ";
  if (evt.altKey) other_str += "[ALT]";
  if (evt.ctrlKey) other_str += "[CTRL]";
  if (evt.metaKey) other_str += "[META]";
  if (evt.shiftKey) other_str += "[SHIFT]";
  other_str += (char) evt.keyCode;
  other_str += "???";
  doc.Redraw();

  return true;
}

int main()
{
  doc << "Press X or Z!  " << UI::Live(x)
      << "<br>" << UI::Live(other_str);

  keypress_manager.AddKeydownCallback('X', [](){ x=100; other_str=""; doc.Redraw(); });
  keypress_manager.AddKeydownCallback('Z', [](){ x=5; other_str=""; doc.Redraw(); });
  keypress_manager.AddKeydownCallback(&OtherKey);
}
