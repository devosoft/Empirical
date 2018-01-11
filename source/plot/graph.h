#ifndef EMP_PLOT_GRAPH
#define EMP_PLOT_GRAPH

#include <algorithm>
#include <iterator>
#include <vector>
#include "math/region.h"
#include "opengl/camera.h"
#include "properties.h"

namespace emp {
  namespace plot {
    namespace __impl {
      template <class T>
      constexpr auto tuple_size_v = std::tuple_size<std::decay_t<T>>::value;

      template <size_t I>
      struct tuple_foreach {
        template <class T, class F>
        static constexpr void call(T&& tuple, F&& callback) {
          callback(std::get<tuple_size_v<T> - I>(tuple));
          tuple_foreach<I - 1>::call(std::forward<T>(tuple),
                                     std::forward<F>(callback));
        }
      };

      template <>
      struct tuple_foreach<1> {
        template <class T, class F>
        static constexpr void call(T&& tuple, F&& callback) {
          std::forward<F>(callback)(
            std::get<tuple_size_v<T> - 1>(std::forward<T>(tuple)));
        }
      };

      template <>
      struct tuple_foreach<0> {
        template <class T, class F>
        static constexpr void call(T&& tuple, F&& callback) {}
      };
    }  // namespace __impl

    template <class T, class F>
    void tuple_foreach(T&& tuple, F&& callback) {
      __impl::tuple_foreach<__impl::tuple_size_v<T>>::call(
        std::forward<T>(tuple), std::forward<F>(callback));
    }

    template <class F, class T>
    auto applyToVector(F&& map, const std::vector<T>& data) {
      using map_t = decltype(map(data[0]));
      std::vector<map_t> mapped;
      mapped.reserve(data.size());

      std::transform(data.begin(), data.end(), std::back_inserter(mapped),
                     std::forward<F>(map));

      return mapped;
    }

    template <class Props, class Layers>
    class Graph {
      private:
      Props props;
      Layers layers;

      public:
      template <class P = Props, class L = Layers>
      Graph(P&& props, L&& layers)
        : props(std::forward<P>(props)), layers(std::forward<L>(layers)) {}

      template <class T>
      void show(const opengl::Camera& camera, const std::vector<T>& data) {
        using namespace properties;
        using namespace math;

        auto stage1{applyToVector(
          [this](auto& d) { return applyPropertyMap(props, d); }, data)};

        Region2D<float> dataRegion;
        for (auto& item : stage1) {
          dataRegion.include(XY::get(item));
        }

        auto viewRegion = camera.getRegion();
        auto stage2{applyToVector(
          [&viewRegion, &dataRegion](auto& d) {
            return XYScaled::set(d, viewRegion.rescale(XY::get(d), dataRegion));
          },
          stage1)};

        tuple_foreach(layers, [&camera, &stage2](auto&& layer) {
          std::forward<decltype(layer)>(layer).show(camera, stage2);
        });
      }
    };

    template <class Props, class Layers>
    Graph<Props, Layers> graph(Props props, Layers layers) {
      return {std::forward<Props>(props), std::forward<Layers>(layers)};
    }
  }  // namespace plot
}  // namespace emp
#endif  // EMP_PLOT_GRAPH
