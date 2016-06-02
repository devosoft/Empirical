#include <iostream>

#include "../../tools/macros.h"

#define  testing(x, y) std::cout <<  x + y << std::endl;

int main()
{
  std::cout << "Testing." << std::endl;

  EMP_WRAP_EACH_1ARG(testing, 1, 10, 20, 30)
}
