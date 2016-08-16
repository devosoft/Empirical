#include <iostream>
#include <cmath>

int main()
{
  const double steps = 1024.0;
  for (double i = 0.0; i < steps; i += 1.0) {
    double val = 1.0 + i/steps;
    // std::cout << i << " " << steps << " " << val << " : " << std::log2(val) << std::endl;
    std::cout << std::log2(val) << ", ";
  }
  std::cout << std::endl;
}
