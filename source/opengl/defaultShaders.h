#ifndef DEFAULT_SHADERS_H
#define DEFAULT_SHADERS_H

#include "color.h"
#include "glcanvas.h"

namespace emp {
  namespace opengl {
    namespace shaders {
      ShaderProgram simpleColorVertices(GLCanvas& canvas) {
        const char* vertexSource =
#ifdef EMSCRIPTEN
          "precision mediump float;"
#endif
          R"glsl(
            attribute vec3 position;
            attribute vec4 color;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 proj;

            varying vec4 fcolor;

            void main()
            {
                gl_Position = proj * view * model * vec4(position, 1.0);
                fcolor = color;
            }
        )glsl";

        const char* fragmentSource =
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

        return canvas.makeShaderProgram(vertexSource, fragmentSource);
      }

      class SimpleSolidColor {
        public:
        ShaderProgram shader;
        Uniform color;
        Uniform model;
        Uniform view;
        Uniform proj;

        VertexArrayObject vao;

        SimpleSolidColor(GLCanvas& canvas)
          : shader(canvas.makeShaderProgram(
#ifdef EMSCRIPTEN
              "precision mediump float;"
#endif
              R"glsl(
                attribute vec3 position;
                uniform vec4 color;

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 proj;

                varying vec4 fcolor;

                void main()
                {
                    gl_Position = proj * view * model * vec4(position, 1.0);
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
            color(shader.uniform("color")),
            model(shader.uniform("model")),
            view(shader.uniform("view")),
            proj(shader.uniform("proj")),
            vao(canvas.makeVAO()
                  .with(BufferType::Array, shader.attribute<Vec3f>("position"))
                  .with(BufferType::ElementArray)) {
        }
      };

      class SimpleVaryingColor {
        public:
        ShaderProgram shader;
        Uniform model;
        Uniform view;
        Uniform proj;

        VertexArrayObject vao;

        struct point_t {
          Vec3f position;
          Color color;
        };

        SimpleVaryingColor(GLCanvas& canvas)
          : shader(canvas.makeShaderProgram(
#ifdef EMSCRIPTEN
              "precision mediump float;"
#endif
              R"glsl(
                attribute vec3 position;
                attribute vec4 color;

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 proj;

                varying vec4 fcolor;

                void main()
                {
                    gl_Position = proj * view * model * vec4(position, 1.0);
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
            model(shader.uniform("model")),
            view(shader.uniform("view")),
            proj(shader.uniform("proj")),
            vao(canvas.makeVAO()
                  .with(BufferType::Array,
                        shader.attribute("position", &point_t::position),
                        shader.attribute("color", &point_t::color))
                  .with(BufferType::ElementArray)) {
        }
      };

    }  // namespace shaders
  }  // namespace opengl
}  // namespace emp

#endif
