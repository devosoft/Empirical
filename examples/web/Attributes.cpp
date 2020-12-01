//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << UI::Text("test1") << "Testing 1.<br>";
  doc << UI::Text("test2").SetCSS("color", "green") << "Testing 2.<br>";
  doc << UI::Text("test3") << "Testing 3.<br>";
  doc << UI::Text("test4").SetAttr("class", "make_red") << "Testing 4.<br>";
  doc << UI::Text("test5") << "Testing 5.<br>";
  doc << UI::Text("test6") << "Click Me.<br>";
  doc << UI::Text("test7") << "Want to know the time?<br>";

  // CSS settings can also be changed after being added to a doc.
  doc.Text("test3").SetCSS("color", "blue");
  doc.Text("test5").SetAttr("class", "make_red make_large");
  doc.Text("test6").SetAttr("onclick", "this.innerHTML='You Clicked!<br>'");
  doc.Text("test7").SetAttr("onclick", "this.innerHTML=Date()");


  doc.Text("test2").On("click", [](UI::MouseEvent evt){ doc.Text("test2")
    .SetCSS("color", "red") << "You Clicked! " << evt.shiftKey << "<br>"; });


  doc.Text("test4").OnCopy( [](){ doc.Text("test1") << "COPY"; } );

  UI::Listeners listeners;
  listeners.Set("click", std::function<void()>([](){ doc.Text("test2").SetAttr("color", "red"); }));
  listeners.Apply("test2");
}
