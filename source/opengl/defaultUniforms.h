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
      auto transposed{value.transposed()};
      glUniformMatrix2fv(
        uniform, 1, GL_FALSE,  // WebGL does not support transposing the matrix
                               // on the GPU, so it must be done manually here
        transposed.data());
#else
      glUniformMatrix2fv(
        uniform,  // the uniform we want to set
        1,  // This just needs to be one, since we are only setting one value
        GL_TRUE,      // Transpose the matrix on the GPU since OpenGL is column
                      // major
        value.data()  // handle to the native pointer to the data held by the
                      // matrix
      );
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat3x3f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix3fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix3fv(uniform, 1, GL_TRUE, value.data());
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat4x4f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix4fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix4fv(uniform, 1, GL_TRUE, value.data());
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat2x3f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix2x3fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix2x3fv(uniform, 1, GL_TRUE, value.data());
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat4x2f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix4x2fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix4x2fv(uniform, 1, GL_TRUE, value.data());
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat2x4f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix2x4fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix2x4fv(uniform, 1, GL_TRUE, value.data());
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat4x3f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix4x3fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix4x3fv(uniform, 1, GL_TRUE, value.data());
#endif
      utils::catchGlError();
    }

    void setUniform(GLint uniform, const Mat3x4f& value) {
#ifdef EMSCRIPTEN
      auto transposed{value.transposed()};
      glUniformMatrix3x4fv(uniform, 1, GL_FALSE, transposed.data());
#else
      glUniformMatrix3x4fv(uniform, 1, GL_TRUE, value.data());
#endif
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

    void setUniform(GLint uniform, GLfloat value) {
      glUniform1f(uniform, value);
      utils::catchGlError();
    }

    void setUniform(GLint uniform, GLint value) {
      glUniform1i(uniform, value);
      utils::catchGlError();
    }

    void setUniform(GLint uniform, GLuint value) {
      glUniform1ui(uniform, value);
      utils::catchGlError();
    }
  }  // namespace opengl
}  // namespace emp
#endif  // DEFAULT_UNIFORMS
