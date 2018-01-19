#ifndef EMP_SCENEGRAPH_SHAPES_H
#define EMP_SCENEGRAPH_SHAPES_H

#include "core.h"
#include "opengl/color.h"
#include "opengl/defaultShaders.h"

namespace emp {
  namespace scenegraph {
    namespace shapes {

      class Rectangle : public Child {
        private:
        opengl::shaders::SimpleSolidColor shader;

        public:
        opengl::Color fill;

        Rectangle(opengl::GLCanvas& canvas) : shader(canvas), fill{0, 0, 0, 1} {
          using namespace emp::opengl;
          using namespace emp::math;

          shader.vao.getBuffer<BufferType::Array>().set(
            {Vec3f{-0.5, +0.5, 0}, Vec3f{+0.5, +0.5, 0}, Vec3f{+0.5, -0.5, 0},
             Vec3f{-0.5, -0.5, 0}},
            BufferUsage::StaticDraw);

          shader.vao.getBuffer<BufferType::ElementArray>().set(
            {
              0, 1, 2,  // First Triangle
              2, 3, 0  // Second Triangle
            },
            BufferUsage::StaticDraw);
        }

        void renderRelative(const Camera& camera,
                            const math::Mat4x4f& transform) {
          using namespace emp::math;
          using namespace emp::opengl;
          shader.shader.use();
          shader.vao.bind();

          shader.proj = camera.getProjection();
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
