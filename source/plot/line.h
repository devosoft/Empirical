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
      void show(Iter begin, Iter end, const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view) {
        using namespace properties;
        using namespace emp::math;
        using namespace emp::opengl;
        using namespace emp::opengl::shaders;

        // Check that there are at least two points to draw
        if (begin == end) return;
        Vec3f start{ScaledX::get(*begin), ScaledY::get(*begin), 0};
        Vec4f startStroke{Stroke::get(*begin)};
        auto startStrokeWeight{StrokeWeight::get(*begin)};
        ++begin;
        if (begin == end) return;

        shader.shader.use();
        shader.vao.bind();
        shader.proj = projection;
        shader.view = view;
        shader.model = Mat4x4f::translation(0, 0);

        Vec3f middle{ScaledX::get(*begin), ScaledY::get(*begin), 0};
        Vec4f middleStroke{Stroke::get(*begin)};
        auto middleStrokeWeight{StrokeWeight::get(*begin)};
        ++begin;

        auto segment = (middle - start).normalized();
        Vec3f normal{-segment.y(), segment.x(), 0};

        std::vector<SimpleVaryingColor::point_t> verts{
          {start + normal * startStrokeWeight, startStroke},
          {start - normal * startStrokeWeight, startStroke}};
        std::vector<GLuint> triangles;

        size_t i = 0;
        for (auto iter = begin; iter != end; ++iter) {
          Vec3f end{ScaledX::get(*iter), ScaledY::get(*iter), 0};
          auto stroke{Stroke::get(*iter)};
          auto weight{StrokeWeight::get(*iter)};

          auto segment1 = (middle - start).normalized();
          Vec3f normal1{-segment1.y(), segment1.x(), 0};
          auto segment2 = (end - middle).normalized();
          Vec3f normal2{-segment2.y(), segment2.x(), 0};

          auto center{(normal1 + normal2).normalized() * middleStrokeWeight};

          verts.push_back(
            SimpleVaryingColor::point_t{middle + center, middleStroke});
          verts.push_back(
            SimpleVaryingColor::point_t{middle - center, middleStroke});

          triangles.push_back(i);
          triangles.push_back(i + 1);
          triangles.push_back(i + 2);

          triangles.push_back(i + 2);
          triangles.push_back(i + 3);
          triangles.push_back(i + 1);
          i += 2;

          start = std::move(middle);
          startStroke = std::move(middleStroke);
          startStrokeWeight = std::move(middleStrokeWeight);

          middle = std::move(end);
          middleStroke = std::move(stroke);
          middleStrokeWeight = std::move(weight);
        }

        segment = (middle - start).normalized();
        normal = {-segment.y(), segment.x(), 0};

        verts.push_back(SimpleVaryingColor::point_t{
          middle + normal * middleStrokeWeight, middleStroke});
        verts.push_back(SimpleVaryingColor::point_t{
          middle - normal * middleStrokeWeight, middleStroke});
        triangles.push_back(i);
        triangles.push_back(i + 1);
        triangles.push_back(i + 2);

        triangles.push_back(i + 2);
        triangles.push_back(i + 3);
        triangles.push_back(i + 1);

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
