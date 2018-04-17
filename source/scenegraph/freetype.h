#ifndef EMP_SCENEGRAPH_FREETYPE_H
#define EMP_SCENEGRAPH_FREETYPE_H

#include <stdexcept>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "math/LinAlg.h"
#include "opengl/texture.h"

namespace emp {
  class FreeTypeError : public std::runtime_error {
    using runtime_error::runtime_error;
  };
  class FontError : public std::runtime_error {
    using runtime_error::runtime_error;
  };

  class FontFace;

  class FreeType {
    private:
    FT_Library library;

    public:
    FreeType() {
      if (FT_Init_FreeType(&library)) {
        throw FreeTypeError("Failed to initialize FreeType");
      }
    }
    FreeType(const FreeType&) = delete;
    FreeType(FreeType&&) = delete;
    FreeType& operator=(const FreeType&) = delete;
    FreeType& operator=(FreeType&&) = delete;

    ~FreeType() { FT_Done_FreeType(library); }

    operator const FT_Library&() const { return library; }

    FontFace load(const char* path, FT_Long face_index = 0) const;
  };

  class FontFace {
    private:
    FT_Face face;

    FontFace(const FreeType& library, const char* path, FT_Long face_index) {
      if (FT_New_Face(library, path, face_index, &face)) {
        throw FontError("Failed to load the font file");
      }
    }

    friend FreeType;

    public:
    FontFace() = delete;
    FontFace(const FontFace&) = delete;
    FontFace(FontFace&&) = delete;
    FontFace& operator=(const FontFace&) = delete;
    FontFace& operator=(FontFace&&) = delete;

    ~FontFace() { FT_Done_Face(face); }

    void SetPixelSize(FT_UInt width, FT_UInt height) {
      FT_Set_Pixel_Sizes(face, width, height);
    }

    void RenderText(const std::string& text, const math::Vec2f& position) {
      using namespace emp::opengl;
      Texture2d texture;

      for (auto& c : text) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          continue;
        }

        auto glyph = face->glyph;

        texture.Data(Texture2DFormat::Alpha, glyph->bitmap.width,
                     glyph->bitmap.rows, TextureType::UnsignedByte,
                     glyph->bitmap.buffer);
      }
    }
  };

  FontFace FreeType::load(const char* path, FT_Long face_index) const {
    return {*this, path, face_index};
  }
}  // namespace emp
#endif  // EMP_SCENEGRAPH_FREETYPE_H
