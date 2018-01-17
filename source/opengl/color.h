#ifndef EMP_OPENGL_COLOR_H
#define EMP_OPENGL_COLOR_H

#include "math/LinAlg.h"

namespace emp {
  namespace opengl {
    class Color : public math::Vec4f {
      public:
      using math::Vec4f::Vec4f;

      constexpr auto red() const { return this->x(); }
      constexpr auto green() const { return this->y(); }
      constexpr auto blue() const { return this->z(); }
      constexpr auto alpha() const { return this->w(); }
    };

    namespace colors {
      constexpr Color Red{1, 0, 0, 0};
    };
  }  // namespace opengl
}  // namespace emp

#endif  // EMP_OPENGL_COLOR_H
