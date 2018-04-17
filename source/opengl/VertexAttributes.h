#ifndef VERTEX_ATTRIBUTES
#define VERTEX_ATTRIBUTES

#include "glutils.h"

namespace emp {
  namespace opengl {
    enum class VertexAttributeSize : GLint {
      One = 1,
      Two = 2,
      Three = 3,
      Four = 4
    };

    std::ostream& operator<<(std::ostream& out,
                             const VertexAttributeSize& size) {
      return out << static_cast<GLint>(size);
    }

    enum class VertexAttributeType : GLenum {
      Byte = GL_BYTE,
      UnsignedByte = GL_UNSIGNED_BYTE,
      Short = GL_SHORT,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Int = GL_INT,
      UnsignedInt = GL_UNSIGNED_INT,
    };

    std::ostream& operator<<(std::ostream& out,
                             const VertexAttributeType& type) {
      switch (type) {
        case VertexAttributeType::Byte:
          return out << "byte";
        case VertexAttributeType::UnsignedByte:
          return out << "unsigned byte";
        case VertexAttributeType::Short:
          return out << "short";
        case VertexAttributeType::UnsignedShort:
          return out << "unsigned short";
        case VertexAttributeType::Int:
          return out << "int";
        case VertexAttributeType::UnsignedInt:
          return out << "unsigned int";
      }
    }

    enum class FloatingVertexAttributeType : GLenum {
      Byte = GL_BYTE,
      UnsignedByte = GL_UNSIGNED_BYTE,
      Short = GL_SHORT,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Int = GL_INT,
      UnsignedInt = GL_UNSIGNED_INT,
      // HalfFloat = GL_HALF_FLOAT,
      Float = GL_FLOAT,
      Fixed = GL_FIXED,
      // INT_2_10_10_10_REV = GL_INT_2_10_10_10_REV,
      // UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV
    };

    std::ostream& operator<<(std::ostream& out,
                             const FloatingVertexAttributeType& type) {
      switch (type) {
        case FloatingVertexAttributeType::Byte:
          return out << "byte";
        case FloatingVertexAttributeType::UnsignedByte:
          return out << "unsigned byte";
        case FloatingVertexAttributeType::Short:
          return out << "short";
        case FloatingVertexAttributeType::UnsignedShort:
          return out << "unsigned short";
        case FloatingVertexAttributeType::Int:
          return out << "int";
        case FloatingVertexAttributeType::UnsignedInt:
          return out << "unsigned int";
        case FloatingVertexAttributeType::Float:
          return out << "float";
        case FloatingVertexAttributeType::Fixed:
          return out << "fixed";
      }
    }

    template <typename T>
    struct VertexAttributeTypeOf {
      static constexpr bool supported = false;
    };

    template <>
    struct VertexAttributeTypeOf<GLbyte> {
      static constexpr bool supported = true;
      static constexpr VertexAttributeType type = VertexAttributeType::Byte;
    };

    template <>
    struct VertexAttributeTypeOf<GLubyte> {
      static constexpr bool supported = true;
      static constexpr VertexAttributeType type =
        VertexAttributeType::UnsignedByte;
    };

    template <>
    struct VertexAttributeTypeOf<GLshort> {
      static constexpr bool supported = true;
      static constexpr VertexAttributeType type = VertexAttributeType::Short;
    };

    template <>
    struct VertexAttributeTypeOf<GLint> {
      static constexpr bool supported = true;
      static constexpr VertexAttributeType type = VertexAttributeType::Int;
    };

    template <>
    struct VertexAttributeTypeOf<GLuint> {
      static constexpr bool supported = true;
      static constexpr VertexAttributeType type =
        VertexAttributeType::UnsignedInt;
    };

    template <>
    struct VertexAttributeTypeOf<GLfloat> {
      static constexpr bool supported = true;
      static constexpr FloatingVertexAttributeType type =
        FloatingVertexAttributeType::Float;
    };

    template <>
    struct VertexAttributeTypeOf<GLushort> {
      static constexpr bool supported = true;
      static constexpr VertexAttributeType type = VertexAttributeType::Short;
    };

    template <typename T>
    struct VertexAttributes {
      static_assert(VertexAttributeTypeOf<T>::supported,
                    "The given type is not usable as a shader attribute");
      static constexpr auto size{VertexAttributeSize::One};
      static constexpr auto type{VertexAttributeTypeOf<T>::type};
    };

    template <typename T, std::size_t N>
    struct VertexAttributes<T[N]> {
      static_assert(1 <= N && N <= 4,
                    "OpenGL does not support empty arrays or arrays with more "
                    "than 4 elements");
      static_assert(VertexAttributeTypeOf<T>::supported,
                    "OpenGL does not support this type!");

      static constexpr VertexAttributeSize size =
        static_cast<VertexAttributeSize>(N);

      static constexpr auto type = VertexAttributeTypeOf<T>::type;
    };

  }  // namespace opengl
}  // namespace emp
#endif
