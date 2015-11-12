// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include "../../web/keypress.h"
#include "../../web/web.h"

namespace web = emp::web;
web::Document doc("emp_base");
web::KeypressManager keypress_manager;
std::string other_str;

int x = 0;

bool OtherKey(const emp::html5::KeyboardEvent & evt)
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
  doc << "Press X or Z!  " << web::Live(x)
      << "<br>" << web::Live(other_str);

  keypress_manager.AddKeydownCallback('X', [](){ x=100; other_str=""; doc.Redraw(); });
  keypress_manager.AddKeydownCallback('Z', [](){ x=5; other_str=""; doc.Redraw(); });
  keypress_manager.AddKeydownCallback(&OtherKey);
}
