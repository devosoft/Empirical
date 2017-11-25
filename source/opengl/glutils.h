#ifndef GL_UTILS_H
#define GL_UTILS_H

#include "gl.h"

#include <ostream>
#ifndef NDEBUG
#include <sstream>
#include <stdexcept>
#endif

namespace emp {
  namespace opengl {
    namespace utils {
      enum class GlError : GLenum {
        NoError = GL_NO_ERROR,
        InvalidEnum = GL_INVALID_ENUM,
        InvalidValue = GL_INVALID_VALUE,
        InvalidOperation = GL_INVALID_OPERATION,
        InvalidFramebufferOperation = GL_INVALID_FRAMEBUFFER_OPERATION,
        OutOfMemory = GL_OUT_OF_MEMORY,
      };

      std::ostream& operator<<(std::ostream& out, const GlError& error) {
        switch (error) {
          case GlError::NoError:
            return out << "NO_ERROR";
          case GlError::InvalidEnum:
            return out << "INVALID_ENUM";
          case GlError::InvalidValue:
            return out << "INVALID_VALUE";
          case GlError::InvalidOperation:
            return out << "INVALID_OPERATION";
          case GlError::InvalidFramebufferOperation:
            return out << "INVALID_FRAMEBUFFER_OPERATION";
          case GlError::OutOfMemory:
            return out << "OUT_OF_MEMORY";
          default:
            return out;
        }
      }

      inline void catchGlError() {
#ifndef NDEBUG
        auto error{glGetError()};
        if (error != GL_NO_ERROR) {
          std::stringstream stream;
          stream << "gl error: " << static_cast<GlError>(error);
          auto str = stream.str();
          throw std::runtime_error(str.c_str());
        }
#endif
      }
    }  // namespace utils
  }    // namespace opengl
}  // namespace emp

#endif  // GL_UTILS_H
