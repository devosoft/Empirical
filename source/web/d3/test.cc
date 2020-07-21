#include <iostream>
#include <vector>
#include <string>

#include "web/init.h"
#include "web/Document.h"
#include "d3_init.h"
#include "selection.h"

// To compile from d3 directory:
// emcc -Wall -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-dollar-in-identifier-extension -std=c++17 -I../../  -pedantic -Wno-dollar-in-identifier-extension -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8"]' -s TOTAL_MEMORY=67108864 --js-library ../library_emp.js --js-library library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 -s WASM=0  test.cc -o main.js 

// WARNING: including the following breaks the code in main() for unknown reasons
// namespace UI = emp::web;
// UI::Document doc("emp_d3_test");

int main() {

  D3::Selection testSel = D3::Select("#emp_d3_test")
                              .Append("svg")
                              .SetAttr("height", 400)
                              .SetAttr("width", 400);

  size_t id = D3::internal::NextD3ID();
  std::cout << "This is the ID you got: " << id << std::endl;
}