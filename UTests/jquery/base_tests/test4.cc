#include <emscripten.h>
#include <iostream>

#include "../../jquery/JQManager.h"

int main() {
  emp::JQManager wm("emp_base");
  emp::JQElement * we = new emp::JQElement("mytext");
  wm.AddFront(*we);
  we->SetText("<p>This is a <b>TEST</b></p>");
}
