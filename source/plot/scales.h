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
    class Scale : public scenegraph::Node<D> {
      public:
      math::Vec<float, D> padding;
      math::Vec<float, D> screen_space;
      math::Region<float, D> data_space;

      template <typename P = math::Vec<float, D>,
                typename S = math::Vec<float, D>>
      constexpr Scale(P&& padding = {0}) : padding(std::forward<P>(padding)) {}

      template <class DATA_ITER>
      auto operator()(DATA_ITER begin, DATA_ITER end) {
        using namespace emp::math;
        using namespace emp::plot::attributes;
        namespace g = emp::graphics;

        data_space.Reset();
        for (auto iter = begin; iter != end; ++iter) {
          data_space.include(Xyz::Get(*iter));
        }
        emp::math::Region<float, D> bordered_space{{0}, screen_space};
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
                          const math::Mat4x4f& transform,
                          const emp::math::Vec2f& allocated_size) override {
        using namespace emp::math;
        using namespace emp::opengl;

        screen_space.x() = allocated_size.x();
        screen_space.y() = allocated_size.y();

        std::stringstream f;
        f << data_space.min.x();

        auto text = f.str();

        auto size = g.Measure(text, 16);
        padding.x() = std::max(padding.x(), size.x());
        padding.y() = std::max(padding.y(), size.y());

        screen_space = allocated_size;

        auto bordered_space = screen_space;

        // Draw the axies
        g.DrawFilled(graphics::Mesh::Region({
                       {padding.x(), padding.y()},
                       {bordered_space.x() - padding.x(), padding.y() + 1},
                     }),
                     emp::graphics::Fill = Color::black(1),
                     emp::graphics::Transform = transform);

        g.DrawFilled(graphics::Mesh::Region({
                       {
                         padding.x(),
                         padding.y(),
                       },
                       {
                         padding.x() + 1,
                         bordered_space.y() - padding.y(),
                       },
                     }),
                     emp::graphics::Fill = Color::black(1),
                     emp::graphics::Transform = transform);

        g.Text()
          .Draw({
            emp::graphics::Text = text,
            emp::graphics::Fill = Color::black(1),
            emp::graphics::Transform =
              transform *
              Mat4x4f::Translation(0, bordered_space.y() - padding.y()),
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
