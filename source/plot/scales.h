#ifndef EMP_PLOT_SCALES_H
#define EMP_PLOT_SCALES_H

#include <algorithm>
#include <iterator>

#include "attributes.h"
#include "flow.h"
#include "math/LinAlg.h"
#include "scenegraph/camera.h"
#include "tools/attrs.h"

namespace emp {
  namespace plot {
    template <size_t D>
    class Scale : public Joinable<Scale<D>> {
      public:
      math::Region<float, D> screenSpace;
      math::Vec<float, D> padding;

      template <class S, class P = math::Vec2f>
      constexpr Scale(S&& screen, P&& padding = {0, 0})
        : screenSpace(std::forward<S>(screen)),
          padding(std::forward<P>(padding)) {}

      template <class DataIter, class PropsIter>
      auto Apply(DataIter dbegin, DataIter dend, PropsIter pbegin,
                 PropsIter pend) const {
        using namespace emp::math;
        using namespace emp::plot::attributes;
        Region<float, D> dataSpace;
        for (auto iter = pbegin; iter != pend; ++iter) {
          dataSpace.include(Xyz::Get(*iter));
        }
        auto borderedSpace{screenSpace};
        borderedSpace.addBorder(padding);

        auto scale =
          MakeAttrs(XyzScaled([&dataSpace, &borderedSpace](const auto& p) {
            return borderedSpace.rescale(Xyz::Get(p), dataSpace);
          }));

        auto map = [&scale](const auto& attrs) {
          return Merge(attrs, scale(attrs));
        };

        using result_t = decltype(map(*pbegin));
        std::vector<result_t> results;
        std::transform(pbegin, pend, std::back_inserter(results), map);
        return results;
      }
    };

  }  // namespace plot
}  // namespace emp

#endif  // EMP_PLOT_SCALES_H
