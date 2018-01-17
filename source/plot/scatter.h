#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include "data.h"
#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"

namespace emp {
  namespace plot {

    DEFINE_PROPERTY(PointSize, pointSize);
    DEFINE_PROPERTY(XYScaled, xyScaled);
    DEFINE_PROPERTY(Fill, fill);

    class Scatter : public scenegraph::Child {
      private:
      emp::opengl::shaders::SimpleSolidColor shader;

      public:
      Scatter(emp::opengl::GLCanvas& canvas) : shader(canvas) {
        using namespace emp::opengl;
        using namespace emp::math;

        shader.vao.getBuffer<BufferType::Array>().set(
          {Vec3f{-1, +1, 0}, Vec3f{+1, +1, 0}, Vec3f{+1, -1, 0},
           Vec3f{-1, -1, 0}},
          BufferUsage::StaticDraw);
        shader.vao.getBuffer<BufferType::ElementArray>().set(
          {
            0, 1, 2,  // First Triangle
            2, 3, 0  // Second Triangle
          },
          BufferUsage::StaticDraw);
      }

      template <typename D>
      void show(const opengl::Camera& camera, const std::vector<D>& data) {
        using namespace emp::math;

        shader.shader.use();
        shader.vao.bind();

        shader.proj = camera.getProjection();
        shader.view = camera.getView();

        for (auto& pt : data) {
          auto model = Mat4x4f::translation(XYScaled::get(pt).x(),
                                            XYScaled::get(pt).y(), 0) *
                       Mat4x4f::scale(PointSize::get(pt));

          shader.model = model;
          shader.color = Fill::get(pt);

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
