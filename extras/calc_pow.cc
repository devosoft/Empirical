#include <iostream>
#include <cmath>

int main()
{
  const double steps = 1024.0;
  for (double i = 0.0; i < steps; i += 1.0) {
    double val = i/steps;
    // std::cout << i << " " << steps << " " << val << " : " << std::log2(val) << std::endl;
    std::cout << std::pow(2, val) << ",";
    if (((int) i) % 8 == 7) std::cout << std::endl;
    else std::cout << " ";
  }
  std::cout << std::endl;
}
