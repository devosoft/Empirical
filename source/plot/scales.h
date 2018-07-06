#ifndef EMP_PLOT_SCALES_H
#define EMP_PLOT_SCALES_H

#include <algorithm>
#include <iterator>
#include <sstream>

#include "attributes.h"
#include "flow.h"
#include "math/LinAlg.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "scenegraph/rendering.h"
#include "tools/attrs.h"

namespace emp {
  namespace plot {

    template <size_t D>
    class Scale : public scenegraph::Child {
      public:
      math::Region<float, D> screenSpace;
      math::Vec<float, D> padding;
      math::Region<float, D> data_space;

      template <class S, class P = math::Vec<float, D>>
      constexpr Scale(S&& screen, P&& padding = {0})
        : screenSpace(std::forward<S>(screen)),
          padding(std::forward<P>(padding)) {}

      template <class DATA_ITER>
      auto operator()(DATA_ITER begin, DATA_ITER end) {
        using namespace emp::math;
        using namespace emp::plot::attributes;
        namespace g = emp::graphics;

        data_space.Reset();
        for (auto iter = begin; iter != end; ++iter) {
          data_space.include(Xyz::Get(*iter));
        }
        auto bordered_space{screenSpace};
        bordered_space.AddBorder(padding);

        auto scale =
          MakeAttrs(g::Transform([this, &bordered_space](const auto& p) {
            return bordered_space.RescalePoint(Xyz::Get(p), data_space);
          }));

        auto map = [&scale](const auto& attrs) {
          return Merge(attrs, scale(attrs));
        };

        using result_t = decltype(map(*begin));
        std::vector<result_t> results;
        std::transform(begin, end, std::back_inserter(results), map);
        return results;
      }

      void RenderRelative(emp::graphics::Graphics& g,
                          const math::Mat4x4f& transform) override {
        using namespace emp::math;
        using namespace emp::opengl;

        auto bordered_space{screenSpace};
        bordered_space.AddBorder(padding);

        auto width = bordered_space.extents().x();
        auto height = bordered_space.extents().y();

        g.DrawFilled(graphics::Mesh::Region({
                       {bordered_space.min.x(), bordered_space.min.y()},
                       {bordered_space.max.x(), bordered_space.min.y() + 1},
                     }),
                     emp::graphics::Fill = Color::black(1),
                     emp::graphics::Transform = Mat4x4f::Identity());

        g.DrawFilled(graphics::Mesh::Region({
                       {bordered_space.min.x(), bordered_space.min.y()},
                       {bordered_space.min.x() + 1, bordered_space.max.y()},
                     }),
                     emp::graphics::Fill = Color::black(1),
                     emp::graphics::Transform = Mat4x4f::Identity());

        std::stringstream f;
        f << data_space.min.x();

        g.Text()
          .Draw({
            emp::graphics::Text = f.str(),
            emp::graphics::Fill = Color::green(1),
            emp::graphics::Transform = Mat4x4f::Translation(
              bordered_space.min.x(), bordered_space.min.y(), 0),
            emp::graphics::TextSize = 16,
          })
          .Flush();

        // // Draw the horizontal axis
        // pen
        //   .Draw({
        //     emp::graphics::Fill = Color::black(1),
        //     emp::graphics::Transform =
        //       Mat4x4f::Translation(bordered_space.min.x(),
        //                            bordered_space.min.y()) *
        //       Mat4x4f::Scale(3, 3, 1),
        //   })
        //   .Draw({
        //     emp::graphics::Fill = Color::black(1),
        //     emp::graphics::Transform =
        //       Mat4x4f::Translation(bordered_space.max.x(),
        //                            bordered_space.max.y()) *
        //       Mat4x4f::Scale(3, 3, 1),
        //   })
        //   .Flush();
      }
    };

  }  // namespace plot
}  // namespace emp

#endif  // EMP_PLOT_SCALES_H
