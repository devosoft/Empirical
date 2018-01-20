#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include "data.h"
#include "math/LinAlg.h"
#include "opengl/color.h"
#include "opengl/defaultShaders.h"
#include "properties.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"

namespace emp {
  namespace plot {

    class Scatter : public scenegraph::Child {
      private:
      emp::opengl::shaders::SimpleSolidColor shader;
      std::vector<std::tuple<math::Mat4x4f, opengl::Color>> points;

      public:
      Scatter(emp::opengl::GLCanvas& canvas) : shader(canvas) {
        using namespace emp::opengl;
        using namespace emp::math;

        shader.vao.getBuffer<BufferType::Array>().set(
          {Vec3f{-0.5, +0.5, 0}, Vec3f{+0.5, +0.5, 0}, Vec3f{+0.5, -0.5, 0},
           Vec3f{-0.5, -0.5, 0}},
          BufferUsage::StaticDraw);
        shader.vao.getBuffer<BufferType::ElementArray>().set(
          {
            0, 1, 2,  // First Triangle
            2, 3, 0  // Second Triangle
          },
          BufferUsage::StaticDraw);
      }

      void renderRelative(const scenegraph::Camera& camera,
                          const math::Mat4x4f& transform) {
        using namespace emp::math;
        using namespace emp::opengl;

        shader.shader.use();
        shader.vao.bind();

        shader.proj = camera.getProjection();
        shader.view = camera.getView();

        for (auto& pt : points) {
          Mat4x4f model;
          std::tie(model, shader.color) = pt;
          shader.model = model * transform;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      }

      template <typename D>
      void setData(const std::vector<D>& data) {
        using namespace emp::math;
        points.clear();
        points.reserve(data.size());

        for (auto& pt : data) {
          auto model =
            Mat4x4f::translation(pt.xyScaled.x(), pt.xyScaled.y(), 0) *
            Mat4x4f::scale(pt.pointSize);

          points.push_back({model, pt.fill});
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
