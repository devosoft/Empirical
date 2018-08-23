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

template <size_t D> class Scale : public scenegraph::Node<D> {
  math::Vec<float, D> padding;

public:
  int xticks = 10, yticks = 10, text_size = 16;
  math::Vec<float, D> margin;
  math::Vec<float, D> screen_space;
  math::Region<float, D> data_space;

  template <typename P = math::Vec<float, D>, typename S = math::Vec<float, D>>
  constexpr Scale(P &&margin = {0}) : margin(std::forward<P>(margin)) {}

  template <class DATA_ITER> auto operator()(DATA_ITER begin, DATA_ITER end) {
    using namespace emp::math;
    using namespace emp::plot::attributes;
    namespace g = emp::graphics;

    data_space.Reset();
    for (auto iter = begin; iter != end; ++iter) {
      data_space.include(Xyz::Get(*iter));
    }
    emp::math::Region<float, D> bordered_space{{0}, screen_space};
    bordered_space.AddBorder(padding);

    auto scale = MakeAttrs(g::Transform([this, &bordered_space](const auto &p) {
      return bordered_space.RescalePoint(Xyz::Get(p), data_space);
    }));

    auto map = [&scale](const auto &attrs) {
      return Merge(attrs, scale(attrs));
    };

    using result_t = decltype(map(*begin));
    std::vector<result_t> results;
    std::transform(begin, end, std::back_inserter(results), map);
    return results;
  }

  void RenderRelative(emp::graphics::Graphics &g,
                      const math::Mat4x4f &transform,
                      const emp::math::Vec2f &allocated_size) override {
    using namespace emp::math;
    using namespace emp::opengl;

    screen_space.x() = allocated_size.x();
    screen_space.y() = allocated_size.y();

    screen_space = allocated_size;

    auto bordered_space = screen_space;

    std::vector<std::string> xlabels;
    std::vector<std::string> ylabels;
    float ylabel_width = 0;
    auto delta = data_space.extents();
    delta.x() /= xticks;
    delta.y() /= yticks;

    Vec<float, D> text_padding;

    for (int i = 0; i < xticks; ++i) {
      auto label = std::to_string(delta.x() * i + data_space.min.x());

      xlabels.push_back(label);
      text_padding.y() =
          std::max(text_padding.y(), g.Measure(label, text_size).y());
    }
    for (int i = 0; i < yticks; ++i) {
      auto label = std::to_string(delta.y() * i + data_space.min.y());

      ylabels.push_back(label);
      text_padding.x() =
          std::max(text_padding.x(), g.Measure(label, text_size).x());
    }
    text_padding.x() += 10;
    text_padding.y() += 4;
    padding = text_padding + margin;

    // Draw the axies
    g.DrawFilled(graphics::Mesh::Region({
                     {padding.x(), padding.y()},
                     {bordered_space.x() - padding.x(), padding.y() + 1},
                 }),
                 emp::graphics::Fill = Color::black(1),
                 emp::graphics::Transform = transform);

    g.DrawFilled(graphics::Mesh::Region({
                     {
                         padding.x(), padding.y(),
                     },
                     {
                         padding.x() + 1, bordered_space.y() - padding.y(),
                     },
                 }),
                 emp::graphics::Fill = Color::black(1),
                 emp::graphics::Transform = transform);

    for (int i = 0; i < xticks; ++i) {
      float pos = ((screen_space.x() - padding.x() * 2) / xticks) * i;

      auto &label = ylabels[i];

      g.Text()
          .Draw({
              emp::graphics::Text = label,
              emp::graphics::Fill = Color::black(1),
              emp::graphics::Transform =
                  transform * Mat4x4f::Translation(padding.x() + pos, 2),
              emp::graphics::TextSize = text_size,
          })
          .Flush();
    }

    for (int i = 0; i < yticks; ++i) {
      float pos = (screen_space.y() / xticks) * i;

      auto &label = ylabels[i];

      g.Text()
          .Draw({
              emp::graphics::Text = label,
              emp::graphics::Fill = Color::black(1),
              emp::graphics::Transform =
                  transform * Mat4x4f::Translation(10, pos - padding.y()),
              emp::graphics::TextSize = text_size,
          })
          .Flush();
    }
  }
};

} // namespace plot
} // namespace emp

#endif // EMP_PLOT_SCALES_H
