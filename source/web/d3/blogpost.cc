#include <iostream>
#include <vector>
#include <string>

#include "web/init.h"
#include "web/Document.h"
#include "../js_utils.h"
#include "../../base/map.h"

#include "d3_init.h"
#include "selection.h"
#include "transition.h"
#include "scales.h"
#include "utils.h"

namespace UI = emp::web;
UI::Document doc("emp_d3_test");
D3::Selection circle_selection;
D3::Selection svg_selection;


void CreateDivScratch() {

  // circle_selection = D3::Select("#emp_d3_test");
  //.Append("svg");

  // test_select.SetAttr("width", 400).SetAttr("height", 400);
  std::cout << circle_selection.GetID() << std::endl;
  std::cout << svg_selection.GetID() << std::endl;
  EM_ASM({
    $("#emp_d3_test").append("<svg id='test_svg'><circle/><circle/></svg>");
  });
  // svg_selection = D3::Select("#test_svg");
  // circle_selection = svg_selection.SelectAll("circle");

  // circle_selection.SetAttr("r", 5);


  D3::LinearScale test_lin;
  test_lin.SetDomain(0, 1000);
  test_lin.SetRange(0, 500);
  std::cout << test_lin.ApplyScale<int, int>(500) << std::endl;
}

int main() {
  D3::internal::get_emp_d3();

  CreateDivScratch();

  return 0;
}