#include "../Empirical/emtools/init.h"
#include <emscripten.h>
#include <iostream>
#include <vector>
#include <string>
#include "selection.h"
#include "../Empirical/emtools/JSWrap.h"
#include "../Empirical/tools/tuple_struct.h"

struct JSDataObject;/*{
  int myvar;
  EMP_BUILD_TUPLE( int, val,
		   std::string, word)
		   };*/


/*class JSObject{

public:
  JSObject();
};

JSObject::JSObject(){

  

  //This is probably not possible

  int buffer = EM_ASM_INT_V({
      var d = ({val: 4, word: "hi"});
      var buffer = Module._malloc(JSON.stringify(d).length+1);
      Module.writeStringToMemory(JSON.stringify(d), buffer);
      return buffer;
    });
   char* string_object = (char*)(buffer);
   std::map<std::string, std::string> variables;
   //std::cout << "string object " << string_object << std::endl;
   char* token = std::strtok(string_object, ",");
   std::cout << "first token " << token << std::endl;
   while (token != NULL){
     std::string string_token = std::string(token);
     size_t location = string_token.find(":");
     std::cout << "location " << location << std::endl;
     if (location != std::string::npos){

       std::string first = string_token.substr(0, location);
       if (strncmp(&first.front(), "{", 1) == 0){
	 first = first.substr(1);
       }
       std::string second = string_token.substr(location+1);
       if (strncmp(&second.back(), "}", 1) == 0){
	 second = second.substr(0, second.length()-1);
       }
       std::cout << "first " << first << std::endl;
       std::cout << "second " << second << std::endl;
       variables[first] = second;
     }
     token = std::strtok(NULL, ",");
   }
   
   auto it = variables.begin();

   struct ObjectStruct {
     EMP_BUILD_TUPLE(std::string, it->first 
     );
   };
   ObjectStruct test;
   test.
  
}*/

int return_val(JSDataObject d, int i=0, int k=0){
  //std::cout << d.word() << std::endl;
  return d.val();
}

int return_d(int d, int i=0, int k=0){
  return d;
}

void mouseover(int id){
  D3::Selection(id).SetAttr("cx", 500);
}



int main()
{
  emp::Initialize();

  //JSObject();
  
  D3::Selection svg = D3::Selection("body").Append("svg");
  D3::Selection text = D3::Selection("body").Append("text");
  text.SetText("Testing");
  std::cout << text.GetText() << std::endl;

  //D3::Selection s = svg.SelectAll("circle");
 
  std::cout << n_selections() << std::endl;
  std::array<int32_t, 3> test_data = {10,30,60};
  JSDataObject test_obj_1;
  test_obj_1.val() = 10;
  test_obj_1.word() = "hi";
  test_obj_1.val2() = 4.4;

  JSDataObject test_obj_2;
  test_obj_2.val() = 40;
  test_obj_2.word() = "hi2";
  test_obj_2.val2() = 11.2;

  std::array<JSDataObject, 2> test_data_2 = {test_obj_1, test_obj_2};

  //D3::ShapesFromData(test_data, "circle");
  //EM_ASM({d3.select("svg").selectAll("circle").data([{val:5, word:"hi", val2:6.3}]).enter().append("circle")});

  svg.SelectAll("circle").Data(test_data_2).EnterAppend("circle");

  std::cout << "data bound" << std::endl;

  //D3::Selection update = s.Data(test_data, 3);
  //D3::Selection update = data_bind[0];
  //D3::Selection enter = data_bind[1];
  //update.EnterAppend("circle");
  D3::Selection circles = D3::Selection("circle", true);
  circles.SetAttr("cx", 25);
  circles.SetAttr("cy", 25);
  circles.SetAttr("r", 25);
  circles.SetStyle("fill", "purple");
  D3::Selection t = circles.Transition();
  //t.Interrupt();
  //t.SetAttr("r", 5);

  //Now let's try making a callback function with JSWrap
  //uint32_t fun_id = emp::JSWrap(return_d, "return_d");
  uint32_t fun_id = emp::JSWrap(return_val, "return_val");
  emp::JSWrap(mouseover, "mouseover");
  //int jsresult = EM_ASM_INT({return emp.return_d($0);},5);

  std::cout << "about to do callbacks" << std::endl;

  t.SetAttr("r", "return_val");
  std::cout << "first callback done" << std::endl;
  t.SetAttr("cy", "return_val");
  t.SetAttr("cx", "return_val");
  t.SetStyle("fill", "green");
  D3::Selection("circle", true).On("mouseover", "mouseover");
  D3::Selection("circle", true).On("mouseover");
  
}


//The only way to bind data appropriately?
//d3.selectAll("svg").selectAll("line").data([1,2,3]).enter().append("line")
