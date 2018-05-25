#ifndef EMP_SCENEGRAPH_RENDERING_H
#define EMP_SCENEGRAPH_RENDERING_H

#include "math/consts.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scenegraph/core.h"
#include "scenegraph/freetype.h"
#include "tools/attrs.h"
#include "tools/resources.h"

#include <vector>

namespace emp {
  namespace graphics {

    DEFINE_ATTR(Transform);
    DEFINE_ATTR(Color);
    DEFINE_ATTR(Text);

    class FillRegularPolygonRenderer {
      ResourceRef<opengl::ShaderProgram> fill_shader;

      opengl::VertexArrayObject vao;

      opengl::BufferVector<opengl::BufferType::Array, math::Vec3f>
        gpu_vertex_buffer;
      opengl::BufferVector<opengl::BufferType::ElementArray, int>
        gpu_elements_buffer;

      struct {
        opengl::Uniform model;
        opengl::Uniform view;
        opengl::Uniform projection;
        opengl::Uniform fill;
      } fill_shader_uniforms;

      public:
      using instance_attributes_type =
        tools::Attrs<TransformValue<math::Mat4x4f>, ColorValue<opengl::Color>>;

      template <typename S = const char*>
      FillRegularPolygonRenderer(opengl::GLCanvas& canvas,
                                 S&& fill_shader = "DefaultSolidColor")
        : fill_shader(std::forward<S>(fill_shader)),
          gpu_vertex_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          gpu_elements_buffer(
            canvas.makeBuffer<opengl::BufferType::ElementArray>()) {
        this->fill_shader.OnSet([this](auto&) {
          fill_shader_uniforms.model = this->fill_shader->Uniform("model");
          fill_shader_uniforms.view = this->fill_shader->Uniform("view");
          fill_shader_uniforms.projection =
            this->fill_shader->Uniform("projection");
          fill_shader_uniforms.fill = this->fill_shader->Uniform("fill");

          vao.bind();
          gpu_vertex_buffer.bind();
          gpu_elements_buffer.bind();
          vao.attr(this->fill_shader->Attribute<math::Vec3f>("position"));
        });
      }

      void BeginBatch(const scenegraph::RenderSettings& settings,
                      size_t vertex_count, const math::Vec2f& radius) {
        auto first = 0;

        for (auto i = 0; i < vertex_count; ++i) {
          auto r =
            static_cast<float>(i) / vertex_count * 2 * math::consts::pi<float> +
            math::consts::pi<float> / vertex_count;

          auto num = gpu_vertex_buffer.Size();
          gpu_vertex_buffer.EmplaceData(cos(r) * radius.x(),
                                        sin(r) * radius.y(), 0);

          if (i == 0) first = num;

          auto next = num + 1;
          if (i == vertex_count - 1) {
            next = first;
          }

          gpu_elements_buffer.PushData(first);
          gpu_elements_buffer.PushData(num);
          gpu_elements_buffer.PushData(next);
        }
        gpu_vertex_buffer.SendToGPU();
        gpu_elements_buffer.SendToGPU();
        fill_shader_uniforms.projection = settings.projection;
        fill_shader_uniforms.view = settings.view;
      }

      void Instance(const instance_attributes_type& attrs) {
        fill_shader->Use();

        fill_shader_uniforms.model = attrs.GetTransform();
        fill_shader_uniforms.fill = attrs.GetColor();

        gpu_elements_buffer.Draw(GL_TRIANGLES);
      }

      void FinishBatch() {}
    };

    class TextRenderer {
      struct data_t {
        math::Vec3f position;
        math::Vec2f texture_coordinates;
      };

      std::string font_name;
      std::string text;
      float height = 8;
      opengl::VertexArrayObject vao;
      opengl::BufferVector<opengl::BufferType::Array, data_t> vertices_buffer;
      ResourceRef<scenegraph::FontFace> font;
      ResourceRef<opengl::ShaderProgram> shader;

      struct {
        opengl::Uniform model;
        opengl::Uniform view;
        opengl::Uniform projection;
        opengl::Uniform tex;
      } shader_uniforms;

      public:
      using instance_attributes_type =
        tools::Attrs<TransformValue<math::Mat4x4f>, ColorValue<opengl::Color>,
                     TextValue<std::string>>;

      template <typename F, typename S = std::string>
      TextRenderer(opengl::GLCanvas& canvas, F&& font,
                   S&& shader = "DefaultFont")
        : vao(canvas.MakeVAO()),
          vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          font(std::forward<F>(font)),
          shader(std::forward<S>(shader)) {
        using namespace emp::opengl;
        using namespace emp::math;

        // SetText(text);

        this->shader.OnSet([this](auto& value) {
          shader_uniforms.model = this->shader->Uniform("model");
          shader_uniforms.view = this->shader->Uniform("view");
          shader_uniforms.projection = this->shader->Uniform("projection");
          shader_uniforms.tex = this->shader->Uniform("tex");

          vao.bind();
          vertices_buffer.bind();
          vao.attr(this->shader->Attribute("position", &data_t::position));
          vao.attr(this->shader->Attribute("uv", &data_t::texture_coordinates));
        });
      }

      void BeginBatch(const scenegraph::RenderSettings& settings) {
        shader_uniforms.projection = settings.projection;
        shader_uniforms.view = settings.view;
      }

      void Instance(const instance_attributes_type& attrs) {
        using namespace emp::opengl;
        using namespace emp::math;

        // std::vector<data_t> points;
        Vec2f cursor{0, 0};

        float scale = height / font->atlas_height;

        int i = 0;
        for (auto& c : attrs.GetText()) {
          auto info = font->Lookup(c);
          auto lcursor = cursor;
          cursor = cursor + Vec2f{info.cursor_advance.x() * scale,
                                  info.cursor_advance.y() * scale};

          if (info.size.x() <= 0 || info.size.y() <= 0) continue;

          auto max =
            lcursor + Vec2f{info.bearing.x() * scale, info.bearing.y() * scale};
          auto min = max - Vec2f{info.size.x() * scale, info.size.y() * scale};

          auto tmin = info.texture_region.min;
          auto tmax = info.texture_region.max;

          vertices_buffer.PushData(
            {{min.x(), min.y(), 0}, {tmin.x(), tmax.y()}});
          vertices_buffer.PushData(
            {{max.x(), min.y(), 0}, {tmax.x(), tmax.y()}});

          vertices_buffer.PushData(
            {{min.x(), max.y(), 0}, {tmin.x(), tmin.y()}});
          vertices_buffer.PushData(
            {{max.x(), min.y(), 0}, {tmax.x(), tmax.y()}});

          vertices_buffer.PushData(
            {{min.x(), max.y(), 0}, {tmin.x(), tmin.y()}});
          vertices_buffer.PushData(
            {{max.x(), max.y(), 0}, {tmax.x(), tmin.y()}});
        }
        vao.bind();
        vertices_buffer.SendToGPU();

        shader->Use();

        shader_uniforms.model = attrs.GetTransform();
        shader_uniforms.tex = font->GetAtlasTexture();
        // fill_shader_uniforms.fill = attrs.GetColor();

        vertices_buffer.Draw(GL_TRIANGLES);
      }

      void FinishBatch() {}
    };

    template <typename R>
    class Pen {
      private:
      R* renderer;

      public:
      using instance_attributes_type = typename R::instance_attributes_type;

      template <typename... T>
      Pen(R* renderer, const scenegraph::RenderSettings& settings, T&&... args)
        : renderer(renderer) {
        renderer->BeginBatch(settings, std::forward<T>(args)...);
      }

      Pen(const Pen&) = delete;
      Pen(Pen&&) = delete;

      Pen& operator=(const Pen&) = delete;
      Pen& operator=(Pen&&) = delete;

      template <typename I, typename U>
      Pen& Data(I begin, I end, const U& transform) {
        for (; begin != end; ++begin) {
          Draw(transform(*begin));
        }
        return *this;
      }

      template <typename T0 = instance_attributes_type, typename... T>
      Pen& Draw(T0&& args0, T&&... args) {
        renderer->Instance({std::forward<T0>(args0), std::forward<T>(args)...});
        return *this;
      }

      ~Pen() { renderer->FinishBatch(); }
    };

    class Graphics {
      FillRegularPolygonRenderer fill_regular_polygon_renderer;
      TextRenderer text_renderer;

      public:
      template <typename F>
      Graphics(opengl::GLCanvas& canvas, F&& font)
        : fill_regular_polygon_renderer(canvas),
          text_renderer(canvas, std::forward<F>(font)) {}

      Graphics(const Graphics&) = delete;
      Graphics(Graphics&&) = delete;

      Graphics& operator=(const Graphics&) = delete;
      Graphics& operator=(Graphics&&) = delete;

      Pen<FillRegularPolygonRenderer> FillRegularPolygons(
        const scenegraph::RenderSettings& settings, size_t vertex_count,
        const math::Vec2f& radius) {
        return {&fill_regular_polygon_renderer, settings, vertex_count, radius};
      }

      Pen<TextRenderer> Text(const scenegraph::RenderSettings& settings) {
        return {&text_renderer, settings};
      }
    };
  }  // namespace graphics
}  // namespace emp

#endif
