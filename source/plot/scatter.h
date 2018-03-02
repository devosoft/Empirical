#ifndef PLOT_SCATTER_H
#define PLOT_SCATTER_H

#include <cmath>
#include "flow.h"
#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "scales.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"

namespace emp {
  namespace plot {
    namespace __shader {
      using namespace opengl;
      constexpr auto VertexShaderSource =
#ifdef EMSCRIPTEN
        "precision mediump float;"
#endif
        R"glsl(
                attribute vec3 position;
                uniform vec4 color;

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 projection;

                varying vec4 fcolor;

                void main()
                {
                    gl_Position = projection * view * model * vec4(position, 1.0);
                    fcolor = color;
                }
            )glsl";

      constexpr auto FragmentShaderSource =
#ifdef EMSCRIPTEN
        "precision mediump float;"
#endif
        R"glsl(
                  varying vec4 fcolor;

                  void main()
                  {
                      gl_FragColor = fcolor;
                  }
              )glsl";

      struct Shader {
        ShaderProgram program;
        Uniform color;
        Uniform model;
        Uniform view;
        Uniform projection;
        VertexArrayObject vao;

        Shader(GLCanvas& canvas)
          : program(canvas.makeShaderProgram(VertexShaderSource,
                                             FragmentShaderSource)),
            color(program.uniform("color")),
            model(program.uniform("model")),
            view(program.uniform("view")),
            projection(program.uniform("projection")),
            vao(canvas.makeVAO()) {
          program.use();
        }
      };
    }  // namespace __shader

    class Scatter : public scenegraph::Child, public Joinable<Scatter> {
      private:
      __shader::Shader shader;
      opengl::BufferObject<opengl::BufferType::Array> verticesBuffer;
      opengl::BufferObject<opengl::BufferType::ElementArray> trianglesBuffer;

      size_t trianglesCount;
      std::vector<std::tuple<math::Mat4x4f, opengl::Color>> points;

      public:
      Scatter(emp::opengl::GLCanvas& canvas, size_t vertexCount = 4)
        : shader(canvas),
          verticesBuffer(canvas.makeBuffer<opengl::BufferType::Array>()),
          trianglesBuffer(
            canvas.makeBuffer<opengl::BufferType::ElementArray>()) {
        using namespace emp::opengl;
        using namespace emp::math;

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
        shader.vao.bind();
        verticesBuffer.init(vertices, BufferUsage::StaticDraw);
        shader.vao.attr(shader.program.attribute<Vec3f>("position"));
        trianglesBuffer.init(triangles, BufferUsage::StaticDraw);
      }

      virtual ~Scatter() {}

      void renderRelative(const scenegraph::Camera& camera,
                          const math::Mat4x4f& transform) {
        using namespace emp::math;
        using namespace emp::opengl;

        shader.program.use();
        shader.vao.bind();
        verticesBuffer.bind();
        trianglesBuffer.bind();

        shader.projection = camera.getProjection();
        shader.view = camera.getView();

        for (auto& pt : points) {
          Mat4x4f model;
          std::tie(model, shader.color) = pt;
          shader.model = transform * model;

          glDrawElements(GL_TRIANGLES, trianglesCount, GL_UNSIGNED_INT, 0);
        }
      }

      template <class DataIter, class Iter>
      void Apply(DataIter, DataIter, Iter begin, Iter end) {
        using namespace emp::math;
        using namespace emp::plot::attributes;
        points.clear();

        for (; begin != end; ++begin) {
          auto model = Mat4x4f::translation(XyzScaled::Get(*begin).x(),
                                            XyzScaled::Get(*begin).y(), 0) *
                       Mat4x4f::scale(PointSize::Get(*begin));

          points.push_back({model, Fill::Get(*begin)});
        }
      }
    };
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_SCATTER_H
