#ifndef PLOT_MAP_H
#define PLOT_MAP_H

#include <algorithm>
#include <vector>

namespace emp {
  namespace plot {

    template <typename... Maps>
    class Map {
      public:
      std::tuple<Maps...> maps;

      template <typename A>
      void change(A&&, const std::index_sequence<>&) {}

      template <typename A, std::size_t I>
      decltype(auto) change(A&& arg, const std::index_sequence<I>&) {
        static_assert(
          is_invocable<std::tuple_element_t<I, decltype(maps)>, A>::value,
          "This map is incompatible with the given iterator! Make sure that "
          "you have passed invocable arguments to your map!");
        return std::get<I>(maps)(std::forward<A>(arg));
      }

      template <typename A, std::size_t H, std::size_t H2, std::size_t... I>
      decltype(auto) change(A&& arg, const std::index_sequence<H, H2, I...>&) {
        static_assert(
          is_invocable<std::tuple_element_t<H, decltype(maps)>, A>::value,
          "This map is incompatible with the given iterator! Make sure that "
          "you have passed invocable arguments to your map!");
        auto tmp = std::get<H>(maps)(std::forward<A>(arg));
        return change(tmp, std::index_sequence<H2, I...>{});
      }

      template <typename A>
      decltype(auto) change(A&& args) {
        return change(std::forward<A>(args),
                      std::make_index_sequence<sizeof...(Maps)>{});
      }

      public:
      Map(const Map&) = default;
      Map(Map&&) = default;
      Map(const std::tuple<Maps...>& maps) : maps{maps} {}
      Map(std::tuple<Maps...>&& maps) : maps{std::move(maps)} {}

      template <typename Iter, typename... Args>
      auto operator()(Iter begin, Iter end, Args&&... args) {
        static_assert(
          std::is_base_of<properties_tag,
                          std::decay_t<typename Iter::value_type>>::value,
          "Maps expect an iterator of properties!");

        using data_point_type = decltype(change(*begin));

        std::vector<data_point_type> dataPoints;
        std::transform(
          begin, end, std::back_inserter(dataPoints), [this](auto&& arg) {
            static_assert(std::is_base_of<properties_tag,
                                          std::decay_t<decltype(arg)>>::value,
                          "Maps expect an iterator of properties!");
            return change(std::forward<decltype(arg)>(arg));
          });
        return dataPoints;
      }
    };  // namespace plot

    template <typename... Maps>
    auto map(Maps&&... maps) {
      return Map<Maps...>{{std::forward<Maps>(maps)...}};
    }
  }  // namespace plot
}  // namespace emp

#endif  // PLOT_GRAPH_H
