#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <string>
#include <sstream>
#include <type_traits>

#define EMP_FUNCTION_TIMER(TEST_FUN) {                                       \
    std::clock_t emp_start_time = std::clock();                              \
    auto emp_result = TEST_FUN;                                              \
    std::clock_t emp_tot_time = std::clock() - emp_start_time;               \
    std::cout << "Time: "                                                    \
              << 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC  \
              << " ms" << std::endl;                                         \
    std::cout << "Result: " << emp_result << std::endl;                      \
  }

namespace emp {

  // Toggle an input bool.
  bool toggle(bool & in_bool) { return (in_bool = !in_bool); }

  // % is actually remainder; this is a proper modulus command that handles negative #'s correctly.
  int mod(int in_val, int mod_val) {
    return (in_val < 0) ? (in_val % mod_val + mod_val) : (in_val % mod_val);
  }

  // A fast (O(log p)) integer-power command.
  int pow(int base, int p) {
    if (p == 0) return 1;
    if (p < 0) return 0;
    
    int r = 1;
    while (true) {
      if (p & 1) r *= base;
      if (!(p >>= 1)) return r;
      base *= base;
    }
  }

  // Swap the value of two variables.
  template <typename TYPE> void swap(TYPE & in1, TYPE & in2) { TYPE tmp = in1; in1 = in2; in2 = tmp; }

  // Run both min and max on a value to put it into a desired range.
  template <typename TYPE> TYPE to_range(const TYPE & value, const TYPE & in_min, const TYPE & in_max) {
    return (value < in_min) ? in_min : ((value > in_max) ? in_max : value);
  }


  //  ----- Variadic Template Helpers! -----

  // The following functions take in a test type and a list of types and return the index that
  // matches the test type in question.
  template <typename TEST_TYPE>
  constexpr int get_type_index() {
    // @CAO We don't have a type that matches, so ideally trigger a compile time error.
    // Given we need this to be constexpr, even run-time errors aren't easy.
    // emp_assert(false && "trying to find index of non-existant type");
    return -1000000;
  }
  template <typename TEST_TYPE, typename FIRST_TYPE, typename... TYPE_LIST>
  constexpr int get_type_index() {
    return (std::is_same<TEST_TYPE, FIRST_TYPE>()) ? 0 : (get_type_index<TEST_TYPE,TYPE_LIST...>() + 1);
  }
};

#endif
