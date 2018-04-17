#ifndef EMP_SCENEGRAPH_SHAPES_H
#define EMP_SCENEGRAPH_SHAPES_H

#include "core.h"
#include "math/region.h"
#include "opengl/color.h"
#include "opengl/glcanvas.h"
#include "opengl/glwrap.h"
#include "opengl/shaders.h"
#include "opengl/texture.h"
#include "tools/resources.h"

namespace emp {
  namespace scenegraph {
    namespace shapes {

      class FilledRectangle : public Child {
        private:
        std::string shader_name;
        opengl::VertexArrayObject vao;
        opengl::BufferObject<opengl::BufferType::Array> vertices_buffer;
        opengl::BufferObject<opengl::BufferType::ElementArray> triangles_buffer;

        public:
        math::Region2f size;
        opengl::Color fill;

        FilledRectangle(opengl::GLCanvas& canvas, const math::Region2f& size,
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
          vertices_buffer.init({Vec3f{size.min.x(), size.max.y(), 0},
                                Vec3f{size.max.x(), size.max.y(), 0},
                                Vec3f{size.max.x(), size.min.y(), 0},
                                Vec3f{size.min.x(), size.min.y(), 0}},
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

          vao.bind();
          shader_name = std::forward<S>(name);
          auto& shader = Resources<ShaderProgram>::Get(shader_name);
          vao.attr(shader.Attribute<Vec3f>("position"));
        }

        virtual ~FilledRectangle() {}

        void RenderRelative(const RenderSettings& settings,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;
          auto& shader = Resources<ShaderProgram>::Get(shader_name);

          shader.use();
          vao.bind();

          shader.Uniform("projection") = settings.projection;
          shader.Uniform("view") = settings.view;

          shader.Uniform("model") = transform;
          shader.Uniform("color") = fill;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      };

      class TextureView : public Child {
        private:
        std::string shader_name;
        opengl::VertexArrayObject vao;
        opengl::BufferObject<opengl::BufferType::Array> vertices_buffer;
        opengl::BufferObject<opengl::BufferType::ElementArray> triangles_buffer;

        struct data_t {
          math::Vec3f position;
          math::Vec2f texture_coordinates;
        };

        public:
        math::Region2f size;
        math::Region2f texture_corrdinates;
        std::shared_ptr<opengl::Texture2d> texture;

        TextureView(opengl::GLCanvas& canvas, const math::Region2f& size,
                    std::shared_ptr<opengl::Texture2d> texture,
                    const std::string& shader_name = "DefaultTextured")
          : vao(canvas.MakeVAO()),
            vertices_buffer(canvas.makeBuffer<opengl::BufferType::Array>()),
            triangles_buffer(
              canvas.makeBuffer<opengl::BufferType::ElementArray>()),
            size{size},
            texture{texture} {
          using namespace emp::opengl;
          using namespace emp::math;

          vao.bind();
          vertices_buffer.init(
            {data_t{{size.min.x(), size.max.y(), 0}, {0, 0}},
             data_t{{size.max.x(), size.max.y(), 0}, {1, 0}},
             data_t{{size.max.x(), size.min.y(), 0}, {1, 1}},
             data_t{{size.min.x(), size.min.y(), 0}, {0, 1}}},
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

          std::cout << "SHADER " << name << std::endl;
          shader_name = std::forward<S>(name);
          vao.bind();
          auto& shader = Resources<ShaderProgram>::Get(shader_name);
          vao.attr(shader.Attribute("position", &data_t::position));
          vao.attr(shader.Attribute("uv", &data_t::texture_coordinates));
          std::cout << "SHADER DONE" << std::endl;
        }

        virtual ~TextureView() {}

        void RenderRelative(const RenderSettings& settings,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;
          auto& shader = Resources<ShaderProgram>::Get(shader_name);

          shader.use();
          vao.bind();

          shader.Uniform("projection") = settings.projection;
          shader.Uniform("view") = settings.view;

          shader.Uniform("model") = transform;
          // shader.Uniform("color") = Color::white();
          // std::cout << texture->texture << " " << GL_TEXTURE0 << std::endl;
          shader.Uniform("tex") = *texture;

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      };
    }  // namespace shapes
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_SHAPES_H
