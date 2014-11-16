#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <string>
#include <sstream>

#define EMP_FUNCTION_TIMER(TEST_FUN) {                                  \
    std::clock_t emp_start_time = std::clock();                         \
    auto emp_result = TEST_FUN;                                         \
    std::clock_t emp_tot_time = std::clock() - emp_start_time;          \
    std::cout << "Time: "                                               \
              << 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC      \
              << " ms" << std::endl;                                    \
    std::cout << "Result: " << emp_result << std::endl;                 \
  }

namespace emp {

};

#endif
