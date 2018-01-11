#ifndef EMP_PLOT_TUPLE_UTILS_H
#define EMP_PLOT_TUPLE_UTILS_H

#include <tuple>
#include <type_traits>

namespace __impl {

  template <size_t I, class Tuple>
  using rtuple_element_t =
    std::tuple_element_t<std::tuple_size<Tuple>::value - I, Tuple>;

  template <class Needle, class Tuple, size_t I>
  struct tuple_has : std::conditional_t<
                       std::is_same<Needle, rtuple_element_t<I, Tuple>>::value,
                       std::true_type, tuple_has<Needle, Tuple, I - 1>> {};

  template <class Needle, class Tuple>
  struct tuple_has<Needle, Tuple, 0> : std::false_type {};

}  // namespace __impl

template <class Needle, class Tuple>
struct tuple_has
  : __impl::tuple_has<Needle, Tuple, std::tuple_size<Tuple>::value> {};

namespace __impl {

  template <class Needle, class Tuple, size_t I>
  struct search
    : std::conditional_t<
        std::is_same<Needle, std::tuple_element_t<I, Tuple>>::value,
        std::integral_constant<size_t, I>, search<Needle, Tuple, I + 1>> {};

  template <class Needle, class Tuple, bool Guard>
  struct tuple_search_guard;

  template <class Needle, class Tuple>
  struct tuple_search_guard<Needle, Tuple, true> : search<Needle, Tuple, 0> {};

  template <class Needle, class Tuple>
  struct tuple_search_guard<Needle, Tuple, false> {
    private:
    static constexpr bool needle_not_found = sizeof(Needle) == 0;

    public:
    static_assert(needle_not_found, "Needle not found");
  };

}  // namespace __impl

template <class Needle, class Tuple>
struct tuple_search
  : __impl::tuple_search_guard<Needle, Tuple, tuple_has<Needle, Tuple>::value> {
};

#endif
