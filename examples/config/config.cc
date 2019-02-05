#include <iostream>

#include "../../source/config/config.h"

#include "config_setup.h"

int main()
{
  MyConfig config;
  config.Write("test.cfg");
  config.WriteMacros("test-macro.h");
  std::cout << "Ping!" << std::endl;
}
