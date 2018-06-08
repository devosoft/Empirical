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
                uniform vec4 fill;

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 projection;

                varying vec4 f_fill;

                void main()
                {
                    gl_Position = projection * view * model * vec4(position, 1.0);
                    f_fill = fill;
                }
            )glsl";

      constexpr auto DEFAULT_SIMPLE_SHADER_FRAGMENT_SRC =
#ifdef EMSCRIPTEN
        "precision mediump float;"
#endif
        R"glsl(
                  varying vec4 f_fill;

                  void main()
                  {
                      gl_FragColor = f_fill;
                  }
              )glsl";

      constexpr auto DEFAULT_TEXTURE_SHADER_VERTEX_SRC =
#ifdef EMSCRIPTEN
        R"glsl(
		precision mediump float;
		attribute vec2 uv;
		attribute vec3 position;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		varying vec2 f_uv;

		void main()
		{
		    gl_Position = projection * view * model * vec4(position, 1.0);
		    f_uv = uv;
		}
            )glsl";
#else
        R"glsl(
          #version 150 core
          in vec2 uv;
          in vec3 position;

          uniform mat4 model;
          uniform mat4 view;
          uniform mat4 projection;
          uniform vec4 color;
          out vec2 f_uv;

          void main()
          {
            gl_Position = projection * view * model * vec4(position, 1.0);
            f_uv = uv;
          }
            )glsl";
#endif

      constexpr auto DEFAULT_TEXTURE_SHADER_FRAGMENT_SRC =
#ifdef EMSCRIPTEN
        R"glsl(
          precision mediump float;

	        varying vec2 f_uv;
          uniform sampler2D tex;

          void main()
          {
              gl_FragColor = mix(texture2D(tex, f_uv), vec4(1, 1, 1, 1), 0.5);
          }
        )glsl";
#else
        R"glsl(
          #version 150 core

          in vec2 f_uv;

          uniform sampler2D tex;
          out vec4 color;

          void main()
          {
            color = mix(texture(tex, f_uv), vec4(0, 0, 0, 1), 0.1);
          }
        )glsl";
#endif

      constexpr auto DEFAULT_FONT_SHADER_VERTEX_SRC =
#ifdef EMSCRIPTEN
        R"glsl(
		precision mediump float;
		attribute vec2 uv;
		attribute vec3 position;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		varying vec2 f_uv;

		void main()
		{
		    gl_Position = projection * view * model * vec4(position, 1.0);
		    f_uv = uv;
		}
            )glsl";
#else
        R"glsl(
          #version 150 core
          in vec2 uv;
          in vec3 position;

          uniform mat4 model;
          uniform mat4 view;
          uniform mat4 projection;
          out vec2 f_uv;

          void main()
          {
            gl_Position = projection * view * model * vec4(position, 1.0);
            f_uv = uv;
          }
            )glsl";
#endif

      constexpr auto DEFAULT_FONT_SHADER_FRAGMENT_SRC =
#ifdef EMSCRIPTEN
        R"glsl(
          precision mediump float;

          varying vec2 f_uv;
          uniform sampler2D tex;
          uniform vec4 fill;

          void main()
          {
              // gl_FragColor = vec4(1, 1, 1, texture2D(tex, f_uv).r);
               gl_FragColor = vec4(0, 0, 0, fill.a * texture2D(tex, f_uv).a);
              // gl_FragColor =  texture2D(tex, f_uv);
          }
        )glsl";
#else
        R"glsl(
          #version 150 core

          in vec2 f_uv;

          uniform vec4 fill;
          uniform sampler2D tex;

          out vec4 color;

          void main()
          {
            // color = vec4(1, 1, 1, texture2D(tex, f_uv).b);
            color = vec4(fill.rgb, fill.a * texture2D(tex, f_uv).r);
          }
        )glsl";
#endif

      void LoadShaders(GLCanvas& canvas) {
        Resources<ShaderProgram>::Add("DefaultVaryingColor", [&canvas] {
          return canvas.makeShaderProgram(DEFAULT_VARYING_SHADER_VERTEX_SRC,
                                          DEFAULT_SIMPLE_SHADER_FRAGMENT_SRC);
        });

        Resources<ShaderProgram>::Add("DefaultSolidColor", [&canvas] {
          return canvas.makeShaderProgram(DEFAULT_SOLID_SHADER_VERTEX_SRC,
                                          DEFAULT_SIMPLE_SHADER_FRAGMENT_SRC);
        });

        Resources<ShaderProgram>::Add("DefaultTextured", [&canvas] {
          return canvas.makeShaderProgram(DEFAULT_TEXTURE_SHADER_VERTEX_SRC,
                                          DEFAULT_TEXTURE_SHADER_FRAGMENT_SRC);
        });
        Resources<ShaderProgram>::Add("DefaultFont", [&canvas] {
          return canvas.makeShaderProgram(DEFAULT_FONT_SHADER_VERTEX_SRC,
                                          DEFAULT_FONT_SHADER_FRAGMENT_SRC);
        });
      }

    }  // namespace shaders
  }  // namespace opengl
}  // namespace emp

#endif
