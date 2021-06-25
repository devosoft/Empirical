#ifndef D3_WEB_TESTING_UTILS_H
#define D3_WEB_TESTING_UTILS_H

#include "control/Signal.hpp"
#include "base/vector.hpp"
#include "web/JSWrap.hpp"
#include "web/Animate.hpp"
#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>

/// Convenience function to reset D3 js namespace.
void ResetD3Context() {
  EM_ASM({
    emp_d3.clear_emp_d3();  // Reset the emp_d3 object tracker
  });
}

#endif