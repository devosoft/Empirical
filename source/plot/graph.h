#ifndef PLOT_GRAPH_H
#define PLOT_GRAPH_H

#include <algorithm>
#include <vector>

#include "../math/region.h"
#include "all.h"
#include "properties.h"

namespace emp {
  namespace plot {

    namespace detail {
      template <typename A, typename B>
      class Then {
        public:
        A first;
        B second;

        template <typename Iter, typename... Args>
        decltype(auto) operator()(Iter begin, Iter end, Args&&... args) {
          auto firstResult{first(begin, end, args...)};

          return second(std::begin(firstResult), std::end(firstResult),
                        std::forward<Args>(args)...);
        }
      };
    }  // namespace detail

    template <typename T = void>
    class Graph {
      private:
      T next;

      public:
      template <typename T_ = T>
      Graph(T_&& next) : next{std::forward<T_>(next)} {}

      template <typename Iter, typename... Args>
      decltype(auto) operator()(Iter begin, Iter end, Args&&... args) {
        using namespace properties;
        using data_point_type = decltype(Value::is(*begin));
        static_assert(std::is_base_of<properties_tag, data_point_type>::value,
                      "Graphs expect an iterator of properties!");

        std::vector<data_point_type> dataPoints;
        std::transform(begin, end, std::back_inserter(dataPoints), Value::is);
        // TODO: change to std::move_iterator(...)
        return next(std::begin(dataPoints), std::end(dataPoints),
                    std::forward<Args>(args)...);
      }

      template <typename O>
      Graph<detail::Then<T, O>> then(O&& other) {
        return {{next, std::forward<O>(other)}};
      }
    };

    template <>
    class Graph<void> {
      public:
      template <typename... Args>
      void operator()(Args&&...) {}

      template <typename T>
      Graph<std::decay_t<T>> then(T&& value) {
        return {std::forward<T>(value)};
      }
    };

    Graph<> graph() { return {}; }

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_GRAPH_H
