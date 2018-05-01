#ifndef EMP_SCENEGRAPH_RENDERING_H
#define EMP_SCENEGRAPH_RENDERING_H

#include "math/consts.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scenegraph/core.h"
#include "tools/resources.h"

#include <vector>

namespace emp {
  namespace scenegraph {
    class FillPen;
    class Context;

    namespace __impl_render_cmds {

      struct Ellipse {
        math::Vec2f radius;
      };

      struct FillRegularPolygon {
        size_t vertex_count;
        math::Vec2f radius;
      };

      struct FillPolygon {
        std::vector<math::Vec3f> vertices;
        std::vector<int> triangles;
      };

      class FillCmd {
        private:
        union data_t {
          FillRegularPolygon regular_polygon;
          FillPolygon polygon;
          Ellipse ellipse;

          data_t() {}
          ~data_t() {}
        } data;
        enum class cmd_t { Ellipse, RegularPolygon, Polygon } cmd;

        public:
        math::Mat4x4f model;
        opengl::Color fill;

        FillCmd(const math::Mat4x4f& model, const opengl::Color& fill)
          : cmd(cmd_t::Ellipse), model(model), fill(fill) {}

        FillCmd(const FillRegularPolygon& fill_regular_polygon,
                const math::Mat4x4f& model, const opengl::Color& fill)
          : cmd(cmd_t::RegularPolygon), model(model), fill(fill) {
          new (&data.regular_polygon) struct FillRegularPolygon(
            fill_regular_polygon);
        }

        FillCmd(FillRegularPolygon&& fill_regular_polygon,
                const math::Mat4x4f& model, const opengl::Color& fill)
          : cmd(cmd_t::RegularPolygon), model(model), fill(fill) {
          new (&data.regular_polygon) struct FillRegularPolygon(
            std::move(fill_regular_polygon));
        }

        FillCmd(const FillPolygon& fill_polygon, const math::Mat4x4f& model,
                const opengl::Color& fill)
          : cmd(cmd_t::Polygon), model(model), fill(fill) {
          new (&data.polygon) struct FillPolygon(fill_polygon);
        }

        FillCmd(FillPolygon&& fill_polygon, const math::Mat4x4f& model,
                const opengl::Color& fill)
          : cmd(cmd_t::Polygon), model(model), fill(fill) {
          new (&data.polygon) struct FillPolygon(std::move(fill_polygon));
        }

        FillCmd(const Ellipse& ellipse, const math::Mat4x4f& model,
                const opengl::Color& fill)
          : cmd(cmd_t::Polygon), model(model), fill(fill) {
          new (&data.ellipse) struct Ellipse(ellipse);
        }

        FillCmd(Ellipse&& ellipse, const math::Mat4x4f& model,
                const opengl::Color& fill)
          : cmd(cmd_t::Polygon), model(model), fill(fill) {
          new (&data.ellipse) struct Ellipse(std::move(ellipse));
        }

        FillCmd(const FillCmd& other)
          : cmd(other.cmd), model(other.model), fill(other.fill) {
          switch (cmd) {
            case cmd_t::RegularPolygon:
              new (&data.regular_polygon) struct FillRegularPolygon(
                other.data.regular_polygon);
              break;
            case cmd_t::Polygon:
              new (&data.polygon) struct FillPolygon(other.data.polygon);
              break;
            case cmd_t::Ellipse:
              new (&data.ellipse) struct Ellipse(other.data.ellipse);
              break;
          }
        }

        FillCmd(FillCmd&& other)
          : cmd(other.cmd),
            model(std::move(other.model)),
            fill(std::move(other.fill)) {
          switch (cmd) {
            case cmd_t::RegularPolygon:
              new (&data.regular_polygon) struct FillRegularPolygon(
                std::move(other.data.regular_polygon));
              break;
            case cmd_t::Polygon:
              new (&data.polygon) struct FillPolygon(
                std::move(other.data.polygon));
              break;
            case cmd_t::Ellipse:
              new (&data.ellipse) struct Ellipse(std::move(other.data.ellipse));
              break;
          }
        }

        FillCmd& operator=(const FillCmd& other) {
          if (this != &other) {
            if (cmd == other.cmd) {
              switch (cmd) {
                case cmd_t::RegularPolygon:
                  data.regular_polygon = other.data.regular_polygon;
                  break;
                case cmd_t::Polygon:
                  data.polygon = other.data.polygon;
                  break;
                case cmd_t::Ellipse:
                  data.ellipse = other.data.ellipse;
                  break;
              }
            } else {
              Free();
              switch (cmd) {
                case cmd_t::RegularPolygon:
                  new (&data.regular_polygon) struct FillRegularPolygon(
                    other.data.regular_polygon);
                  break;
                case cmd_t::Polygon:
                  new (&data.polygon) struct FillPolygon(other.data.polygon);
                  break;
                case cmd_t::Ellipse:
                  new (&data.ellipse) struct Ellipse(other.data.ellipse);
                  break;
              }
            }

            cmd = other.cmd;
            model = other.model;
            fill = other.fill;
          }
          return *this;
        }

        FillCmd& operator=(FillCmd&& other) {
          if (this != &other) {
            if (cmd == other.cmd) {
              switch (cmd) {
                case cmd_t::RegularPolygon:
                  data.regular_polygon = std::move(other.data.regular_polygon);
                  break;
                case cmd_t::Polygon:
                  data.polygon = std::move(other.data.polygon);
                  break;
                case cmd_t::Ellipse:
                  data.ellipse = std::move(other.data.ellipse);
                  break;
              }
            } else {
              Free();
              switch (cmd) {
                case cmd_t::RegularPolygon:
                  new (&data.regular_polygon) struct FillRegularPolygon(
                    std::move(other.data.regular_polygon));
                  break;
                case cmd_t::Polygon:
                  new (&data.polygon) struct FillPolygon(
                    std::move(other.data.polygon));
                  break;
                case cmd_t::Ellipse:
                  new (&data.ellipse) struct Ellipse(
                    std::move(other.data.ellipse));
                  break;
              }
            }

            cmd = other.cmd;
            model = other.model;
            fill = other.fill;
          }
          return *this;
        }

        void Free() {
          std::cout << "FREE" << std::endl;
          switch (cmd) {
            case cmd_t::RegularPolygon:
              data.regular_polygon.~FillRegularPolygon();
              break;
            case cmd_t::Polygon:
              data.polygon.~FillPolygon();
              break;
            case cmd_t::Ellipse:
              data.ellipse.~Ellipse();
              break;
          }
        }

        ~FillCmd() { Free(); }

        bool IsEllipse() const { return cmd == cmd_t::Ellipse; }
        bool IsPolygon() const { return cmd == cmd_t::Polygon; }
        bool IsRegularPolygon() const { return cmd == cmd_t::RegularPolygon; }

        template <typename F1, typename F2, typename F3>
        void Apply(F1&& ellipse, F2&& polygon, F3&& regular_polygon) {
          switch (cmd) {
            case cmd_t::Ellipse:
              std::forward<F1>(ellipse)(data.ellipse);
              break;
            case cmd_t::Polygon:
              std::forward<F2>(polygon)(data.polygon);
              break;
            case cmd_t::RegularPolygon:
              std::forward<F3>(regular_polygon)(data.regular_polygon);
              break;
          }
        }

        template <typename F1, typename F2, typename F3>
        void Apply(F1&& ellipse, F2&& polygon, F3&& regular_polygon) const {
          switch (cmd) {
            case cmd_t::Ellipse:
              std::forward<F1>(ellipse)(data.ellipse);
              break;
            case cmd_t::Polygon:
              std::forward<F2>(polygon)(data.polygon);
              break;
            case cmd_t::RegularPolygon:
              std::forward<F3>(regular_polygon)(data.regular_polygon);
              break;
          }
        }

      };  // namespace __impl_render_cmds
    }  // namespace __impl_render_cmds

    class Graphics {
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

      friend Context;
      friend FillPen;

      void GenerateRegularPolygon(size_t vertex_count,
                                  const math::Vec2f& radius) {
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
      }

      public:
      template <typename S = const char*>
      Graphics(opengl::GLCanvas& canvas, S&& fill_shader = "DefaultSolidColor")
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

      template <typename Iter>
      auto RenderBatch(const RenderSettings& settings, Iter begin, Iter end) ->
        typename std::enable_if<
          std::is_convertible<decltype(*begin),
                              __impl_render_cmds::FillCmd>::value,
          void>::type {
        for (; begin != end; ++begin) {
          begin->Apply(
            [this](const __impl_render_cmds::Ellipse& ellipse) {
              GenerateRegularPolygon(64, ellipse.radius);
            },
            [this](const __impl_render_cmds::FillPolygon& polygon) {
              gpu_vertex_buffer.PushAll(polygon.vertices);
              gpu_elements_buffer.PushAll(polygon.triangles);
            },
            [this](
              const __impl_render_cmds::FillRegularPolygon& regular_polygon) {
              GenerateRegularPolygon(regular_polygon.vertex_count,
                                     regular_polygon.radius);
            });

          gpu_vertex_buffer.SendToGPU();
          gpu_elements_buffer.SendToGPU();
          fill_shader->Use();

          fill_shader_uniforms.projection = settings.projection;
          fill_shader_uniforms.view = settings.view;
          fill_shader_uniforms.model = begin->model;
          fill_shader_uniforms.fill = begin->fill;

          gpu_elements_buffer.Draw(GL_TRIANGLES);
        }
      }

      Context Context(RenderSettings settings);
    };

    class Context {
      private:
      Graphics* g;
      RenderSettings settings;

      friend FillPen;

      template <typename Iter>
      void RenderBatch(Iter begin, Iter end) {
        g->RenderBatch(settings, begin, end);
      }

      public:
      Context(Graphics* g, RenderSettings settings)
        : g(g), settings(settings) {}

      Context(const Context& other) : g(other.g), settings(other.settings) {}
      Context(Context&& other)
        : g(other.g), settings(std::move(other.settings)) {}

      Context& operator=(const Context& other) {
        g = other.g;
        settings = other.settings;
        return *this;
      }

      Context& operator=(Context&& other) {
        g = other.g;
        settings = std::move(other.settings);
        return *this;
      }

      template <typename F>
      void Fill(F&& callback);
    };

    class FillPen {
      private:
      Context* ctx;
      friend Context;
      math::Mat4x4f model_transform;

      std::vector<__impl_render_cmds::FillCmd> cmds;

      FillPen(Context* ctx)
        : ctx(ctx), model_transform(math::Mat4x4f::Identity()) {}

      public:
      FillPen(const FillPen& other) = delete;
      FillPen(FillPen&& other)
        : ctx(other.ctx),
          model_transform(std::move(other.model_transform)),
          cmds(std::move(other.cmds)) {
        other.ctx = nullptr;
      }

      FillPen& operator=(const FillPen&) = delete;
      FillPen& operator=(FillPen&& other) {
        if (this != &other) {
          ctx = other.ctx;
          other.ctx = nullptr;

          model_transform = std::move(other.model_transform);
          cmds = std::move(other.cmds);
        }
        return *this;
      };

      ~FillPen() {
        if (ctx != nullptr || !cmds.empty()) {
          ctx->RenderBatch(cmds.begin(), cmds.end());
          cmds.clear();
        }
      }

      template <typename U0 = math::Vec3f, typename... U>
      FillPen& Move(U0&& arg0, U&&... args) {
        model_transform =
          model_transform * math::Mat4x4f::Translation(
                              std::forward<U0>(arg0), std::forward<U>(args)...);
        return *this;
      }

      template <typename U0 = math::Vec3f, typename... U>
      FillPen& Scale(U0&& arg0, U&&... args) {
        model_transform *= math::Mat4x4f::Scale(std::forward<U0>(arg0),
                                                std::forward<U>(args)...);
        return *this;
      }

      FillPen& Reset() {
        model_transform = math::Mat4x4f::Identity();
        return *this;
      }

      FillPen& Rect(const math::Vec2f& size, const opengl::Color& fill) {
        cmds.emplace_back(__impl_render_cmds::FillRegularPolygon{4, size},
                          model_transform, fill);
        return *this;
      }

      FillPen& Ellipse(const math::Vec2f& radius, const opengl::Color& fill) {
        cmds.emplace_back(__impl_render_cmds::Ellipse{radius}, model_transform,
                          fill);
        return *this;
      }
    };

    template <typename F>
    void Context::Fill(F&& callback) {
      auto pen = FillPen{this};
      std::forward<F>(callback)(pen);
    }

    Context Graphics::Context(RenderSettings settings) {
      return {this, settings};
    }

  }  // namespace scenegraph
}  // namespace emp

#endif
