#define EMSCRIPTEN 1
#include "../Empirical/emtools/init.h"
#include <emscripten.h>
#include <iostream>
#include <vector>
#include <string>
#include "selection.h"
#include "../Empirical/emtools/JSWrap.h"


int return_d(int d){
  return d;
}

int main()
{
  std::cout << "argcount" << EMP_GetCBArgCount() << std::endl;
  D3::Selection svg = D3::Selection("body").Append("svg");
  
  D3::Selection s = svg.SelectAll("circle");
 
  std::cout << n_selections() << std::endl;
  int32_t test_data[] = {1,2,3};
  D3::Selection update = s.Data(test_data, 3);
  //D3::Selection update = data_bind[0];
  //D3::Selection enter = data_bind[1];
  update.EnterAppend("circle");
  D3::Selection circles = D3::Selection("circle", true);
  circles.SetAttr("cx", 25);
  circles.SetAttr("cy", 25);
  circles.SetAttr("r", 25);
  circles.SetStyle("fill", "purple");

  //Now let's try making a callback function with JSWrap
  uint32_t fun_id = emp::JSWrap(return_d, "return_d");
  std::cout << "function id: " << fun_id << std::endl;

  //It works if you call empCppCallback from C++...
  EM_ASM({
      emp_i.cb_args.push(2);
    });
  empCppCallback(fun_id);
  EM_ASM({
      console.log(emp_i.cb_return);
    });

  //Calling the function directly from C++ works fine...
  std::cout << "calling from C++: " << return_d(4) << std::endl;

  //Calling the function directly from Javascript doesn't work
  //This breaks with: "Uncaught ReferenceError: empCppCallback is not defined"
  int jsresult = EM_ASM_INT({return emp.return_d($0);},5);
  std::cout << "calling from JS: " << jsresult << std::endl;

  //circles.SetAttrString("r", "emp.return_d");
  //EM_ASM({js.selections[4].enter().append("circle")});
  //D3::Selection enter_circles = enter.Append("circle");
  //enter_circles.SetStyleString("fill", "green");
  
}


//The only way to bind data appropriately?
//d3.selectAll("svg").selectAll("line").data([1,2,3]).enter().append("line")
