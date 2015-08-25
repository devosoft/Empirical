#include "../../web/web.h"

namespace UI = emp::web;
UI::Document doc("emp_base");

int main() 
{
  doc << "<h1>Testing!</h1>";

  doc << UI::TextArea([](const std::string & in){emp::Alert(in);}).Size(300,300);
}

