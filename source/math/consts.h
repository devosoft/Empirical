#ifndef CONSTS_H
#define CONSTS_H
#include <math.h>

namespace emp {
  namespace math {
    namespace consts {
      template <typename T>
      constexpr T pi = static_cast<T>(M_PI);
    }
  }  // namespace math
}  // namespace emp

#endif
