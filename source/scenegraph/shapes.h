#ifndef EMP_SCENEGRAPH_SHAPES_H
#define EMP_SCENEGRAPH_SHAPES_H

#include "core.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"

#include "tools/resources.h"

namespace emp {
  namespace scenegraph {
    namespace shapes {

      class Rectangle : public Child {
        private:
        std::string shader_name;
        opengl::VertexArrayObject vao;
        opengl::BufferObject<opengl::BufferType::Array> vertices_buffer;
        opengl::BufferObject<opengl::BufferType::ElementArray> triangles_buffer;

        public:
        float size;
        opengl::Color fill;

        Rectangle(opengl::GLCanvas& canvas, float size = 2,
                  const opengl::Color& fill = {0, 0, 0, 1},
                  const std::string& shader_name = "DefaultSolidColor")
          : vao(canvas.MakeVAO()),
            vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
            triangles_buffer(
              canvas.makeBuffer<opengl::BufferType::ElementArray>()),
            size{size},
            fill{fill} {
          using namespace emp::opengl;
          using namespace emp::math;

          vao.bind();

          vertices_buffer.init({Vec3f{-0.5, +0.5, 0}, Vec3f{+0.5, +0.5, 0},
                                Vec3f{+0.5, -0.5, 0}, Vec3f{-0.5, -0.5, 0}},
                               BufferUsage::StaticDraw);

          triangles_buffer.init(
            {
              0, 1, 2,  // First Triangle
              2, 3, 0  // Second Triangle
            },
            BufferUsage::StaticDraw);

          SetShaderName(shader_name);
        }

        template <typename S>
        void SetShaderName(S&& name) {
          using namespace emp::opengl;
          using namespace emp::math;

          shader_name = std::forward<S>(name);
          auto& shader = Resources<ShaderProgram>::Get(shader_name);
          vao.attr(shader.Attribute<Vec3f>("position"));
        }

        virtual ~Rectangle() {}

        void RenderRelative(const RenderSettings& settings,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;
          auto& shader = Resources<ShaderProgram>::Get(shader_name);

          shader.use();
          vao.bind();

          shader.Uniform("projection") = settings.projection;
          shader.Uniform("view") = settings.view;

          shader.Uniform("model") = transform * Mat4x4f::Scale(size);
          shader.Uniform("color") = fill;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      };

    }  // namespace shapes
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_SHAPES_H
