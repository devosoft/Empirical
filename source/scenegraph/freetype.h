#ifndef EMP_SCENEGRAPH_FREETYPE_H
#define EMP_SCENEGRAPH_FREETYPE_H

#include <algorithm>
#include <iomanip>
#include <stdexcept>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "math/LinAlg.h"
#include "math/region.h"
#include "opengl/texture.h"

#include <memory>

namespace emp {
  namespace scenegraph {
    class FontFace;

    class FreeType {
      private:
      FT_Library library;

      public:
      FreeType() {
        auto err = FT_Init_FreeType(&library);
        emp_assert(err == 0, "Warning: FreeType failed to initialize");
      }
      FreeType(const FreeType&) = delete;
      FreeType(FreeType&&) = delete;
      FreeType& operator=(const FreeType&) = delete;
      FreeType& operator=(FreeType&&) = delete;

      ~FreeType() { FT_Done_FreeType(library); }

      operator const FT_Library&() const { return library; }

      FontFace load(const char* path, FT_Long face_index = 0,
                    GLenum texture = GL_TEXTURE1) const;
    };

    struct atlas_char_t {
      char character;
      math::Vec2f size;

      float atlas_x;
      float atlas_y;

      math::Vec2f cursor_advance;

      math::ColVec2f bearing;

      math::Region2f texture_region;
    };

    class FontFace {
      private:
      FT_Face face = nullptr;
      static constexpr char begin_ascii = 32;
      static constexpr char end_ascii = 126;

      std::vector<atlas_char_t> atlas;

      FontFace(const FreeType& library, const char* path, FT_Long face_index,
               GLenum texture)
        : atlas_texture(std::make_shared<opengl::Texture2d>(texture)) {
        auto err = FT_New_Face(library, path, face_index, &face);
        emp_assert(err == 0, "Warning: FreeType failed to load font");
      }

      friend FreeType;
      std::shared_ptr<opengl::Texture2d> atlas_texture;

      bool dirty = true;

      public:
      float atlas_width, atlas_height;

      FontFace(const FontFace&) = delete;
      FontFace(FontFace&& other)
        : face(other.face),
          atlas(std::move(other.atlas)),
          atlas_texture(std::move(other.atlas_texture)),
          atlas_width(other.atlas_width),
          atlas_height(other.atlas_height) {
        other.face = nullptr;
      }

      FontFace& operator=(const FontFace&) = delete;
      FontFace& operator=(FontFace&& other) {
        if (this != &other) {
          face = nullptr;
          std::swap(face, other.face);
          atlas = std::move(other.atlas);
          atlas_texture = std::move(other.atlas_texture);

          atlas_width = other.atlas_width;
          atlas_height = other.atlas_height;
          dirty = other.dirty;
        }
        return *this;
      }

      ~FontFace() {
        if (face != nullptr) {
          FT_Done_Face(face);
          face = nullptr;
        }
      }

      void SetFreeTypePixelSize(FT_UInt width, FT_UInt height) {
        auto err = FT_Set_Pixel_Sizes(face, width, height);
        emp_assert(err == 0);
        dirty = true;
      }

      void BulidAsciiAtlas() {
        if (!dirty) return;

        emp_assert(face != nullptr,
                   "Warning: Failed to build atlas for font, because the font "
                   "is not properly initialized");

        atlas_width = 0;
        atlas_height = 0;

        atlas.reserve(end_ascii - begin_ascii);
        for (char c = begin_ascii; c < end_ascii; ++c) {
          auto err = FT_Load_Char(face, c, FT_LOAD_RENDER);
          if (err) {
            emp_assert(err != FT_Err_Invalid_Size_Handle,
                       "Warning: You forgot to set the size");
            std::cerr << "Warning: faild to load glyph '" << c << "'"
                      << std::endl;
            continue;
          }

          auto g = face->glyph;
          atlas_char_t character{c,
                                 {static_cast<float>(g->bitmap.width),
                                  static_cast<float>(g->bitmap.rows)},
                                 atlas_width + 1,
                                 0,
                                 {static_cast<float>(g->advance.x / 64),
                                  static_cast<float>(g->advance.y / 64)},
                                 {static_cast<float>(g->bitmap_left),
                                  static_cast<float>(g->bitmap_top)}};

          atlas_width += character.size.x() + 1;
          atlas_height = std::max(atlas_height, character.size.y());

          atlas.push_back(character);
        }

#ifdef EMSCRIPTEN
        constexpr auto format{opengl::Texture2DFormat::Alpha};
#else
        constexpr auto format{opengl::Texture2DFormat::R};
#endif

        atlas_texture->Activate();
        atlas_texture->Bind();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        atlas_texture->Data(format, atlas_width, atlas_height,
                            opengl::TextureType::UnsignedByte, nullptr);

        for (auto& character : atlas) {
          if (FT_Load_Char(face, character.character, FT_LOAD_RENDER)) continue;
          if (character.size.x() <= 0 || character.size.y() <= 0) continue;

          float atlas_texture_x = character.atlas_x / atlas_width;
          float atlas_texture_y = character.atlas_y / atlas_height;

          character.texture_region.min = {atlas_texture_x, atlas_texture_y};
          character.texture_region.max = {
            atlas_texture_x + character.size.x() / atlas_width,
            atlas_texture_y + character.size.y() / atlas_height};

          auto g = face->glyph;

          atlas_texture->SubData(character.atlas_x, character.atlas_y,
                                 character.size.x(), character.size.y(), format,
                                 opengl::TextureType::UnsignedByte,
                                 g->bitmap.buffer);
        }
        atlas_texture->SetMinFilter(opengl::TextureMinFilter::Linear);
        atlas_texture->SetMagFilter(opengl::TextureMagFilter::Linear);
        atlas_texture->SetTextureWrap(opengl::TextureWrap::ClampToEdge,
                                      opengl::TextureWrap::ClampToEdge);

        dirty = false;
      }

      atlas_char_t Lookup(char character) const {
        auto idx = character - begin_ascii;
        emp_assert(idx < atlas.size());
        return atlas[idx];
      }

      const std::shared_ptr<opengl::Texture2d> ComputeAtlasTexture() {
        BulidAsciiAtlas();
        return atlas_texture;
      }
    };

    FontFace FreeType::load(const char* path, FT_Long face_index,
                            GLenum texture) const {
      return {*this, path, face_index, texture};
    }
  }  // namespace scenegraph
}  // namespace emp
#endif  // EMP_SCENEGRAPH_FREETYPE_H
