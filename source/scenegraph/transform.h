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
        : T{std::forward<U>(args)...}, transform{math::Mat4x4f::Identity()} {}

      template <class... U>
      Transform& translate(U&&... args) {
        transform =
          transform * math::Mat4x4f::Translation(std::forward<U>(args)...);
        return *this;
      }

      void RenderRelative(const RenderSettings& settings,
                          const math::Mat4x4f& transform) override {
        T::RenderRelative(settings, this->transform * transform);
      }
    };
  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_TRANSFORM_H
