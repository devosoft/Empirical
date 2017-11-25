#ifndef PLOT_MAP_H
#define PLOT_MAP_H

#include <algorithm>
#include <vector>

namespace emp {
  namespace plot {

    template <typename C, typename T>
    class Map {
      public:
      C change;
      T next;

      public:
      template <typename C1, typename T1>
      Map(C1&& change, T1&& next)
        : change(std::forward<C1>(change)), next(std::forward<T1>(next)) {}

      template <typename Iter, typename... Args>
      void show(Iter begin, Iter end, Args&&... args) {
        using data_point_type = decltype(change(*begin));

        std::vector<data_point_type> dataPoints;
        std::transform(begin, end, std::back_inserter(dataPoints), change);
        next.show(dataPoints.begin(), dataPoints.end(),
                  std::forward<Args>(args)...);
      }
    };

    template <typename C, typename T>
    auto map(C&& change, T&& next) {
      return Map<C, T>{std::forward<C>(change), std::forward<T>(next)};
    }
  }  // namespace plot
}  // namespace emp

#endif  // PLOT_GRAPH_H
