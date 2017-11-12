#ifndef PLOT_LINE_H
#define PLOT_LINE_H

#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "properties.h"

namespace emp {
  namespace plot {
    class Line {
      private:
      emp::opengl::shaders::SimpleSolidColor shader;

      public:
      Line(emp::opengl::GLCanvas& canvas) : shader(canvas) {}

      template <typename Iter>
      void show(const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view, Iter begin, Iter end) {
        using namespace properties;
        using namespace emp::math;
        using namespace emp::opengl;

        if (begin == end) return;
        Vec2f last{X::get(*begin), Y::get(*begin)};
        ++begin;
        if (begin == end) return;

        shader.shader.use();
        shader.vao.bind();
        shader.proj = projection;
        shader.view = view;
        shader.color = Vec4f{1, 1, 0, 1};
        shader.model = Mat4x4f::translation(0, 0);

        std::vector<Vec3f> verts{{last.x(), last.y() + 2, 0},
                                 {last.x(), last.y() - 2, 0}};
        std::vector<GLuint> triangles;
        size_t i = 0;
        for (auto iter = begin; iter != end; ++iter) {
          verts.emplace_back(X::get(*iter), Y::get(*iter) + 2, 0);
          verts.emplace_back(X::get(*iter), Y::get(*iter) - 2, 0);

          triangles.push_back(i);
          triangles.push_back(i + 1);
          triangles.push_back(i + 2);

          triangles.push_back(i + 2);
          triangles.push_back(i + 3);
          triangles.push_back(i + 1);
          i += 2;
        }

        shader.vao.getBuffer<BufferType::Array>().set(verts,
                                                      BufferUsage::DynamicDraw);
        shader.vao.getBuffer<BufferType::ElementArray>().set(
          triangles, BufferUsage::DynamicDraw);

        glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);
      }
    };
  }  // namespace plot
}  // namespace emp

#endif  // PLOT_LINE_H
