#include "../../tools/alert.h"
#include "../../web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

std::string TestFun() { return "abcd"; }

int x = 20;

int main()
{
  doc << "Testing testing!!!"
      << "<br>" << std::function<std::string()>(TestFun)
      << "<br>" << x;

  doc << UI::Button([](){ x++; }, "Test");
}
