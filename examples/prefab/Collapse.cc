//  This file is part of Config Panel App
//  Copyright (C) Matthew Andres Moreno, 2020.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "web/web.h"
#include "web/Div.h"

#include "prefab/Collapse.h"
#include "prefab/CommentBox.h"


emp::web::Document doc("emp_base");

int main()
{
  emp::prefab::CommentBox box1;
  box1.AddContent("<h3>Box 1</h3>");
  emp::web::Div btn1;
  btn1.SetAttr("class", "btn btn-info");
  btn1 << "Button 1: controls box 1";

  emp::prefab::CommentBox box2;
  box2.AddContent("<h3>Box 2</h3>");
  emp::web::Div btn2;
  btn2.SetAttr("class", "btn btn-info");
  btn2 << "Button 2: controls box 2";

  emp::prefab::CollapseCoupling collapse1(btn1, box1, true);
  emp::prefab::CollapseCoupling collapse2(btn2, box2, true);

  emp::web::Div btn3;
  btn3.SetAttr("class", "btn btn-success");
  btn3 << "Button 3: controls all boxes";

  collapse1.AddController(btn3, true);
  collapse2.AddController(collapse1.GetControllerDiv(1), true);

  doc << collapse1.GetControllerDiv(0);
  doc << collapse1.GetTargetDiv(0);
  doc << collapse2.GetControllerDiv(0);
  doc << collapse2.GetTargetDiv(0);
  doc << collapse1.GetControllerDiv(1);

  std::cout << "end of main... !" << std::endl;
}
