#ifndef EMP_SCENEGRAPH_RENDERABLE_H
#define EMP_SCENEGRAPH_RENDERABLE_H

#include "camera.h"
#include "math/LinAlg.h"

namespace emp {
  namespace scenegraph {

    class Renderable {
      public:
      virtual void renderRelative(const Camera& camera,
                                  const math::Mat4x4f& origin) = 0;
    };

  }  // namespace scenegraph
}  // namespace emp

#endif  // EMP_SCENEGRAPH_RENDERABLE_H
