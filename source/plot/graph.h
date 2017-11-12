#ifndef PLOT_GRAPH_H
#define PLOT_GRAPH_H

#include <algorithm>
#include <vector>

#include "math/LinAlg.h"
#include "properties.h"

namespace emp {
  namespace plot {
    template <typename T>
    class Graph {
      private:
      T next;

      public:
      template <typename T1>
      Graph(T1&& next) : next(std::forward<T1>(next)) {}

      template <typename Iter>
      void show(const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view, Iter begin, Iter end) {
        using namespace properties;
        auto map = [](auto&& value) {
          return nullProps().set<properties::Value>(value);
        };
        using data_point_type = decltype(map(*begin));

        std::vector<data_point_type> dataPoints;
        std::transform(begin, end, std::back_inserter(dataPoints), map);
        next.show(projection, view, dataPoints.begin(), dataPoints.end());

        // float dminX = std::numeric_limits<float>::max();
        // float dminY = std::numeric_limits<float>::max();
        //
        // float dmaxX = std::numeric_limits<float>::lowest();
        // float dmaxY = std::numeric_limits<float>::lowest();
        //
        // for (auto& dp : dataPoints) {
        //   if (dp.x > dmaxX) dmaxX = dp.x;
        //   if (dp.x < dminX) dminX = dp.x;
        //
        //   if (dp.y > dmaxY) dmaxY = dp.y;
        //   if (dp.y < dminY) dminY = dp.y;
        // }
        // allDo(
        //   [&](auto&& layer) {
        //     std::forward<decltype(layer)>(layer).show(
        //       projection, view, dataPoints.begin(), dataPoints.end());
        //   },
        //   layers);
      }
    };

    template <typename T>
    auto graph(T&& next) {
      return Graph<T>{std::forward<T>(next)};
    }

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_GRAPH_H
