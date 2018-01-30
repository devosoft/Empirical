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
#ifndef EMSCRIPTEN
    class BufferAccess {
      private:
      GLenum access;

      constexpr BufferAccess(GLenum access) : access(access) {}

      public:
      static constexpr BufferAccess read() { return {GL_MAP_READ_BIT}; };
      static constexpr BufferAccess write() { return {GL_MAP_WRITE_BIT}; };

      BufferAccess& invalidatesRange(bool set = true) {
        access |= GL_MAP_INVALIDATE_RANGE_BIT & set;
        return *this;
      }

      BufferAccess& invalidatesBuffer(bool set = true) {
        access |= GL_MAP_INVALIDATE_BUFFER_BIT & set;
        return *this;
      }

      BufferAccess& explicitFlush(bool set = true) {
        access |= GL_MAP_FLUSH_EXPLICIT_BIT & set;
        return *this;
      }

      BufferAccess& unsynchronized(bool set = true) {
        access |= GL_MAP_UNSYNCHRONIZED_BIT & set;
        return *this;
      }

      explicit operator GLenum() const { return access; }
    };
#endif

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

    namespace __impl {
      GLuint boundBuffer = 0;
    }

    template <BufferType TYPE>
    class BufferObject {
      private:
      GLuint handle = 0;

      public:
      BufferObject(GLuint handle) : handle(handle) {}

      BufferObject(const BufferObject&) = delete;
      BufferObject(BufferObject&& other) noexcept : handle(other.handle) {
        other.handle = 0;
      }

      BufferObject& operator=(const BufferObject&) = delete;
      BufferObject& operator=(BufferObject&& other) noexcept {
        if (this != &other) {
          destroy();

          std::swap(handle, other.handle);
        }
        return *this;
      }

      ~BufferObject() { destroy(); }

      void destroy() {
        if (handle != 0) {
          glDeleteBuffers(1, &handle);
          if (__impl::boundBuffer == handle) __impl::boundBuffer = 0;
          utils::catchGlError();
        }
      }

      template <typename T>
      void init(const T* data, std::size_t size, BufferUsage usage) {
        bind();
        glBufferData(static_cast<GLenum>(TYPE), size, data,
                     static_cast<GLenum>(usage));
        utils::catchGlError();
      }

      template <typename T, std::size_t N>
      void init(const T (&data)[N], BufferUsage usage) {
        init(&data, sizeof(data), usage);
      }

      template <typename T>
      void init(const std::vector<T>& data, BufferUsage usage) {
        init(data.data(), sizeof(T) * data.size(), usage);
      }

      template <typename T>
      void subset(const T* data, std::size_t size, std::size_t offset = 0) {
        bind();
        glBufferSubData(static_cast<GLenum>(TYPE), offset, size, data);
        utils::catchGlError();
      }

      template <typename T, std::size_t N>
      void subset(const T (&data)[N]) {
        subset(&data, sizeof(data));
      }

      template <typename T>
      void subset(const std::vector<T>& data) {
        subset(data.data(), sizeof(T) * data.size());
      }
#ifndef EMSCRIPTEN
      template <class T>
      T* map(std::size_t offset, std::size_t length, BufferAccess access) {
        bind();
        auto buffer = static_cast<T*>(
          glMapBufferRange(static_cast<GLenum>(TYPE), offset, length,
                           static_cast<GLenum>(access)));
        utils::catchGlError();
        return buffer;
      }

      template <class T>
      T* map(std::size_t length, BufferAccess access) {
        return map<T>(0, length, access);
      }

      bool unmap() {
        bind();
        auto unmap = glUnmapBuffer(static_cast<GLenum>(TYPE));
        utils::catchGlError();
        return unmap;
      }
#endif

      BufferObject& bind() {
        if (__impl::boundBuffer != handle) {
          glBindBuffer(static_cast<GLenum>(TYPE), handle);
          utils::catchGlError();
          __impl::boundBuffer = handle;
        }

        return *this;
      }

      operator bool() const { return handle != 0; }
      operator GLuint() const { return handle; }
    };

    template <class F, BufferType... TYPES>
    void mapBuffers(F&& callback, BufferAccess access,
                    BufferObject<TYPES>&... buffers) {
      // Map all the buffers and send them into a callback to use them
      std::forward<F>(callback)(buffers.map(access)...);
      // Now that we are done with the buffers, unmap them
      // Hack to get around not having the c++17 fold expressions
      auto _ = [](auto&&...) {};
      _(buffers.unmap()...);
    }

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

    class VertexArrayObject {
      private:
      static GLuint boundVAO;
      GLuint handle = 0;

      public:
      explicit VertexArrayObject() {
        glGenVertexArrays(1, &handle);
        utils::catchGlError();
      }

      explicit VertexArrayObject(GLuint handle) : handle(handle) {}
      VertexArrayObject(const VertexArrayObject&) = delete;
      VertexArrayObject(VertexArrayObject&& other) : handle(other.handle) {
        other.handle = 0;
      }

      VertexArrayObject& operator=(const VertexArrayObject&) = delete;
      VertexArrayObject& operator=(VertexArrayObject&& other) {
        if (this != &other) {
          destoy();
          std::swap(handle, other.handle);
        }
        return *this;
      }

      ~VertexArrayObject() { destoy(); }

      void destoy() {
        if (handle != 0) {
          unbind();
          glDeleteVertexArrays(1, &handle);
          handle = 0;
        }
      }

      void bind() {
        emp_assert(handle != 0);
        if (boundVAO != handle) {
          glBindVertexArray(handle);
          boundVAO = handle;
        }
      }

      void unbind() {
        emp_assert(handle != 0);
        if (boundVAO == handle) {
          glBindVertexArray(0);
          boundVAO = 0;
        }
      }

      template <typename... T>
      VertexArrayObject& attr(T&&... vertexAttributes) {
        bind();
        applyAll(std::forward<T>(vertexAttributes)...);

        return *this;
      }

      operator bool() const { return handle != 0; }
      operator GLuint() const { return handle; }
    };

    GLuint VertexArrayObject::boundVAO = 0;

  }  // namespace opengl
}  // namespace emp
#endif
