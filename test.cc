#include <emscripten.h>
#include <iostream>
#include <vector>
#include <string>
#include "selection.h"

int main()
{
  D3::Selection svg = D3::Selection("body").Append("svg");
  
  D3::Selection s = svg.SelectAll("circle");
 
  std::cout << n_selections() << std::endl;
  int32_t test_data[] = {1,2,3};
  D3::Selection update = s.Data(test_data, 3);
  //D3::Selection update = data_bind[0];
  //D3::Selection enter = data_bind[1];
  update.EnterAppend("circle");
  D3::Selection circles = D3::Selection("circle", true);
  circles.SetAttrNumeric("cx", 25);
  circles.SetAttrNumeric("cy", 25);
  circles.SetAttrNumeric("r", 25);
  circles.SetStyleString("fill", "purple");
  //EM_ASM({js.selections[4].enter().append("circle")});
  //D3::Selection enter_circles = enter.Append("circle");
  //enter_circles.SetStyleString("fill", "green");
  
}


//The only way to bind data appropriately?
//d3.selectAll("svg").selectAll("line").data([1,2,3]).enter().append("line")
