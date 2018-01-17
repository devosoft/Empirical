#ifndef EMP_PLOT_SCALES_H
#define EMP_PLOT_SCALES_H

#include "data.h"
#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "scenegraph/camera.h"

namespace emp {
  namespace plot {
    DEFINE_PROPERTY(XY, xy);
    DEFINE_PROPERTY(XYScaled, xyScaled);

    template <class I, class M, class R>
    class ScaleData {
      private:
      opengl::shaders::SimpleSolidColor shader;
      std::tuple<C...> children;

      public:
      template <class D>
      void show(const opengl::Camera& camera, std::vector<D>& data) {}
    };
  }  // namespace plot
}  // namespace emp

#endif  // EMP_PLOT_SCALES_H
