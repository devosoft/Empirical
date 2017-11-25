#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "properties.h"

namespace emp {
  namespace plot {

    struct PointSize : properties::PropertyName<PointSize> {};

    class Scatter {
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
            2, 3, 0   // Second Triangle
          },
          BufferUsage::StaticDraw);
      }

      template <typename Iter>
      void show(Iter begin, Iter end, const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view) {
        using namespace properties;
        using namespace emp::math;

        shader.shader.use();
        shader.vao.bind();

        shader.proj = projection;
        shader.view = view;

        for (auto iter = begin; iter != end; ++iter) {
          auto model =
            Mat4x4f::translation(ScaledX::get(*iter), ScaledY::get(*iter)) *
            Mat4x4f::scale(PointSize::get(*iter));
          shader.model = model;
          shader.color = Fill::get(*iter);

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
