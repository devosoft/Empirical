#ifndef PLOT_LINE_H
#define PLOT_LINE_H

#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "properties.h"

namespace emp {
  namespace plot {
    class Line {
      private:
      emp::opengl::shaders::SimpleVaryingColor shader;

      public:
      Line(emp::opengl::GLCanvas& canvas) : shader(canvas) {}

      template <typename Iter>
      void show(const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view, Iter begin, Iter end) {
        using namespace properties;
        using namespace emp::math;
        using namespace emp::opengl;
        using namespace emp::opengl::shaders;

        // Check that there are at least two points to draw
        if (begin == end) return;
        Vec3f start{ScaledX::get(*begin), ScaledY::get(*begin), 0};
        Vec4f startColor{Stroke::get(*begin)};
        auto firstWeight{StrokeWeight::get(*begin)};
        ++begin;
        if (begin == end) return;

        shader.shader.use();
        shader.vao.bind();
        shader.proj = projection;
        shader.view = view;
        shader.model = Mat4x4f::translation(0, 0);

        Vec3f middle{ScaledX::get(*begin), ScaledY::get(*begin), 0};
        Vec4f middleColor{Stroke::get(*begin)};
        ++begin;

        auto segment = (middle - start).normalized();
        Vec3f normal{-segment.y(), segment.x(), 0};
        auto secondWeight{StrokeWeight::get(*begin)};

        std::vector<SimpleVaryingColor::point_t> verts{
          {start + normal * firstWeight, startColor},
          {start - normal * secondWeight, startColor},
          {middle + normal * secondWeight, middleColor},
          {middle - normal * secondWeight, middleColor}};
        std::vector<GLuint> triangles{0, 1, 2, 2, 3, 1};

        size_t i = 2;
        for (auto iter = begin; iter != end; ++iter) {
          Vec3f end{ScaledX::get(*iter), ScaledY::get(*iter), 0};
          Vec4f color{Stroke::get(*iter)};
          auto weight{StrokeWeight::get(*iter)};

          auto segment1 = (middle - start).normalized();
          Vec3f normal1{-segment1.y(), segment1.x(), 0};
          auto segment2 = (end - middle).normalized();
          Vec3f normal2{-segment2.y(), segment2.x(), 0};

          Vec3f center = (normal1 + normal2).normalized() * weight;

          verts.push_back(SimpleVaryingColor::point_t{middle + center, color});
          verts.push_back(SimpleVaryingColor::point_t{middle - center, color});

          triangles.push_back(i);
          triangles.push_back(i + 1);
          triangles.push_back(i + 2);

          triangles.push_back(i + 2);
          triangles.push_back(i + 3);
          triangles.push_back(i + 1);
          i += 2;

          start = middle;
          middle = end;
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
