#ifndef GLWRAP_H
#define GLWRAP_H

#include "gl.h"

#include <cstdint>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "VertexAttributes.h"
#include "base/assert.h"
#include "glutils.h"

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

    std::ostream& operator<<(std::ostream& out, const BufferType& buffer) {
      switch (buffer) {
        case BufferType::Array:
          return out << "GL_ARRAY_BUFFER";
        case BufferType::CopyRead:
          return out << "GL_COPY_READ_BUFFER";
        case BufferType::CopyWrite:
          return out << "GL_COPY_WRITE_BUFFER";
        case BufferType::ElementArray:
          return out << "GL_ELEMENT_ARRAY_BUFFER";
        case BufferType::PixelPack:
          return out << "GL_PIXEL_PACK_BUFFER";
        case BufferType::PixelUnpack:
          return out << "GL_PIXEL_UNPACK_BUFFER";
        case BufferType::TransformFeedback:
          return out << "GL_TRANSFORM_FEEDBACK_BUFFER";
        case BufferType::Uniform:
        default:
          return out << "GL_UNIFORM_BUFFER";
      }
    }

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
      static GLuint boundBuffer;

      public:
      BufferObject(BufferType type) : hasValue(true), type(type) {
        glGenBuffers(1, &handle);
        utils::catchGlError();
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
          if (boundBuffer == handle) boundBuffer = 0;
          utils::catchGlError();
        }
      }

      template <typename T, std::size_t N>
      void set(const T (&data)[N], BufferUsage usage) {
        bind();
        glBufferData(static_cast<GLenum>(type), sizeof(data), data,
                     static_cast<GLenum>(usage));
        utils::catchGlError();
      }

      template <typename T>
      void set(const std::vector<T>& data, BufferUsage usage) {
        bind();
        glBufferData(static_cast<GLenum>(type), sizeof(T) * data.size(),
                     data.data(), static_cast<GLenum>(usage));
        utils::catchGlError();
      }

      BufferObject& bind() {
        if (boundBuffer != handle) {
          glBindBuffer(static_cast<GLenum>(type), handle);
          utils::catchGlError();
          boundBuffer = handle;
        }

        return *this;
      }
    };

    GLuint BufferObject::boundBuffer = 0;

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
      VertexArrayObject(VertexArrayObject&& other)
        : handle(other.handle), buffers(std::move(other.buffers)) {
        other.hasValue = false;
      }

      VertexArrayObject& operator=(const VertexArrayObject&) = delete;
      VertexArrayObject& operator=(VertexArrayObject&& other) {
        if (this != &other) {
          destoy();

          hasValue = other.hasValue;
          handle = other.handle;
          other.hasValue = false;

          buffers = std::move(other.buffers);
        }
        return *this;
      }

      ~VertexArrayObject() { destoy(); }

      void destoy() {
        if (hasValue) {
          unbind();
          glDeleteVertexArrays(1, &handle);
          hasValue = false;
        }
      }

      void bind() {
        emp_assert(hasValue);
        if (boundVAO != handle) {
          glBindVertexArray(handle);
          boundVAO = handle;
        }
      }

      void unbind() {
        emp_assert(hasValue);
        if (boundVAO == handle) {
          glBindVertexArray(0);
          boundVAO = 0;
        }
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
        utils::catchGlError();

        glEnableVertexAttribArray(index);
        utils::catchGlError();
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
        utils::catchGlError();

        glEnableVertexAttribArray(index);
        utils::catchGlError();
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
        utils::catchGlError();

        glBindVertexArray(handle);
        utils::catchGlError();
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
