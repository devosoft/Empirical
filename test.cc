#include "../Empirical/emtools/init.h"
#include <emscripten.h>
#include <iostream>
#include <vector>
#include <string>
#include "selection.h"
#include "../Empirical/emtools/JSWrap.h"


int return_d(int d, int i=0, int k=0){
  return d;
}

void mouseover(int id){
  D3::Selection(id).SetAttr("cx", 500);
}

int main()
{
  emp::Initialize();

  D3::Selection svg = D3::Selection("body").Append("svg");
  
  D3::Selection s = svg.SelectAll("circle");
 
  std::cout << n_selections() << std::endl;
  int32_t test_data[] = {10,30,60};
  D3::Selection update = s.Data(test_data, 3);
  //D3::Selection update = data_bind[0];
  //D3::Selection enter = data_bind[1];
  update.EnterAppend("circle");
  D3::Selection circles = D3::Selection("circle", true);
  circles.SetAttr("cx", 25);
  circles.SetAttr("cy", 25);
  circles.SetAttr("r", 25);
  circles.SetStyle("fill", "purple");
  D3::Selection t = circles.Transition();
  //t.Interrupt();
  t.SetAttr("r", 5);

  //Now let's try making a callback function with JSWrap
  uint32_t fun_id = emp::JSWrap(return_d, "return_d");
  emp::JSWrap(mouseover, "mouseover");
  //int jsresult = EM_ASM_INT({return emp.return_d($0);},5);

  t.SetAttr("r", "return_d");
  t.SetAttr("cy", "return_d");
  t.SetAttr("cx", "return_d");
  t.SetStyle("fill", "green");
  D3::Selection("circle", true).On("mouseover", "mouseover");
  
}


//The only way to bind data appropriately?
//d3.selectAll("svg").selectAll("line").data([1,2,3]).enter().append("line")
