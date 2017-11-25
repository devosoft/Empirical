#ifndef PLOT_GRAPH_H
#define PLOT_GRAPH_H

#include <algorithm>
#include <vector>

#include "../math/region.h"
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

      template <typename Iter, typename... Args>
      void show(Iter begin, Iter end, Args&&... args) {
        using namespace properties;
        auto map = [](auto&& value) {
          return nullProps().set<properties::Value>(value);
        };
        using data_point_type = decltype(map(*begin));

        std::vector<data_point_type> dataPoints;
        std::transform(begin, end, std::back_inserter(dataPoints), map);
        next.show(dataPoints.begin(), dataPoints.end(),
                  std::forward<Args>(args)...);
      }
    };

    template <typename T>
    auto graph(T&& next) {
      return Graph<T>{std::forward<T>(next)};
    }

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_GRAPH_H
