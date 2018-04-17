#ifndef EMP_OPENGL_TEXTURE_H
#define EMP_OPENGL_TEXTURE_H

#include <vector>
#include "opengl/color.h"
#include "opengl/glutils.h"

namespace emp {
  namespace opengl {
    enum class TextureBindTarget : GLenum {
      TwoDimensional = GL_TEXTURE_2D,
      CubeMap = GL_TEXTURE_CUBE_MAP,
#ifndef __EMSCRIPTEN__
      OneDimensional = GL_TEXTURE_1D,
      ThreeDimensional = GL_TEXTURE_3D,
      OneDimensionalArray = GL_TEXTURE_1D_ARRAY,
      TwoDimensionalArray = GL_TEXTURE_2D_ARRAY,
      Rectangle = GL_TEXTURE_RECTANGLE,
      CubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
      TwoDimensionalMultisample = GL_TEXTURE_2D_MULTISAMPLE,
      TwoDimensionalMultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
#endif
    };

    enum class TextureSwizzle : decltype(GL_RED) {
      Red = GL_RED,
      Green = GL_GREEN,
      Blue = GL_BLUE,
      Alpha = GL_ALPHA,
      Zero = GL_ZERO,
      One = GL_ONE
    };

    enum class TextureWrap : decltype(GL_CLAMP_TO_EDGE) {

      MirroredRepeat = GL_MIRRORED_REPEAT,
      Repeat = GL_REPEAT,
#ifndef __EMSCRIPTEN__
      MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
      ClampToEdge = GL_CLAMP_TO_EDGE,
      ClampToBorder = GL_CLAMP_TO_BORDER,
#endif
    };

    enum class TextureMinFilter : decltype(GL_NEAREST) {
      Nearest = GL_NEAREST,
      Linear = GL_LINEAR,
      NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
      LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
      NearestMipmapLienar = GL_NEAREST_MIPMAP_LINEAR,
      LinaerMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class TextureMagFilter : decltype(GL_NEAREST) {
      Nearest = GL_NEAREST,
      Linear = GL_LINEAR
    };

    enum class Texture2DFormat : GLint {
      Alpha = GL_ALPHA,
      Luminance = GL_LUMINANCE,
      LuminanceAlpha = GL_LUMINANCE_ALPHA,
      RGB = GL_RGB,
      RGBA = GL_RGBA
    };

    enum class TextureType : GLenum {
      UnsignedByte = GL_UNSIGNED_BYTE,
      UnsignedShort = GL_UNSIGNED_SHORT_5_6_5,
      UnsignedShort_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
      UnsignedShort_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,

#ifndef __EMSCRIPTEN__
      // OpenGL
      Byte = GL_BYTE,
      Short = GL_SHORT,
      UnsignedInt = GL_UNSIGNED_INT,
      Int = GL_INT,
      Float = GL_FLOAT,
      UnsignedByte_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
      UnsignedByte_2_3_3_Rev = GL_UNSIGNED_BYTE_2_3_3_REV,
      UnsignedShort_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
      UnsignedShort_5_6_5_Rev = GL_UNSIGNED_SHORT_5_6_5_REV,
      UnsignedShort_4_4_4_4_Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,
      UnsignedShort_1_5_5_5 = GL_UNSIGNED_SHORT_1_5_5_5_REV,
      UnsignedInt_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
      UnsignedInt_8_8_8_8_Rev = GL_UNSIGNED_INT_8_8_8_8_REV,
      UnsignedInt_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
      UnsignedInt_2_10_10_10 = GL_UNSIGNED_INT_2_10_10_10_REV,
#endif
    };
    template <typename>
    constexpr TextureType TextureTypeOf();

#define __emp_opengl_DEFINE_TEXTURE_TYPE_OF(gltype, variant) \
  template <>                                                \
  constexpr TextureType TextureTypeOf<gltype>() {            \
    return TextureType::variant;                             \
  }

    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLubyte, UnsignedByte);
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLushort, UnsignedShort);

#ifndef __EMSCRIPTEN__
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLbyte, Byte);
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLshort, Short);
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLint, Int);
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLuint, UnsignedInt);
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(GLfloat, Float);
    __emp_opengl_DEFINE_TEXTURE_TYPE_OF(Color, Float);
#endif
#undef __emp_opengl_DEFINE_TEXTURE_TYPE_OF

    template <typename>
    constexpr Texture2DFormat Texture2DFormatOf();

#define __emp_opengl_DEFINE_TEXTURE_2D_FORMAT_OF(gltype, variant) \
  template <>                                                     \
  constexpr Texture2DFormat Texture2DFormatOf<gltype>() {         \
    return Texture2DFormat::variant;                              \
  }

    __emp_opengl_DEFINE_TEXTURE_2D_FORMAT_OF(Color, RGBA);

#undef __emp_opengl_DEFINE_TEXTURE_2D_FORMAT_OF

    namespace __impl_emp_opengl_texture_base {
      GLenum active_texture = GL_TEXTURE0;

      template <TextureBindTarget TARGET>
      class Texture {
        static GLuint bound;

        public:
        GLenum texture;
        static constexpr TextureBindTarget target{TARGET};

        private:
        GLuint name = 0;

        public:
        explicit Texture(GLenum texture = GL_TEXTURE0) : texture(texture) {
          glActiveTexture(texture);
          utils::catchGlError();

          glGenTextures(1, &name);
          utils::catchGlError();

          Bind();
        }

        Texture(const Texture&) = delete;
        // Texture(Texture&& other) : name(other.name) { other.name = 0; }
        Texture(Texture&& other) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) = delete;
        // Texture& operator=(Texture&& other) {
        //   std::swap(name, other.name);
        //   return *this;
        // }

        virtual ~Texture() {
          if (name != 0) {
            if (name == bound) bound = 0;

            glDeleteTextures(1, &name);
            utils::catchGlError();
          }
        }

        operator GLuint() const { return name; }
        operator bool() const { return name != 0; }

        void Activate(GLenum texture) {
          this->texture = texture;
          Activate();
        }

        void Activate() {
          glActiveTexture(texture);
          utils::catchGlError();
        }

        void Bind() {
          Activate();
          if (bound != name) {
            glBindTexture(static_cast<GLenum>(target), name);
            utils::catchGlError();
          }
        }

        void SetDepthStencilTextureMode() {}
        void SetBaseMipmapLevel(GLint value = 0) {
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_BASE_LEVEL,
                          value);
          utils::catchGlError();
        }
#ifndef __EMSCRIPTEN__
        void SetBorderColor(const Color& color) {
          glTexParameterfv(static_cast<GLenum>(target), GL_TEXTURE_BORDER_COLOR,
                           color.RgbaPtr());
          utils::catchGlError();
        }
#endif

        void SetCompareFunc() {}

        void SetCompareMode() {}

        void SetLODBias();

        void SetMinFilter(TextureMinFilter filter) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_MIN_FILTER,
                          static_cast<decltype(GL_NEAREST)>(filter));
          utils::catchGlError();
        }

        void SetMagFilter(TextureMagFilter filter) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_MAG_FILTER,
                          static_cast<decltype(GL_NEAREST)>(filter));
          utils::catchGlError();
        }

        void SetMinLOD(float value) {
          Bind();
          glTexParameterf(static_cast<GLenum>(target), GL_TEXTURE_MIN_LOD,
                          value);
          utils::catchGlError();
        }
        void SetMaxLOD(float value) {
          Bind();
          glTexParameterf(static_cast<GLenum>(target), GL_TEXTURE_MAX_LOD,
                          value);
          utils::catchGlError();
        }
        void SetMaxMipmapLevel(int value) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_MAX_LEVEL,
                          value);
          utils::catchGlError();
        }

        void SetRedSwizzle(TextureSwizzle value) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_SWIZZLE_R,
                          static_cast<GLint>(value));
          utils::catchGlError();
        }
        void SetGreenSwizzle(TextureSwizzle value) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_SWIZZLE_G,
                          static_cast<GLint>(value));
          utils::catchGlError();
        }
        void SetBlueSwizzle(TextureSwizzle value) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_SWIZZLE_B,
                          static_cast<GLint>(value));
          utils::catchGlError();
        }
        void SetAlphaSwizzle(TextureSwizzle value) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_SWIZZLE_A,
                          static_cast<GLint>(value));
          utils::catchGlError();
        }

        void SetSwizzle(TextureSwizzle red = TextureSwizzle::Red,
                        TextureSwizzle green = TextureSwizzle::Green,
                        TextureSwizzle blue = TextureSwizzle::Blue,
                        TextureSwizzle alpha = TextureSwizzle::Alpha) {
#ifdef __EMSCRIPTEN__
          SetRedSwizzle(red);
          SetGreenSwizzle(green);
          SetBlueSwizzle(blue);
          SetAlphaSwizzle(alpha);
#else
          GLint params[] = {static_cast<GLint>(red), static_cast<GLint>(green),
                            static_cast<GLint>(blue),
                            static_cast<GLint>(alpha)};
          Bind();
          glTexParameteriv(static_cast<GLenum>(target), GL_TEXTURE_SWIZZLE_RGBA,
                           params);
          utils::catchGlError();
#endif
        }

        void SetTextureWrapS(TextureWrap wrap) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_WRAP_S,
                          static_cast<GLint>(wrap));
          utils::catchGlError();
        }

        void SetTextureWrapT(TextureWrap wrap) {
          Bind();
          glTexParameteri(static_cast<GLenum>(target), GL_TEXTURE_WRAP_T,
                          static_cast<GLint>(wrap));
          utils::catchGlError();
        }
      };
      template <TextureBindTarget TARGET>
      constexpr TextureBindTarget Texture<TARGET>::target;

      template <TextureBindTarget TARGET>
      GLuint Texture<TARGET>::bound{0};

      template <TextureBindTarget TARGET>
      class Texture2d : public Texture<TARGET> {
        public:
        using Texture<TARGET>::target;
        using Texture<TARGET>::Texture;

        template <typename T>
        void Data(GLint mipmap_level, Texture2DFormat internal_format,
                  GLsizei width, GLsizei height, Texture2DFormat format,
                  TextureType type, T&& data) {
          Texture<TARGET>::Bind();
          glTexImage2D(static_cast<GLenum>(target), mipmap_level,
                       static_cast<GLint>(internal_format), width, height, 0,
                       static_cast<GLint>(format), static_cast<GLenum>(type),
                       &data[0]);
          utils::catchGlError();
        }

        template <typename T>
        void Data(GLint mipmap_level, Texture2DFormat internal_format,
                  GLsizei width, GLsizei height, Texture2DFormat format,
                  T&& data) {
          using value_type = std::decay_t<decltype(*std::begin(data))>;
          Texture<TARGET>::Bind();
          Data(mipmap_level, internal_format, width, height, format,
               TextureTypeOf<value_type>(), data);
        }

        template <typename T>
        void Data(GLint mipmap_level, Texture2DFormat format, GLsizei width,
                  GLsizei height, TextureType type, T&& data) {
          Data(mipmap_level, format, width, height, format, type,
               std::forward<T>(data));
        }

        template <typename T>
        void Data(GLint mipmap_level, Texture2DFormat format, GLsizei width,
                  GLsizei height, T&& data) {
          Data(mipmap_level, format, width, height, format,
               std::forward<T>(data));
        }

        template <typename T>
        void Data(Texture2DFormat format, GLsizei width, GLsizei height,
                  TextureType type, T&& data) {
          Data(0, format, width, height, format, type, std::forward<T>(data));
        }

        template <typename T>
        void Data(Texture2DFormat format, GLsizei width, GLsizei height,
                  T&& data) {
          Data(0, format, width, height, format, std::forward<T>(data));
        }

        template <typename T>
        void Data(GLsizei width, GLsizei height, T&& data) {
          using value_type = std::decay_t<decltype(*std::begin(data))>;
          Data(0, Texture2DFormatOf<value_type>(), width, height, data);
        }

        template <typename T>
        void Data(GLint mipmap_level, GLsizei width, GLsizei height, T&& data) {
          using value_type = std::decay_t<decltype(*std::begin(data))>;
          Data(mipmap_level, Texture2DFormatOf<value_type>(), width, height,
               data);
        }

        void SetTextureWrap(TextureWrap s, TextureWrap t) {
          Texture<TARGET>::SetTextureWrapS(s);
          Texture<TARGET>::SetTextureWrapT(t);
        }
      };
    };  // namespace __impl_emp_opengl_texture_base

    template <TextureBindTarget>
    class Texture;

    template <>
    class Texture<TextureBindTarget::TwoDimensional>
      : public __impl_emp_opengl_texture_base::Texture2d<
          TextureBindTarget::TwoDimensional> {
      using __impl_emp_opengl_texture_base::Texture2d<
        TextureBindTarget::TwoDimensional>::target;
      using __impl_emp_opengl_texture_base::Texture2d<target>::Texture2d;
    };

    using Texture2d = Texture<TextureBindTarget::TwoDimensional>;

    template <TextureBindTarget TARGET>
    void setUniform(
      GLint uniform,
      const __impl_emp_opengl_texture_base::Texture<TARGET>& texture) {
      glUniform1i(uniform, texture.texture - GL_TEXTURE0);
      utils::catchGlError();
    }

  }  // namespace opengl
}  // namespace emp

#endif
