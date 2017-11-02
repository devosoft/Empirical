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

      class SimpleSolidColor {
        public:
        ShaderProgram shader;
        Uniform color;
        Uniform model;
        Uniform view;
        Uniform proj;

        VertexArrayObject vao;

        SimpleSolidColor(GLCanvas& canvas)
          : shader(canvas.makeShaderProgram(R"glsl(
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
                  .with(BufferType::ElementArray)) {}
      };

      // class SimpleSolidColor {
      //   public:
      //   ShaderProgram shader;
      //   Uniform color;
      //   Uniform model;
      //   Uniform view;
      //   Uniform proj;
      //
      //   VertexArrayObject vao;
      //
      //   private:
      //   template <typename SP, typename C, typename M, typename V, typename
      //   P,
      //             typename VAO>
      //   SimpleSolidColor(SP&& shader, C&& color, M&& model, V&& view,
      //                    P&& projection, VAO&& vao)
      //     : shader(std::forward<SP>(shader)),
      //       color(std::forward<C>(color)),
      //       model(std::forward<M>(model)),
      //       view(std::forward<V>(view)),
      //       proj(std::forward<P>(projection)),
      //       vao(std::forward<VAO>(vao)) {}
      //
      //   public:
      //   static SimpleSolidColor create(GLCanvas& canvas) {
      //     auto shader{canvas.makeShaderProgram(R"glsl(
      //                       attribute vec3 position;
      //                       uniform vec4 color;
      //
      //                       uniform mat4 model;
      //                       uniform mat4 view;
      //                       uniform mat4 proj;
      //
      //                       varying vec4 fcolor;
      //
      //                       void main()
      //                       {
      //                           gl_Position = proj * view * model *
      //                           vec4(position, 1.0); fcolor = color;
      //                       }
      //                   )glsl",
      //                                          R"glsl(
      //                         varying vec4 fcolor;
      //
      //                         void main()
      //                         {
      //                             gl_FragColor = fcolor;
      //                         }
      //                     )glsl")};
      //     auto color{shader.uniform("color")};
      //     auto model{shader.uniform("model")};
      //     auto view{shader.uniform("view")};
      //     auto projection{shader.uniform("proj")};
      //
      //     return {
      //       std::move(shader),
      //       std::move(color),
      //       std::move(model),
      //       std::move(view),
      //       std::move(projection),
      //       canvas.makeVAO()
      //         .with(BufferType::Array, shader.attribute<Vec3f>("position"))
      //         .with(BufferType::ElementArray)};
      //   }
      // };

    }  // namespace shaders
  }    // namespace opengl
}  // namespace emp

#endif
