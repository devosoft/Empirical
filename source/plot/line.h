#ifndef PLOT_LINE_H
#define PLOT_LINE_H

#include "attrs.h"
#include "math/LinAlg.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scales.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"

namespace emp {
  namespace plot {
    class Line : public scenegraph::Child {
      private:
      struct __Shader {
        opengl::ShaderProgram program;
        opengl::Uniform model, view, projection;
        opengl::VertexArrayObject vao;
        struct point_t {
          math::Vec3f position;
          opengl::Color color;
        };

        __Shader(opengl::GLCanvas& canvas)
          : program(canvas.makeShaderProgram(
#ifdef EMSCRIPTEN
              "precision mediump float;"
#endif
              R"glsl(
                attribute vec3 position;
                attribute vec4 color;

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 projection;

                varying vec4 fcolor;

                void main()
                {
                    gl_Position = projection * view * model * vec4(position, 1.0);
                    fcolor = color;
                }
            )glsl",
#ifdef EMSCRIPTEN
              "precision mediump float;"
#endif
              R"glsl(
                  varying vec4 fcolor;

                  void main()
                  {
                      gl_FragColor = fcolor;
                  }
              )glsl")),
            model(program.uniform("model")),
            view(program.uniform("view")),
            projection(program.uniform("projection")) {
        }
      } shader;
      opengl::VertexArrayObject vao;

      opengl::BufferObject<opengl::BufferType::Array> verticiesBuffer;
      opengl::BufferObject<opengl::BufferType::ElementArray> trianglesBuffer;
      size_t maxElementCount = 0;

      size_t elementCount = 0;

      public:
      Line(emp::opengl::GLCanvas& canvas)
        : shader(canvas),
          vao(canvas.makeVAO()),
          verticiesBuffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          trianglesBuffer(
            canvas.makeBuffer<opengl::BufferType::ElementArray>()) {
        using namespace emp::opengl;

        vao.bind();
        verticiesBuffer.bind();
        vao.attr(
          shader.program.attribute("position", &__Shader::point_t::position));
        vao.attr(shader.program.attribute("color", &__Shader::point_t::color));
        trianglesBuffer.bind();
      }
      virtual ~Line() {}

      void renderRelative(const scenegraph::Camera& camera,
                          const math::Mat4x4f& transform) {
        using namespace emp::math;
        if (elementCount > 0) {
          shader.program.use();
          vao.bind();
          verticiesBuffer.bind();
          trianglesBuffer.bind();

          shader.model = transform * Mat4x4f::translation(0, 0);
          shader.projection = camera.getProjection();
          shader.view = camera.getView();

          glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
        }
      }

      template <class DataIter, class Iter>
      void apply(DataIter, DataIter, Iter begin, Iter end) {
        using namespace emp::math;
        using namespace emp::opengl;
        using namespace emp::plot::attrs;

        // If there is nothing to show, then bail out
        elementCount = 0;
        if (begin == end) return;

        // Capture the first element of the dataset
        Vec3f start{begin->xyScaled.x(), begin->xyScaled.y(), 0};
        auto startStroke{begin->stroke};
        auto startStrokeWeight{begin->strokeWeight * 0.5f};
        ++begin;
        // Make sure that there is at least one more item in the dataset
        if (begin == end) return;

        // capture the second element
        Vec3f middle{begin->xyScaled.x(), begin->xyScaled.y(), 0};
        auto middleStroke{begin->stroke};
        auto middleStrokeWeight{begin->strokeWeight * 0.5f};

        auto segment = (middle - start).normalized();
        Vec3f normal{-segment.y(), segment.x(), 0};

        // Place the first two verticies into the list of vertices
        std::vector<__Shader::point_t> verts{
          {start + normal * startStrokeWeight, startStroke},
          {start - normal * startStrokeWeight, startStroke}};
        std::vector<GLuint> triangles;

        size_t i = 0;
        for (++begin; begin != end; ++begin) {
          Vec3f end{begin->xyScaled.x(), begin->xyScaled.y(), 0};
          auto stroke{begin->stroke};
          auto weight{begin->strokeWeight * 0.5f};

          auto segment1 = (middle - start).normalized();
          Vec3f normal1{-segment1.y(), segment1.x(), 0};
          auto segment2 = (end - middle).normalized();
          Vec3f normal2{-segment2.y(), segment2.x(), 0};

          auto center{(normal1 + normal2).normalized() * middleStrokeWeight};

          verts.push_back(__Shader::point_t{middle + center, middleStroke});
          verts.push_back(__Shader::point_t{middle - center, middleStroke});

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

        verts.push_back(__Shader::point_t{middle + normal * middleStrokeWeight,
                                          middleStroke});
        verts.push_back(__Shader::point_t{middle - normal * middleStrokeWeight,
                                          middleStroke});
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
          group.verticiesBuffer.subset(verts);
          group.trianglesBuffer.subset(triangles);
#else
          auto mappedVerticiesBuffer = verticiesBuffer.map<__Shader::point_t>(
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
