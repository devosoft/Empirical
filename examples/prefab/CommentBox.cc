//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "web/Div.h"
#include "web/Element.h"
#include "prefab/CommentBox.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
    emp::prefab::CommentBox box;
    UI::Div title("desktop_content");
    title << "<h1>Content that shows on all screen sizes</h1>";
    box.AddContent(title);
    UI::Element mobile("span");
    mobile << "<p>Content that only shows on small screens <br>Web conponents can be added as content</p>";
    mobile << "<hr>";
    box.AddMobileContent(mobile);
    box.AddMobileContent("<b>String literals can also be added to box content!</b>");
    doc << box;
}
