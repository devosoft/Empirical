#ifndef EMP_SCENEGRAPH_SHAPES_H
#define EMP_SCENEGRAPH_SHAPES_H

#include "core.h"
#include "math/region.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "opengl/texture.h"
#include "scenegraph/freetype.h"
#include "tools/resources.h"

namespace emp {
  namespace scenegraph {
    namespace shapes {

      class FilledRectangle : public Child {
        private:
        opengl::VertexArrayObject vao;
        opengl::BufferObject<opengl::BufferType::Array> vertices_buffer;
        opengl::BufferObject<opengl::BufferType::ElementArray> triangles_buffer;

        public:
        math::Region2f size;
        opengl::Color fill;
        ResourceRef<opengl::ShaderProgram> shader;

        template <typename S = std::string>
        FilledRectangle(opengl::GLCanvas& canvas, const math::Region2f& size,
                        const opengl::Color& fill = {0, 0, 0, 1},
                        S&& shader = "DefaultSolidColor")
          : vao(canvas.MakeVAO()),
            vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
            triangles_buffer(
              canvas.makeBuffer<opengl::BufferType::ElementArray>()),
            size{size},
            fill{fill},
            shader(std::forward<S>(shader)) {
          using namespace emp::opengl;
          using namespace emp::math;

          vao.bind();
          vertices_buffer.init({Vec3f{size.min.x(), size.max.y(), 0},
                                Vec3f{size.max.x(), size.max.y(), 0},
                                Vec3f{size.max.x(), size.min.y(), 0},
                                Vec3f{size.min.x(), size.min.y(), 0}},
                               BufferUsage::StaticDraw);

          triangles_buffer.init(
            {
              0, 1, 2,  // First Triangle
              2, 3, 0  // Second Triangle
            },
            BufferUsage::StaticDraw);

          this->shader.OnSet([this](auto& value) {
            vao.bind();
            vao.attr(this->shader->Attribute<Vec3f>("position"));
          });

          // SetShaderName(shader_name);
        }

        virtual ~FilledRectangle() {}

        void RenderRelative(const RenderSettings& settings,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;

          shader->Use();
          vao.bind();

          shader->Uniform("projection") = settings.projection;
          shader->Uniform("view") = settings.view;

          shader->Uniform("model") = transform;
          shader->Uniform("color") = fill;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      };

      class TextureView : public Child {
        private:
        opengl::VertexArrayObject vao;
        opengl::BufferObject<opengl::BufferType::Array> vertices_buffer;
        opengl::BufferObject<opengl::BufferType::ElementArray> triangles_buffer;

        struct data_t {
          math::Vec3f position;
          math::Vec2f texture_coordinates;
        };

        public:
        math::Region2f size;
        math::Region2f texture_corrdinates;
        std::shared_ptr<opengl::Texture2d> texture;
        ResourceRef<opengl::ShaderProgram> shader;

        template <typename S = std::string>
        TextureView(opengl::GLCanvas& canvas, const math::Region2f& size,
                    std::shared_ptr<opengl::Texture2d> texture,
                    S&& shader = "DefaultTextured")
          : vao(canvas.MakeVAO()),
            vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
            triangles_buffer(
              canvas.makeBuffer<opengl::BufferType::ElementArray>()),
            size{size},
            texture{texture},
            shader(std::forward<S>(shader)) {
          using namespace emp::opengl;
          using namespace emp::math;

          vertices_buffer.init(
            {data_t{{size.min.x(), size.max.y(), 0}, {0, 0}},
             data_t{{size.max.x(), size.max.y(), 0}, {1, 0}},
             data_t{{size.max.x(), size.min.y(), 0}, {1, 1}},
             data_t{{size.min.x(), size.min.y(), 0}, {0, 1}}},
            BufferUsage::StaticDraw);

          triangles_buffer.init(
            {
              0, 1, 2,  // First Triangle
              2, 3, 0  // Second Triangle
            },
            BufferUsage::StaticDraw);

          this->shader.OnSet([this](auto&) {
            vao.bind();
            vertices_buffer.bind();
            triangles_buffer.bind();

            vao.attr(this->shader->Attribute("position", &data_t::position));
            vao.attr(
              this->shader->Attribute("uv", &data_t::texture_coordinates));
          });
        }

        virtual ~TextureView() {}

        void RenderRelative(const RenderSettings& settings,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;

          shader->Use();
          vao.bind();

          shader->Uniform("projection") = settings.projection;
          shader->Uniform("view") = settings.view;

          shader->Uniform("model") = transform;
          // shader.Uniform("color") = Color::white();
          shader->Uniform("tex") = *texture;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      };

      class Text : public Child {
        private:
        std::string font_name;
        std::string text;
        std::string shader_name;
        float height = 8;
        opengl::VertexArrayObject vao;
        opengl::BufferObject<opengl::BufferType::Array> vertices_buffer;
        ResourceRef<FontFace> font;
        ResourceRef<opengl::ShaderProgram> shader;

        struct data_t {
          math::Vec3f position;
          math::Vec2f texture_coordinates;
        };

        public:
        math::Region2f size;
        math::Region2f texture_corrdinates;
        std::shared_ptr<opengl::Texture2d> texture;

        template <typename F, typename S = std::string>
        Text(opengl::GLCanvas& canvas, const std::string& text, F&& font,
             S&& shader = "DefaultFont")
          : vao(canvas.MakeVAO()),
            vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
            // triangles_buffer(
            //   canvas.makeBuffer<opengl::BufferType::ElementArray>()),
            font(std::forward<F>(font)),
            shader(std::forward<S>(shader)) {
          using namespace emp::opengl;
          using namespace emp::math;

          SetText(text);

          this->shader.OnSet([this](auto& value) {
            vao.bind();
            vertices_buffer.bind();
            vao.attr(this->shader->Attribute("position", &data_t::position));
            vao.attr(
              this->shader->Attribute("uv", &data_t::texture_coordinates));
          });
        }

        template <typename S>
        void SetText(S&& text) {
          using namespace emp::opengl;
          using namespace emp::math;

          std::vector<data_t> points;
          Vec2f cursor{0, 0};

          float scale = height / font->atlas_height;

          int i = 0;
          for (auto& c : text) {
            auto info = font->Lookup(c);
            auto lcursor = cursor;
            cursor = cursor + Vec2f{info.cursor_advance.x() * scale,
                                    info.cursor_advance.y() * scale};

            if (info.size.x() <= 0 || info.size.y() <= 0) continue;

            auto max = lcursor + Vec2f{info.bearing.x() * scale,
                                       info.bearing.y() * scale};
            auto min =
              max - Vec2f{info.size.x() * scale, info.size.y() * scale};

            auto tmin = info.texture_region.min;
            auto tmax = info.texture_region.max;

            points.push_back({{min.x(), min.y(), 0}, {tmin.x(), tmax.y()}});
            points.push_back({{max.x(), min.y(), 0}, {tmax.x(), tmax.y()}});

            points.push_back({{min.x(), max.y(), 0}, {tmin.x(), tmin.y()}});
            points.push_back({{max.x(), min.y(), 0}, {tmax.x(), tmax.y()}});

            points.push_back({{min.x(), max.y(), 0}, {tmin.x(), tmin.y()}});
            points.push_back({{max.x(), max.y(), 0}, {tmax.x(), tmin.y()}});
          }
          vao.bind();
          vertices_buffer.init(points, BufferUsage::StaticDraw);
          // triangles_buffer.init(idxs, BufferUsage::StaticDraw);

          this->text = std::forward<S>(text);
        }

        virtual ~Text() {}

        void RenderRelative(const RenderSettings& settings,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;

          shader->Use();
          vao.bind();

          shader->Uniform("projection") = settings.projection;
          shader->Uniform("view") = settings.view;

          shader->Uniform("model") = transform;
          // shader.Uniform("color") = Color::white();
          shader->Uniform("tex") = font->GetAtlasTexture();

          // glDrawElements(GL_TRIANGLES, 6 * text.size(), GL_UNSIGNED_INT,
          // 0);
          glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());
        }
      };  // namespace shapes

    }  // namespace shapes
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_SHAPES_H
