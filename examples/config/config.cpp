#include <iostream>

#include "emp/config/config.hpp"
#include "config_setup.hpp"

int main()
{
  MyConfig config;
  config.Write("test.cfg");
  config.WriteMacros("test-macro.h");
  std::cout << "Ping!" << std::endl;
}
