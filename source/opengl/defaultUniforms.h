#ifndef DEFAULT_UNIFORMS

#define DEFAULT_UNIFORMS

#include "../math/LinAlg.h"

namespace emp {
  namespace opengl {
    using namespace emp::math;

    void setUniform(GLint uniform, const Mat2x2f& value) {
      glUniformMatrix2fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat3x3f& value) {
      glUniformMatrix3fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat4x4f& value) {
      glUniformMatrix4fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat2x3f& value) {
      glUniformMatrix2x3fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat4x2f& value) {
      glUniformMatrix4x2fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat2x4f& value) {
      glUniformMatrix2x4fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat4x3f& value) {
      glUniformMatrix4x3fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }

    void setUniform(GLint uniform, const Mat3x4f& value) {
      glUniformMatrix3x4fv(uniform, 1, GL_TRUE, value.data());
      emp_assert(glGetError() == GL_NO_ERROR)
    }
  }  // namespace opengl
}  // namespace emp
#endif  // DEFAULT_UNIFORMS
