#ifndef PLOT_LINE_H
#define PLOT_LINE_H

#include "flow.h"
#include "math/LinAlg.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scales.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"
#include "tools/resources.h"

// Based on
// https://blog.mapbox.com/drawing-antialiased-lines-with-opengl-8766f34192dc

namespace emp {
  namespace plot {
    class Line : public scenegraph::Child, public Joinable<Line> {
      private:
      ResourceRef<opengl::ShaderProgram> shader;

      opengl::Uniform model, view, projection;
      opengl::VertexArrayObject vao;
      struct point_t {
        math::Vec3f position;
        math::Vec2f normal;
        float weight;
        opengl::Color color;
      };

      opengl::BufferObject<opengl::BufferType::Array> verticiesBuffer;
      opengl::BufferObject<opengl::BufferType::ElementArray> trianglesBuffer;
      size_t maxElementCount = 0;

      size_t elementCount = 0;

      public:
      template <typename S = std::string>
      Line(opengl::GLCanvas& canvas, S&& shader = "DefaultVaryingColor")
        : shader(std::forward<S>(shader)),
          vao(canvas.MakeVAO()),
          verticiesBuffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          trianglesBuffer(
            canvas.makeBuffer<opengl::BufferType::ElementArray>()) {
        using namespace emp::opengl;

        this->shader.OnSet([this](auto& shader) {
          vao.bind();
          trianglesBuffer.bind();
          verticiesBuffer.bind();
          vao.attr(shader.Attribute("position", &point_t::position));
          vao.attr(shader.Attribute("normal", &point_t::normal));
          vao.attr(shader.Attribute("weight", &point_t::weight));
          vao.attr(shader.Attribute("color", &point_t::color));
        });
      }
      virtual ~Line() {}

      void RenderRelative(const scenegraph::RenderSettings& settings,
                          const math::Mat4x4f& transform) override {
        using namespace emp::math;
        if (elementCount > 0) {
          shader->Use();
          vao.bind();
          verticiesBuffer.bind();
          trianglesBuffer.bind();

          shader->Uniform("model") = transform * Mat4x4f::Translation(0, 0);
          shader->Uniform("projection") = settings.projection;
          shader->Uniform("view") = settings.view;

          glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
        }
      }

      template <class DataIter, class Iter>
      void Apply(DataIter, DataIter, Iter begin, Iter end) {
        using namespace emp::math;
        using namespace emp::opengl;
        using namespace emp::plot::attributes;

        // If there is nothing to show, then bail out
        elementCount = 0;
        if (begin == end) return;

        // Capture the first element of the dataset
        Vec3f start{XyzScaled::Get(*begin).x(), XyzScaled::Get(*begin).y(), 0};
        auto startStroke{Stroke::Get(*begin)};
        auto startStrokeWeight{StrokeWeight::Get(*begin) * 0.5f};
        ++begin;
        // Make sure that there is at least one more item in the dataset
        if (begin == end) return;

        // capture the second element
        Vec3f middle{XyzScaled::Get(*begin).x(), XyzScaled::Get(*begin).y(), 0};
        auto middleStroke{Stroke::Get(*begin)};
        auto middleStrokeWeight{StrokeWeight::Get(*begin) * 0.5f};

        auto segment = (middle - start).Normalized();
        Vec2f normal{-segment.y(), segment.x()};

        // Place the first two verticies into the list of vertices
        std::vector<point_t> verts{
          {start, normal, startStrokeWeight, startStroke},
          {start, -normal, startStrokeWeight, startStroke}};
        std::vector<GLuint> triangles;

        size_t i = 0;
        for (++begin; begin != end; ++begin) {
          Vec3f end{XyzScaled::Get(*begin).x(), XyzScaled::Get(*begin).y(), 0};
          auto stroke{Stroke::Get(*begin)};
          auto weight{StrokeWeight::Get(*begin) * 0.5f};

          auto segment1 = (middle - start).Normalized();
          Vec2f normal1{-segment1.y(), segment1.x()};
          auto segment2 = (end - middle).Normalized();
          Vec2f normal2{-segment2.y(), segment2.x()};

          auto center{(normal1 + normal2).Normalized()};

          verts.push_back(
            point_t{middle, center, middleStrokeWeight, middleStroke});
          verts.push_back(
            point_t{middle, -center, middleStrokeWeight, middleStroke});

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

        segment = (middle - start).Normalized();
        normal = {-segment.y(), segment.x()};

        verts.push_back(
          point_t{middle, normal, middleStrokeWeight, middleStroke});
        verts.push_back(
          point_t{middle, -normal, middleStrokeWeight, middleStroke});
        triangles.push_back(i);
        triangles.push_back(i + 1);
        triangles.push_back(i + 2);

        triangles.push_back(i + 2);
        triangles.push_back(i + 3);
        triangles.push_back(i + 1);

        elementCount = triangles.size();
        if (elementCount > maxElementCount) {
          verticiesBuffer.init(verts, BufferUsage::DynamicDraw);
          trianglesBuffer.init(triangles, BufferUsage::DynamicDraw);
          maxElementCount = elementCount;
        } else {
#ifdef EMSCRIPTEN
          verticiesBuffer.subset(verts);
          trianglesBuffer.subset(triangles);
#else
          auto mappedVerticiesBuffer = verticiesBuffer.map<point_t>(
            verts.size(), BufferAccess::write().invalidatesBuffer());
          std::copy(verts.begin(), verts.end(), mappedVerticiesBuffer);
          verticiesBuffer.unmap();

          auto mappedTrianglesBuffer = trianglesBuffer.map<GLuint>(
            triangles.size(), BufferAccess::write().invalidatesBuffer());
          std::copy(triangles.begin(), triangles.end(), mappedTrianglesBuffer);
          trianglesBuffer.unmap();
#endif
        }
      }
    };
  }  // namespace plot
}  // namespace emp

#endif  // PLOT_LINE_H
