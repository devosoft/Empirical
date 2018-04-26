#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include <cmath>
#include "flow.h"
#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scales.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"

#include "tools/resources.h"

namespace emp {
  namespace plot {
    class Scatter : public scenegraph::Child, public Joinable<Scatter> {
      private:
      ResourceRef<opengl::ShaderProgram> shader;
      opengl::VertexArrayObject vao;
      opengl::BufferObject<opengl::BufferType::Array> verticesBuffer;
      opengl::BufferObject<opengl::BufferType::ElementArray> trianglesBuffer;

      size_t trianglesCount;
      std::vector<std::tuple<math::Mat4x4f, opengl::Color>> points;

      // opengl::Uniform color;
      // opengl::Uniform model;
      // opengl::Uniform projection;
      // opengl::Uniform view;

      public:
      template <typename S = std::string>
      Scatter(emp::opengl::GLCanvas& canvas, size_t vertexCount = 4,
              S&& shader = "DefaultSolidColor")
        : shader(std::forward<S>(shader)),
          verticesBuffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          trianglesBuffer(
            canvas.makeBuffer<opengl::BufferType::ElementArray>()) {
        using namespace emp::opengl;
        using namespace emp::math;

        std::vector<Vec3f> vertices;
        vertices.reserve(vertexCount + 1);
        std::vector<int> triangles;
        trianglesCount = vertexCount * 3;
        triangles.reserve(trianglesCount);

        vertices.emplace_back(0, 0, 0);

        for (auto i = 0; i < vertexCount; ++i) {
          auto r = static_cast<float>(i) / vertexCount * 2 * consts::pi<float>;

          vertices.emplace_back(cos(r) * 0.5f, sin(r) * 0.5f, 0);

          triangles.push_back(0);
          triangles.push_back(i + 1);
          triangles.push_back(((i + 1) % vertexCount) + 1);
        }
        verticesBuffer.init(vertices, BufferUsage::StaticDraw);
        trianglesBuffer.init(triangles, BufferUsage::StaticDraw);

        this->shader.OnSet([this](auto&) {
          vao.bind();
          verticesBuffer.bind();
          trianglesBuffer.bind();
          vao.attr(this->shader->Attribute<Vec3f>("position"));
        });
      }

      virtual ~Scatter() {}

      void RenderRelative(const scenegraph::RenderSettings& settings,
                          const math::Mat4x4f& transform) {
        using namespace emp::math;
        using namespace emp::opengl;

        shader->Use();
        vao.bind();

        shader->Uniform("projection") = settings.projection;
        shader->Uniform("view") = settings.view;

        for (auto& pt : points) {
          shader->Uniform("model") = transform * std::get<0>(pt);
          shader->Uniform("color") = std::get<1>(pt);

          glDrawElements(GL_TRIANGLES, trianglesCount, GL_UNSIGNED_INT, 0);
        }
      }

      template <class DataIter, class Iter>
      void Apply(DataIter, DataIter, Iter begin, Iter end) {
        using namespace emp::math;
        using namespace emp::plot::attributes;
        points.clear();

        for (; begin != end; ++begin) {
          auto model = Mat4x4f::Translation(XyzScaled::Get(*begin).x(),
                                            XyzScaled::Get(*begin).y(), 0) *
                       Mat4x4f::Scale(PointSize::Get(*begin));

          points.push_back({model, Fill::Get(*begin)});
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
