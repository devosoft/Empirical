#ifndef DEFAULT_UNIFORMS

#define DEFAULT_UNIFORMS

#include "../math/LinAlg.h"
#include "glutils.h"

namespace emp {
  namespace opengl {
    using namespace emp::math;

    void setUniform(GLint uniform, const Mat2x2f& value) {
      // The canonical representation of these matricies is transposed relative
      // to the representation used by OpenGL, so we have to transpose them
      // before they can be used.
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(glUniformMatrix2fv(
        uniform, 1, GL_FALSE,  // WebGL does not support transposing the matrix
                               // on the GPU, so it must be done manually here
        transposed.Data()));
#else
      emp_checked_gl_void(glUniformMatrix2fv(
        uniform,  // the uniform we want to set
        1,  // This just needs to be one, since we are only setting one value
        GL_TRUE,  // Transpose the matrix on the GPU since OpenGL is column
                  // major
        value.Data()  // handle to the native pointer to the data held by the
                      // matrix
        ));
#endif
    }

    void setUniform(GLint uniform, const Mat3x3f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix3fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix3fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Mat4x4f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix4fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix4fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Mat2x3f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix2x3fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix2x3fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Mat4x2f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix4x2fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix4x2fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Mat2x4f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix2x4fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix2x4fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Mat4x3f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix4x3fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix4x3fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Mat3x4f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.Transposed()};
      emp_checked_gl_void(
        glUniformMatrix3x4fv(uniform, 1, GL_FALSE, transposed.Data()));
#else
      emp_checked_gl_void(
        glUniformMatrix3x4fv(uniform, 1, GL_TRUE, value.Data()));
#endif
    }

    void setUniform(GLint uniform, const Vec2f& value) {
      emp_checked_gl_void(glUniform2fv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec3f& value) {
      emp_checked_gl_void(glUniform3fv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec4f& value) {
      emp_checked_gl_void(glUniform4fv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec2i& value) {
      emp_checked_gl_void(glUniform2iv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec3i& value) {
      emp_checked_gl_void(glUniform3iv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec4i& value) {
      emp_checked_gl_void(glUniform4iv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec2u& value) {
      emp_checked_gl_void(glUniform2uiv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec3u& value) {
      emp_checked_gl_void(glUniform3uiv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, const Vec4u& value) {
      emp_checked_gl_void(glUniform4uiv(uniform, 1, value.Data()));
    }

    void setUniform(GLint uniform, GLfloat value) {
      emp_checked_gl_void(glUniform1f(uniform, value));
    }

    void setUniform(GLint uniform, GLint value) {
      emp_checked_gl_void(glUniform1i(uniform, value));
    }

    void setUniform(GLint uniform, GLuint value) {
      emp_checked_gl_void(glUniform1ui(uniform, value));
    }
  }  // namespace opengl
}  // namespace emp
#endif  // DEFAULT_UNIFORMS
