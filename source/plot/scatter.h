#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include <cmath>
#include "attrs.h"
#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/color.h"
#include "opengl/defaultShaders.h"
#include "scales.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"

namespace emp {
  namespace plot {

    class Scatter : public scenegraph::Child {
      private:
      emp::opengl::shaders::SimpleSolidColor shader;
      size_t trianglesCount;
      std::vector<std::tuple<math::Mat4x4f, opengl::Color>> points;

      public:
      Scatter(emp::opengl::GLCanvas& canvas, size_t vertexCount = 4)
        : shader(canvas) {
        using namespace emp::opengl;
        using namespace emp::math;
        using namespace emp::plot::attrs;

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

        shader.vao.getBuffer<BufferType::Array>().set(vertices,
                                                      BufferUsage::StaticDraw);
        shader.vao.getBuffer<BufferType::ElementArray>().set(
          triangles, BufferUsage::StaticDraw);
      }
      virtual ~Scatter() {}

      void renderRelative(const scenegraph::Camera& camera,
                          const math::Mat4x4f& transform) {
        using namespace emp::math;
        using namespace emp::opengl;

        shader.shader.use();
        shader.vao.bind();
        shader.vao.getBuffer<BufferType::Array>().bind();
        shader.vao.getBuffer<BufferType::ElementArray>().bind();

        shader.proj = camera.getProjection();
        shader.view = camera.getView();

        for (auto& pt : points) {
          Mat4x4f model;
          std::tie(model, shader.color) = pt;
          shader.model = transform * model;

          glDrawElements(GL_TRIANGLES, trianglesCount, GL_UNSIGNED_INT, 0);
        }
      }

      template <class DataIter, class Iter>
      void apply(DataIter, DataIter, Iter begin, Iter end) {
        using namespace emp::math;
        points.clear();

        for (; begin != end; ++begin) {
          auto model =
            Mat4x4f::translation(begin->xyScaled.x(), begin->xyScaled.y(), 0) *
            Mat4x4f::scale(begin->pointSize);

          points.push_back({model, begin->fill});
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
