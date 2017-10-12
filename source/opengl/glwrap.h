#ifndef GLWRAP_H
#define GLWRAP_H

#include <GLES3/gl3.h>

#include <cstdint>
#include <unordered_map>

#include "base/assert.h"

namespace emp {
  namespace opengl {

    enum class BufferType : GLenum {
      Array = GL_ARRAY_BUFFER,
      // AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
      CopyRead = GL_COPY_READ_BUFFER,
      CopyWrite = GL_COPY_WRITE_BUFFER,
      // DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
      // DispatchIndirect = GL_DISPATCH_INDIRECT_BUFFER,
      ElementArray = GL_ELEMENT_ARRAY_BUFFER,
      PixelPack = GL_PIXEL_PACK_BUFFER,
      PixelUnpack = GL_PIXEL_UNPACK_BUFFER,
      // ShaderStorage = GL_SHADER_STORAGE_BUFFER,
      TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
      Uniform = GL_UNIFORM_BUFFER
    };

    enum class BufferUsage : GLenum {
      StreamDraw = GL_STREAM_DRAW,
      StreamRead = GL_STREAM_READ,
      StreamCopy = GL_STREAM_COPY,
      StaticDraw = GL_STATIC_DRAW,
      StaticRead = GL_STATIC_READ,
      StaticCopy = GL_STATIC_COPY,
      DynamicDraw = GL_DYNAMIC_DRAW,
      DynamicRead = GL_DYNAMIC_READ,
      DynamicCopy = GL_DYNAMIC_COPY
    };

    class BufferObject {
      private:
      bool hasValue;
      GLuint handle;
      BufferType type;

      public:
      BufferObject(BufferType type) : hasValue(true), type(type) {
        glGenBuffers(1, &handle);
        emp_assert(glGetError() == GL_NO_ERROR);
      }

      BufferObject(const BufferObject&) = delete;
      BufferObject(BufferObject&& other) noexcept
        : hasValue(other.hasValue), handle(other.handle), type(other.type) {
        other.hasValue = false;
      }

      BufferObject& operator=(const BufferObject&) = delete;
      BufferObject& operator=(BufferObject&& other) noexcept {
        if (this != &other) {
          destroy();

          hasValue = other.hasValue;
          handle = other.handle;
          type = other.type;

          other.hasValue = false;
        }
        return *this;
      }

      ~BufferObject() { destroy(); }

      void destroy() {
        if (hasValue) {
          hasValue = false;
          glDeleteBuffers(1, &handle);
          emp_assert(glGetError() == GL_NO_ERROR);
        }
      }

      template <typename T, std::size_t N>
      void push(const T (&data)[N], BufferUsage usage) {
        glBufferData(static_cast<GLenum>(type), sizeof(data), data,
                     static_cast<GLenum>(usage));
        emp_assert(glGetError() == GL_NO_ERROR);
      }

      BufferObject& bind() {
        glBindBuffer(static_cast<GLenum>(type), handle);
        emp_assert(glGetError() == GL_NO_ERROR);

        return *this;
      }
    };

    enum class VertexAttributeSize : GLint {
      One = 1,
      Two = 2,
      Three = 3,
      Four = 4
    };

    enum class VertexAttributeType : GLenum {
      Byte = GL_BYTE,
      UnsignedByte = GL_UNSIGNED_BYTE,
      Short = GL_SHORT,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Int = GL_INT,
      UnsignedInt = GL_UNSIGNED_INT,
    };

    enum class FloatingVertexAttributeType : GLenum {
      Byte = GL_BYTE,
      UnsignedByte = GL_UNSIGNED_BYTE,
      Short = GL_SHORT,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Int = GL_INT,
      UnsignedInt = GL_UNSIGNED_INT,
      HalfFloat = GL_HALF_FLOAT,
      Float = GL_FLOAT,
      Fixed = GL_FIXED,
      INT_2_10_10_10_REV = GL_INT_2_10_10_10_REV,
      UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV
    };

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
    struct VertexAttributes;

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

    class VertexArrayObject {
      private:
      static GLuint boundVAO;
      bool hasValue = true;
      GLuint handle;
      std::unordered_map<BufferType, BufferObject> buffers;

      friend class VertexArrayObjectConfigurator;

      explicit VertexArrayObject(
        GLuint handle, std::unordered_map<BufferType, BufferObject>&& buffers)
        : hasValue(true), handle(handle), buffers(std::move(buffers)) {}

      public:
      VertexArrayObject(const VertexArrayObject&) = delete;
      VertexArrayObject(VertexArrayObject&& other) : handle(other.handle) {
        other.hasValue = false;
      }
      VertexArrayObject& operator=(const VertexArrayObject&) = delete;
      VertexArrayObject& operator=(VertexArrayObject&& other) {
        if (this != &other) {
          hasValue = other.hasValue;
          handle = other.handle;

          other.hasValue = false;
        }
        return *this;
      }

      ~VertexArrayObject() {
        if (hasValue) {
          glDeleteVertexArrays(1, &handle);
        }
      }

      void bind() {
        emp_assert(hasValue);
        // TODO: EMP warning?
        // emp_assert(boundVAO != handle);
        glBindVertexArray(handle);
        boundVAO = handle;
      }

      void unbind() {
        emp_assert(boundVAO == handle);
        glBindVertexArray(0);
        boundVAO = 0;
      }

      operator bool() const { return hasValue; }
      operator GLuint() const { return handle; }

      template <BufferType TYPE>
      BufferObject& getBuffer() {
        return buffers.at(TYPE);
      }

      template <BufferType TYPE>
      const BufferObject& getBuffer() const {
        return buffers.at(TYPE);
      }
    };

    class VertexAttribute {
      private:
      GLuint index;
      VertexAttributeSize size;
      VertexAttributeType type;
      GLsizei stride;
      const void* offset;

      public:
      VertexAttribute(GLuint index, VertexAttributeSize size,
                      VertexAttributeType type, GLsizei stride = 0,
                      const void* offset = nullptr)
        : index(index),
          size(size),
          type(type),
          stride(stride),
          offset(offset) {}

      void apply() {
        glVertexAttribIPointer(index, static_cast<GLint>(size),
                               static_cast<GLenum>(type), stride, offset);
        emp_assert(glGetError() == GL_NO_ERROR);

        glEnableVertexAttribArray(index);
        emp_assert(glGetError() == GL_NO_ERROR);
      }
    };

    class FloatingVertexAttribute {
      private:
      GLuint index;
      VertexAttributeSize size;
      FloatingVertexAttributeType type;
      bool normalized;
      GLsizei stride;
      const void* offset;

      public:
      FloatingVertexAttribute(GLuint index, VertexAttributeSize size,
                              FloatingVertexAttributeType type,
                              bool normalized = false, GLsizei stride = 0,
                              const void* offset = nullptr)
        : index(index),
          size(size),
          type(type),
          normalized(normalized),
          stride(stride),
          offset(offset) {}

      void apply() {
        glVertexAttribPointer(index, static_cast<GLint>(size),
                              static_cast<GLenum>(type), normalized, stride,
                              offset);
        emp_assert(glGetError() == GL_NO_ERROR);

        glEnableVertexAttribArray(index);
        emp_assert(glGetError() == GL_NO_ERROR);
      }
    };

    template <typename... C>
    void applyAll(C&&...);

    template <typename H, typename... T>
    void applyAll(H&& head, T&&... tail) {
      std::forward<H>(head).apply();
      applyAll(std::forward<T>(tail)...);
    }

    template <>
    void applyAll() {}

    GLuint VertexArrayObject::boundVAO = 0;

    class VertexArrayObjectConfigurator {
      private:
      std::unordered_map<BufferType, BufferObject> buffers;
      bool hasValue = true;
      GLuint handle;

      public:
      VertexArrayObjectConfigurator() {
        glGenVertexArrays(1, &handle);
        emp_assert(glGetError() == GL_NO_ERROR);

        glBindVertexArray(handle);
        emp_assert(glGetError() == GL_NO_ERROR);
        VertexArrayObject::boundVAO = handle;
      }

      VertexArrayObjectConfigurator(const VertexArrayObjectConfigurator&) =
        delete;
      VertexArrayObjectConfigurator(VertexArrayObjectConfigurator&&) = delete;
      VertexArrayObjectConfigurator& operator=(
        const VertexArrayObjectConfigurator&) = delete;
      VertexArrayObjectConfigurator& operator=(
        VertexArrayObjectConfigurator&&) = delete;

      ~VertexArrayObjectConfigurator() { emp_assert(!hasValue); }

      operator GLuint() const {
        emp_assert(hasValue);
        return handle;
      }

      template <typename... T>
      VertexArrayObjectConfigurator& with(BufferType bufferType,
                                          T&&... vertexAttributes) {
        emp_assert(hasValue);
        emp_assert(buffers.find(bufferType) == buffers.end());

        // Create a new buffer. This also binds the buffer to be active
        auto& buffer = buffers.emplace(bufferType, bufferType).first->second;
        buffer.bind();

        applyAll(std::forward<T>(vertexAttributes)...);

        return *this;
      }

      VertexArrayObject finish() {
        emp_assert(hasValue);
        hasValue = false;
        return VertexArrayObject(handle, std::move(buffers));
      }

      operator VertexArrayObject() { return finish(); }
    };
  }  // namespace opengl
}  // namespace emp
#endif
