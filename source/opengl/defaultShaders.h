#ifndef DEFAULT_SHADERS_H
#define DEFAULT_SHADERS_H

#include "../tools/resources.h"
#include "color.h"
#include "glcanvas.h"

namespace emp {
  namespace opengl {
    namespace shaders {

      constexpr auto DEFAULT_VARYING_SHADER_VERTEX_SRC =
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

      constexpr auto DEFAULT_SOLID_SHADER_VERTEX_SRC =
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

      constexpr auto DEFAULT_SIMPLE_SHADER_FRAGMENT_SRC =
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

      void LoadShaders(GLCanvas& canvas) {
        Resources<ShaderProgram>::Lazy("DefaultVaryingColor", [&canvas] {
          return canvas.makeShaderProgram(DEFAULT_VARYING_SHADER_VERTEX_SRC,
                                          DEFAULT_SIMPLE_SHADER_FRAGMENT_SRC);
        });

        Resources<ShaderProgram>::Lazy("DefaultSolidColor", [&canvas] {
          return canvas.makeShaderProgram(DEFAULT_SOLID_SHADER_VERTEX_SRC,
                                          DEFAULT_SIMPLE_SHADER_FRAGMENT_SRC);
        });
      }

    }  // namespace shaders
  }  // namespace opengl
}  // namespace emp

#endif
