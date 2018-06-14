#ifndef EMP_PLOT_SCALES_H
#define EMP_PLOT_SCALES_H

#include <algorithm>
#include <iterator>

#include "attributes.h"
#include "flow.h"
#include "math/LinAlg.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"

namespace emp {
  namespace plot {

    class Scale {};

    // template <size_t D>
    // class Scale : public Joinable<Scale<D>> {
    //   public:
    //   math::Region<float, D> screenSpace;
    //   math::Vec<float, D> padding;

    //   template <class S, class P = math::Vec<float, D>>
    //   constexpr Scale(S&& screen, P&& padding = {0})
    //     : screenSpace(std::forward<S>(screen)),
    //       padding(std::forward<P>(padding)) {}

    //   template <class DataIter, class PropsIter>
    //   auto Apply(DataIter dbegin, DataIter dend, PropsIter pbegin,
    //              PropsIter pend) const {
    //     using namespace emp::math;
    //     using namespace emp::plot::attributes;
    //     Region<float, D> data_space;
    //     for (auto iter = pbegin; iter != pend; ++iter) {
    //       data_space.include(Xyz::Get(*iter));
    //     }
    //     auto bordered_space{screenSpace};
    //     bordered_space.addBorder(padding);

    //     auto scale =
    //       MakeAttrs(XyzScaled([&data_space, &bordered_space](const auto& p) {
    //         return bordered_space.RescalePoint(Xyz::Get(p), data_space);
    //       }));

    //     auto map = [&scale](const auto& attrs) {
    //       return Merge(attrs, scale(attrs));
    //     };

    //     using result_t = decltype(map(*pbegin));
    //     std::vector<result_t> results;
    //     std::transform(pbegin, pend, std::back_inserter(results), map);
    //     return results;
    //   }

    //   void RenderRelative(const scenegraph::RenderSettings& settings,
    //                       const math::Mat4x4f& transform) {
    //     using namespace emp::math;
    //     using namespace emp::opengl;
    //   }
    // };

  }  // namespace plot
}  // namespace emp

#endif  // EMP_PLOT_SCALES_H
