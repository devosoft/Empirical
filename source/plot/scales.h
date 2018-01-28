#ifndef EMP_PLOT_SCALES_H
#define EMP_PLOT_SCALES_H

#include "attrs.h"
#include "math/LinAlg.h"
#include "scenegraph/camera.h"

namespace emp {
  namespace plot {

    class Scale {
      public:
      math::Region2df screenSpace;
      math::Vec2f padding;

      template <class S, class P = math::Vec2f>
      constexpr Scale(S&& screen, P&& padding = {0, 0})
        : screenSpace(std::forward<S>(screen)),
          padding(std::forward<P>(padding)) {}

      template <class DataIter, class PropsIter>
      auto apply(DataIter dbegin, DataIter dend, PropsIter pbegin,
                 PropsIter pend) const {
        using namespace emp::math;
        using namespace emp::plot::attrs;
        Region2df dataSpace;
        for (auto iter = pbegin; iter != pend; ++iter) {
          dataSpace.include(XY::get(*iter));
        }
        auto borderedSpace{screenSpace};
        borderedSpace.addBorder(padding);
        return xyScaled([&dataSpace, &borderedSpace](const auto& p) {
                 return borderedSpace.rescale(XY::get(p), dataSpace);
               })
          .applyToRange(pbegin, pend);
      }
    };

  }  // namespace plot
}  // namespace emp

#endif  // EMP_PLOT_SCALES_H
