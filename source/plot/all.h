#ifndef PLOT_ALL_H
#define PLOT_ALL_H

#include "math/LinAlg.h"

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

      template <typename Iter>
      void show(const emp::math::Mat4x4f& projection,
                const emp::math::Mat4x4f& view, Iter begin, Iter end) {
        allDo(
          [&](auto&& child) {
            std::forward<decltype(child)>(child).show(projection, view, begin,
                                                      end);
          },
          children);
      }
    };

    template <typename... T>
    auto all(T&&... next) {
      return All<T...>{std::forward<T>(next)...};
    }

  }  // namespace plot
}  // namespace emp

#endif  // PLOT_ALL_H
