#ifndef PLOT_LINE_H
#define PLOT_LINE_H

#include "data.h"
#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"

namespace emp {
  namespace plot {
    class Line : public scenegraph::Child {
      private:
      emp::opengl::shaders::SimpleVaryingColor shader;

      public:
      Line(emp::opengl::GLCanvas& canvas) : shader(canvas) {}

      template <typename D>
      void setData(const opengl::Camera& camera, const std::vector<D>& data) {
        using namespace properties;
        using namespace emp::math;
        using namespace emp::opengl;
        using namespace emp::opengl::shaders;

        if (data.size() <= 1) return;

        shader.shader.use();
        shader.vao.bind();
        shader.proj = camera.getProjection();
        shader.view = camera.getView();
        shader.model = Mat4x4f::translation(0, 0);

        auto& startData = data[0];
        Vec3f start{XYScaled::get(startData).x(), XYScaled::get(startData).y(),
                    0};
        auto startStroke{Stroke::get(startData)};
        auto startStrokeWeight{StrokeWeight::get(startData)};

        auto& middleData = data[1];
        Vec3f middle{XYScaled::get(middleData).x(),
                     XYScaled::get(middleData).y(), 0};
        Vec4f middleStroke{Stroke::get(middleData)};
        auto middleStrokeWeight{StrokeWeight::get(middleData)};

        auto segment = (middle - start).normalized();
        Vec3f normal{-segment.y(), segment.x(), 0};

        std::vector<SimpleVaryingColor::point_t> verts{
          {start + normal * startStrokeWeight, startStroke},
          {start - normal * startStrokeWeight, startStroke}};
        std::vector<GLuint> triangles;

        size_t i = 0;
        for (auto& value : data) {
          Vec3f end{XYScaled::get(value).x(), XYScaled::get(value).y(), 0};
          auto stroke{Stroke::get(value)};
          auto weight{StrokeWeight::get(value)};

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
