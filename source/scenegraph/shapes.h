#ifndef EMP_SCENEGRAPH_SHAPES_H
#define EMP_SCENEGRAPH_SHAPES_H

#include "core.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"

namespace emp {
  namespace scenegraph {
    namespace shapes {
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

      class Rectangle : public Child {
        private:
        __shader::Shader shader;
        opengl::BufferObject<opengl::BufferType::Array> verticesBuffer;
        opengl::BufferObject<opengl::BufferType::ElementArray> trianglesBuffer;

        public:
        opengl::Color fill;

        Rectangle(opengl::GLCanvas& canvas)
          : shader(canvas),
            verticesBuffer(canvas.makeBuffer<opengl::BufferType::Array>()),
            trianglesBuffer(
              canvas.makeBuffer<opengl::BufferType::ElementArray>()),
            fill{0, 0, 0, 1} {
          using namespace emp::opengl;
          using namespace emp::math;

          shader.vao.bind();

          verticesBuffer.init({Vec3f{-0.5, +0.5, 0}, Vec3f{+0.5, +0.5, 0},
                               Vec3f{+0.5, -0.5, 0}, Vec3f{-0.5, -0.5, 0}},
                              BufferUsage::StaticDraw);
          shader.vao.attr(shader.program.attribute<Vec3f>("position"));

          trianglesBuffer.init(
            {
              0, 1, 2,  // First Triangle
              2, 3, 0  // Second Triangle
            },
            BufferUsage::StaticDraw);
        }

        virtual ~Rectangle() {}

        void renderRelative(const Camera& camera,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;
          shader.program.use();
          shader.vao.bind();

          shader.projection = camera.getProjection();
          shader.view = camera.getView();

          shader.model = transform * Mat4x4f::scale(10);

          shader.color = fill;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      };

    }  // namespace shapes
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_SHAPES_H
