#include <iostream>

#include "config_setup.hpp"
#include "emp/config/config.hpp"

int main()
{
  MyConfig config;
  config.Write("test.cfg");
  config.WriteMacros("test-macro.h");
  std::cout << "Ping!" << std::endl;
}
