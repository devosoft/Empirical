#ifndef EMP_PLOT_SCALES_H
#define EMP_PLOT_SCALES_H

#include "attrs.h"
#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "scenegraph/camera.h"

namespace emp {
  namespace plot {

    class Scale {
      public:
      math::Region2df screenSpace;

      template <class... U>
      constexpr Scale(U&&... args) : screenSpace(std::forward<U>(args)...) {}

      template <class DataIter, class PropsIter>
      auto apply(DataIter dbegin, DataIter dend, PropsIter pbegin,
                 PropsIter pend) const {
        using namespace emp::math;
        using namespace emp::plot::attrs;
        Region2df dataSpace;
        for (auto iter = pbegin; iter != pend; ++iter) {
          dataSpace.include(XY::get(*iter));
        }
        return xyScaled([&dataSpace, this](const auto& p) {
                 return screenSpace.rescale(XY::get(p), dataSpace);
               })
          .applyToRange(pbegin, pend);
      }
    };

  }  // namespace plot
}  // namespace emp

#endif  // EMP_PLOT_SCALES_H
