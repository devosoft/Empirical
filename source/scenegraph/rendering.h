#ifndef EMP_SCENEGRAPH_RENDERING_H
#define EMP_SCENEGRAPH_RENDERING_H

#include "math/consts.h"
#include "math/region.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scenegraph/camera.h"
#include "scenegraph/freetype.h"
#include "tools/attrs.h"
#include "tools/resources.h"

#include <memory>
#include <vector>

namespace emp {
  namespace graphics {
    struct RenderSettings {
      math::Mat4x4f projection;
      math::Mat4x4f view;
    };

    enum class TextDirections { Horizontal, Vertical };

    DEFINE_ATTR(Transform);
    DEFINE_ATTR(Fill);
    DEFINE_ATTR(Text);
    DEFINE_ATTR(TextSize);
    DEFINE_ATTR(TextDirection);

    struct Face {
      int a, b, c;
    };

    struct Mesh {
      std::vector<emp::math::Vec3f> verticies;
      std::vector<Face> faces;

      static Mesh Region(const emp::math::Region2f& region) {
        emp::math::Vec3f bottom_left{region.min.x(), region.min.y(), 0};
        emp::math::Vec3f bottom_right{region.max.x(), region.min.y(), 0};
        emp::math::Vec3f upper_right{region.max.x(), region.max.y(), 0};
        emp::math::Vec3f upper_left{region.min.x(), region.max.y(), 0};

        return {{bottom_left, bottom_right, upper_right, upper_left},
                {{0, 3, 1}, {3, 2, 1}}};
      }
      static Mesh Polygon(size_t vertex_count,
                          const emp::math::Vec2f& radius = {0.5, 0.5}) {
        Mesh mesh;
        int first = 0;
        for (auto i = 0; i < vertex_count; ++i) {
          auto r =
            static_cast<float>(i) / vertex_count * 2 * math::consts::pi<float> +
            math::consts::pi<float> / vertex_count;

          int num = mesh.verticies.size();
          mesh.verticies.emplace_back(cos(r) * radius.x() * 2,
                                      sin(r) * radius.y() * 2, 0);

          if (i == 0) first = num;

          auto next = num + 1;
          if (i == vertex_count - 1) {
            next = first;
          }

          mesh.faces.push_back({first, num, next});
        }

        return mesh;
      }
    };

    class FillRenderer {
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
        tools::Attrs<TransformValue<math::Mat4x4f>, FillValue<opengl::Color>>;

      private:
      std::vector<instance_attributes_type> draw_queue;

      public:
      template <typename S = const char*>
      FillRenderer(opengl::GLCanvas& canvas,
                   S&& fill_shader = "DefaultSolidColor")
        : fill_shader(std::forward<S>(fill_shader)),
          vao(canvas.MakeVAO()),
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

      void BeginBatch(const RenderSettings& settings, const Mesh& mesh) {
        auto first = 0;

        gpu_elements_buffer.Clear();
        gpu_vertex_buffer.Clear();

        for (auto& vertex : mesh.verticies) {
          gpu_vertex_buffer.EmplaceData(vertex);
        }

        for (auto& face : mesh.faces) {
          gpu_elements_buffer.PushData(face.a);
          gpu_elements_buffer.PushData(face.b);
          gpu_elements_buffer.PushData(face.c);
        }

        vao.bind();
        gpu_vertex_buffer.SendToGPU();
        gpu_elements_buffer.SendToGPU();

        fill_shader->Use();
        fill_shader_uniforms.projection = settings.projection;
        fill_shader_uniforms.view = settings.view;
      }

      template <typename I = instance_attributes_type>
      void Instance(I&& attrs) {
        draw_queue.emplace_back(std::forward<I>(attrs));
      }

      template <typename I = instance_attributes_type>
      void Instance(I&& attrs, float width, float height) {
        Transform::Get(attrs) *= emp::math::Mat4x4f::Scale(width, height, 1);

        draw_queue.emplace_back(std::forward<I>(attrs));
      }

      void FinishBatch() {
        // #ifndef EMSCRIPTEN
        //         if (draw_queue.size() > 10000) {
        //           return;
        //         }
        // #endif

        fill_shader->Use();
        vao.bind();

        for (auto& attrs : draw_queue) {
          fill_shader_uniforms.model = attrs.GetTransform();
          fill_shader_uniforms.fill = attrs.GetFill();

          gpu_elements_buffer.Draw(GL_TRIANGLES);
        }
        draw_queue.clear();
      }
    };

    class TextRenderer {
      struct data_t {
        math::Vec3f position;
        math::Vec2f texture_coordinates;
      };

      opengl::VertexArrayObject vao;
      opengl::BufferVector<opengl::BufferType::Array, data_t> vertices_buffer;
      ResourceRef<scenegraph::FontFace> font;
      ResourceRef<opengl::ShaderProgram> shader;

      struct {
        opengl::Uniform model;
        opengl::Uniform view;
        opengl::Uniform projection;
        opengl::Uniform tex;
        opengl::Uniform fill;
      } shader_uniforms;

      public:
      using instance_attributes_type =
        tools::Attrs<TransformValue<math::Mat4x4f>, FillValue<opengl::Color>,
                     TextValue<std::string>, TextSizeValue<float>>;

      template <typename F, typename S = std::string>
      TextRenderer(opengl::GLCanvas& canvas, F&& font,
                   S&& shader = "DefaultFont")
        : vao(canvas.MakeVAO()),
          vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          font(std::forward<F>(font)),
          shader(std::forward<S>(shader)) {
        using namespace emp::opengl;
        using namespace emp::math;

        this->shader.OnSet([this](auto& value) {
          shader_uniforms.model = this->shader->Uniform("model");
          shader_uniforms.view = this->shader->Uniform("view");
          shader_uniforms.projection = this->shader->Uniform("projection");
          shader_uniforms.tex = this->shader->Uniform("tex");
          shader_uniforms.fill = this->shader->Uniform("fill");

          vao.bind();
          vertices_buffer.bind();
          vao.attr(this->shader->Attribute("position", &data_t::position));
          vao.attr(this->shader->Attribute("uv", &data_t::texture_coordinates));
        });
      }

      void BeginBatch(const RenderSettings& settings) {
        this->shader->Use();
        shader_uniforms.projection = settings.projection;
        shader_uniforms.view = settings.view;
      }

      emp::math::Vec2f Measure(
        const std::string& text, float text_size,
        TextDirections direction = TextDirections::Horizontal) const {
        float scale = text_size / font->atlas_height;
        emp::math::Vec2f cursor{0, 0};

        for (auto& c : text) {
          auto info = font->Lookup(c);

          switch (direction) {
              // Advance the cursor by the size of the character
            case TextDirections::Horizontal:
              cursor.x() += info.cursor_advance.x() * scale;
              cursor.y() = std::max(cursor.y(), info.bitmap_size.y() * scale);
              break;
            case TextDirections::Vertical:
              cursor.x() = std::max(cursor.x(), info.bitmap_size.x() * scale);
              cursor.y() += info.cursor_advance.y() * scale;
              break;
          }
        }

        return cursor;
      }

      void Instance(const instance_attributes_type& attrs) {
        using namespace emp::opengl;
        using namespace emp::math;

        Vec2f cursor{0, 0};
        vertices_buffer.Clear();

        float scale = TextSize::Get(attrs) / font->atlas_height;

        int i = 0;
        for (auto& c : attrs.GetText()) {
          auto info = font->Lookup(c);
          auto lcursor = cursor;
          // Calculate the start of the next character
          cursor = cursor + Vec2f{info.cursor_advance.x() * scale,
                                  info.cursor_advance.y() * scale};

          // Skip characters who have no size, such as spaces
          if (info.bitmap_size.x() <= 0 || info.bitmap_size.y() <= 0) continue;

          // See https://www.freetype.org/freetype2/docs/tutorial/step2.html for
          // what bearing is. Basically, it is the position of this character
          // relative to the last
          auto max =
            lcursor + Vec2f{info.bearing.x() * scale, info.bearing.y() * scale};
          auto min = max - Vec2f{info.bitmap_size.x() * scale,
                                 info.bitmap_size.y() * scale};

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
        shader->Use();
        vao.bind();
        vertices_buffer.SendToGPU();

        shader_uniforms.model = attrs.GetTransform();
        shader_uniforms.tex = *font->ComputeAtlasTexture();
        shader_uniforms.fill = attrs.GetFill();

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
      Pen(R* renderer, const RenderSettings& settings, T&&... args)
        : renderer(renderer) {
        renderer->BeginBatch(settings, std::forward<T>(args)...);
      }

      template <typename I, typename... U>
      Pen& Data(I begin, I end, const tools::Attrs<U...>& transform) {
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

      void Flush() { renderer->FinishBatch(); }
    };

    class Graphics {
      FillRenderer fill_renderer;
      TextRenderer text_renderer;

      public:
      std::shared_ptr<scenegraph::Camera> camera;
      std::shared_ptr<scenegraph::Eye> eye;

      template <typename F>
      Graphics(opengl::GLCanvas& canvas, F&& font,
               std::shared_ptr<scenegraph::Camera> camera,
               std::shared_ptr<scenegraph::Eye> eye)
        : fill_renderer(canvas),
          text_renderer(canvas, std::forward<F>(font)),
          camera(camera),
          eye(eye) {}

      Graphics(const Graphics&) = delete;
      Graphics(Graphics&&) = delete;

      Graphics& operator=(const Graphics&) = delete;
      Graphics& operator=(Graphics&&) = delete;

      auto Measure(const std::string& text, float text_size) const {
        return text_renderer.Measure(text, text_size);
      }

      void Clear(float r, float g, float b, float a = 1) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }

      void Clear(const opengl::Color& color) {
        Clear(color.r, color.g, color.b, color.a);
      }

      Pen<FillRenderer> Fill(const Mesh& mesh) {
        return {&fill_renderer,
                {camera->GetProjection(), eye->CalculateView()},
                mesh};
      }

      template <typename A0 = typename FillRenderer::instance_attributes_type,
                typename... A>
      void DrawFilled(const Mesh& mesh, A0&& attributes, A&&... args) {
        Fill(mesh)
          .Draw(std::forward<A0>(attributes), std::forward<A>(args)...)
          .Flush();
      }

      Pen<TextRenderer> Text() {
        return {&text_renderer,
                {camera->GetProjection(), eye->CalculateView()}};
      }
    };
  }  // namespace graphics
}  // namespace emp

#endif
