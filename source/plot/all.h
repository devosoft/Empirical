#ifndef PLOT_ALL_H
#define PLOT_ALL_H

#include "../math/region.h"
#include "math/LinAlg.h"
#include "properties.h"

namespace emp {
  namespace plot {
    namespace detail {
      template <typename F, typename T>
      void allDo(F&& callback, T&& tuple, const std::index_sequence<>&) {}

      template <typename F, typename T, size_t H>
      void allDo(F&& callback, T&& tuple, const std::index_sequence<H>&) {
        std::forward<F>(callback)(std::get<H>(std::forward<T>(tuple)));
      }

      template <typename F, typename T, size_t H, size_t H2, size_t... I>
      void allDo(F&& callback, T&& tuple,
                 const std::index_sequence<H, H2, I...>&) {
        callback(std::get<H>(tuple));
        allDo(std::forward<F>(callback), std::forward<T>(tuple),
              std::index_sequence<H2, I...>{});
      }

    }  // namespace detail

    template <typename F, typename T>
    void allDo(F&& callback, T&& tuple) {
      detail::allDo(std::forward<F>(callback), std::forward<T>(tuple),
                    std::make_index_sequence<
                      std::tuple_size<typename std::decay<T>::type>::value>{});
    }

    template <typename... T>
    class All {
      public:
      std::tuple<T...> children;

      public:
      template <typename... T1>
      All(T1&&... children) : children(std::forward<T1>(children)...) {}

      template <typename R, typename Iter>
      void show(const R& region, const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view, Iter begin, Iter end) {
        allDo(
          [&](auto&& child) {
            std::forward<decltype(child)>(child).show(region, begin, end);
          },
          children);
      }
    };

    template <typename... T>
    auto all(T&&... next) {
      return All<T...>{std::forward<T>(next)...};
    }

    template <typename... T>
    class Views {
      public:
      std::tuple<T...> children;

      public:
      template <typename... T1>
      Views(T1&&... children) : children(std::forward<T1>(children)...) {}

      template <typename R, typename Iter>
      void show(const R& region, Iter begin, Iter end) {
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

        allDo(
          [&](auto&& child) {
            std::forward<decltype(child)>(child).show(
              proj, view, dataPoints.begin(), dataPoints.end());
          },
          children);
      }
    };

    template <typename... T>
    auto views(T&&... next) {
      return Views<T...>{std::forward<T>(next)...};
    }

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_ALL_H
