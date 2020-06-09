


#include <iostream>
#include <vector>
#include <string>

#include "web/init.h"
#include "web/Document.h"
#include "d3_init.h"
#include "selection.h"

// namespace UI = emp::web;
// UI::Document doc("emp_d3_test");

// EM_JS(const char*, hello, (), {
//   const js_str = "Hello World!";
//   return js_str;
// });

int main() {

  D3::internal::get_emp_d3();
  size_t id = D3::internal::NextD3ID();
  std::cout << "This is the ID you got: " << id << std::endl;
  // std::cout << "This string is from javascript: " << hello() << std::endl;

}