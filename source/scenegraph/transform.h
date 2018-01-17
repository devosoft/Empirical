#ifndef EMP_SCENEGRAPH_TRANSFORM_H
#define EMP_SCENEGRAPH_TRANSFORM_H

#include "core.h"
#include "math/LinAlg.h"

namespace emp {
  namespace scenegraph {
    template <class T>
    class Transform : public T {
      public:
      math::Mat4x4f transform;

      template <class... U>
      Transform(U&&... args)
        : T{std::forward<U>(args)...}, transform{math::Mat4x4f::identity()} {}

      template <class... U>
      Transform& translate(U&&... args) {
        transform =
          transform * math::Mat4x4f::translation(std::forward<U>(args)...);
        return *this;
      }

      void renderRelative(const opengl::Camera& camera,
                          const math::Mat4x4f& transform) {
        T::renderRelative(camera, this->transform * transform);
      }
    };
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_TRANSFORM_H
