#ifndef PLOT_ALL_H
#define PLOT_ALL_H

#include "../math/region.h"
#include "math/LinAlg.h"
#include "properties.h"

namespace emp {
  namespace plot {

    template <typename... T>
    class All {
      private:
      std::tuple<T...> children;

      template <size_t H, size_t H2, size_t... I, typename Iter,
                typename... Args>
      void showImpl(const std::index_sequence<H, H2, I...>&, Iter begin,
                    Iter end, Args&&... args) {
        std::get<H>(children).show(begin, end, args...);
        showImpl(std::index_sequence<H2, I...>{}, begin, end,
                 std::forward<Args>(args)...);
      }

      template <size_t H, size_t... I, typename Iter, typename... Args>
      void showImpl(const std::index_sequence<H, I...>&, Iter begin, Iter end,
                    Args&&... args) {
        std::get<H>(children).show(begin, end, std::forward<Args>(args)...);
      }

      template <typename Iter, typename... Args>
      void showImpl(const std::index_sequence<>&, Iter begin, Iter end,
                    Args&&... args) {}

      public:
      template <typename... T1>
      All(T1&&... children) : children(std::forward<T1>(children)...) {}

      template <typename Iter, typename... Args>
      void show(Iter begin, Iter end, Args&&... args) {
        showImpl(std::make_index_sequence<sizeof...(T)>{}, begin, end,
                 std::forward<Args>(args)...);
      }
    };

    template <typename... T>
    auto all(T&&... next) {
      return All<T...>{std::forward<T>(next)...};
    }

    template <typename... T>
    class Views : All<T...> {
      public:
      using All<T...>::All;

      template <typename Iter, typename... Args>
      void show(Iter begin, Iter end, const math::Region2D<float>& region,
                Args&&... args) {
        using namespace properties;
        using namespace math;

        Region2D<float> dataRegion;
        for (auto iter{begin}; iter != end; ++iter) {
          dataRegion.include(X::get(*iter), Y::get(*iter));
        }
        dataRegion.addBorder(15);

        auto proj = proj::orthoFromScreen(region.width(), region.height(),
                                          region.width(), region.height());
        auto view = Mat4x4f::translation(0, 0, 0);

        auto rescaler = [&](auto&& point) {
          auto pos = region.rescale({X::get(point), Y::get(point)}, dataRegion);

          return std::forward<decltype(point)>(point) >> ScaledX::is(pos.x()) >>
                 ScaledY::is(pos.y());
        };
        using data_point_type = decltype(rescaler(*begin));

        std::vector<data_point_type> dataPoints;
        std::transform(begin, end, std::back_inserter(dataPoints), rescaler);

        All<T...>::show(dataPoints.begin(), dataPoints.end(), proj, view,
                        std::forward<Args>(args)...);
      }
    };

    template <typename... T>
    auto views(T&&... next) {
      return Views<T...>{std::forward<T>(next)...};
    }

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_ALL_H
