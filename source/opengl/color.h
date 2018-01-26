#ifndef EMP_OPENGL_COLOR_H
#define EMP_OPENGL_COLOR_H

#include "math/LinAlg.h"

namespace emp {
  namespace opengl {
    class Color {
      public:
      float r, g, b, a;

      constexpr Color(float r = 0, float g = 0, float b = 0, float a = 1)
        : r(r), g(g), b(b), a(a) {}

      constexpr static Color red(float v = 1, float a = 1) {
        return {v, 0, 0, a};
      }
      constexpr static Color green(float v = 1, float a = 1) {
        return {0, v, 0, a};
      }
      constexpr static Color blue(float v = 1, float a = 1) {
        return {0, 0, v, a};
      }
    };
    template <>
    struct VertexAttributes<Color> {
      static constexpr auto size{VertexAttributeSize::Four};
      static constexpr auto type{FloatingVertexAttributeType::Float};
    };

    void setUniform(GLint uniform, const Color& value) {
      glUniform4f(uniform, value.r, value.g, value.b, value.a);
      utils::catchGlError();
    }
  }  // namespace opengl
}  // namespace emp

#endif  // EMP_OPENGL_COLOR_H
