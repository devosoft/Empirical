#ifndef DEFAULT_SHADERS_H
#define DEFAULT_SHADERS_H

#include "glcanvas.h"

namespace emp {
  namespace opengl {
    namespace shaders {
      ShaderProgram simpleColorVertices(GLCanvas& canvas) {
        const char* vertexSource = R"glsl(
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

        const char* fragmentSource = R"glsl(
            varying vec4 fcolor;

            void main()
            {
                gl_FragColor = fcolor;
            }
        )glsl";

        return canvas.makeShaderProgram(vertexSource, fragmentSource);
      }

      ShaderProgram simpleSolidColor(GLCanvas& canvas) {
        const char* vertexSource = R"glsl(
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
        )glsl";

        const char* fragmentSource = R"glsl(
            varying vec4 fcolor;

            void main()
            {
                gl_FragColor = fcolor;
            }
        )glsl";

        return canvas.makeShaderProgram(vertexSource, fragmentSource);
      }
    }  // namespace shaders
  }    // namespace opengl
}  // namespace emp

#endif
