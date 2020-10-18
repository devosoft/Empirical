#include <iostream>
#include <cmath>

int main()
{
//   const double steps = 1024.0;
//   for (double i = 0.0; i < steps; i += 1.0) {
//     double val = i/steps;
//     // std::cout << i << " " << steps << " " << val << " : " << std::log2(val) << std::endl;
//     std::cout << std::pow(2, val) << ",";
//     if (((int) i) % 8 == 7) std::cout << std::endl;
//     else std::cout << " ";
//   }


  // We really only need to know the effect of each bit past the decimal point on pow.
  std::cout.precision(16);
  std::cout.setf( std::ios::fixed, std:: ios::floatfield );
  double val = 1.0;
  for (int i = 0; i < 32; i++) {
    val /= 2.0;
    std::cout << std::pow(2.0, val) << ", ";
  }


  std::cout << std::endl;
}
