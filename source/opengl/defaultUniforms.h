#ifndef DEFAULT_UNIFORMS

#define DEFAULT_UNIFORMS

#include "../math/LinAlg.h"
#include "glutils.h"

namespace emp {
  namespace opengl {
    using namespace emp::math;

    void setUniform(GLint uniform, const Mat2x2f& value) {
      glUniformMatrix2fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat3x3f& value) {
      glUniformMatrix3fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat4x4f& value) {
      glUniformMatrix4fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat2x3f& value) {
      glUniformMatrix2x3fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat4x2f& value) {
      glUniformMatrix4x2fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat2x4f& value) {
      glUniformMatrix2x4fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat4x3f& value) {
      glUniformMatrix4x3fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat3x4f& value) {
      glUniformMatrix3x4fv(uniform, 1, GL_TRUE, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec2f& value) {
      glUniform2fv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec3f& value) {
      glUniform3fv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec4f& value) {
      glUniform4fv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec2i& value) {
      glUniform2iv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec3i& value) {
      glUniform3iv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec4i& value) {
      glUniform4iv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec2u& value) {
      glUniform2uiv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec3u& value) {
      glUniform3uiv(uniform, 1, value.data());
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Vec4u& value) {
      glUniform4uiv(uniform, 1, value.data());
      utils::catchGlError();
    }
  }  // namespace opengl
}  // namespace emp
#endif  // DEFAULT_UNIFORMS
