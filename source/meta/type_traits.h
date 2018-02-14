//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Extensions on the standard library type traits to handle Empirical classes (such as Ptr).

#ifndef EMP_TYPE_TRAITS_H
#define EMP_TYPE_TRAITS_H

#include <functional>
#include <tuple>
#include <utility>

#include "../base/Ptr.h"

namespace emp {

  // Customized type traits
  template <typename T> struct is_ptr_type           { enum { value = false }; };
  template <typename T> struct is_ptr_type<T*>       { enum { value = true }; };
  template <typename T> struct is_ptr_type<T* const> { enum { value = true }; };
  template <typename T> struct is_ptr_type<Ptr<T>>   { enum { value = true }; };
  template <typename T>
  constexpr bool is_ptr_type_v(const T&) { return is_ptr_type<T>::value; }

  template <typename T> struct remove_ptr_type { };    // Not ponter; should break!
  template <typename T> struct remove_ptr_type<T*>     { using type = T; };
  template <typename T> struct remove_ptr_type<Ptr<T>> { using type = T; };
  template <typename T>
  using remove_ptr_type_t = typename remove_ptr_type<T>::type;
  // @CAO: Make sure we are dealing with const and volitile pointers correctly.

  // Can we convert the first pointer into the second?
  template <typename T1, typename T2> struct ptr_pair {
    static constexpr bool Same() { return false; }
    static constexpr bool SameBase() { return false; }
    static bool ConvertOK(T1 * ptr) { return dynamic_cast<T2*>(ptr); }
  };
  template <typename T> struct ptr_pair<T,T> {
    static constexpr bool Same() { return true; }
    static constexpr bool SameBase() { return true; }
    static constexpr bool ConvertOK(T *) { return true; }
  };
  template <typename T> struct ptr_pair<T, const T> {
    static constexpr bool Same() { return false; }
    static constexpr bool SameBase() { return true; }
    static constexpr bool ConvertOK(T *) { return true; }
  };
  template <typename T> struct ptr_pair<const T, T> {
    static constexpr bool Same() { return false; }
    static constexpr bool SameBase() { return true; }
    static constexpr bool ConvertOK(T *) { return false; }
  };

  namespace detail {
    template <typename Fn, typename... Args>
    struct is_invocable_helper {
      private:
      // If U can be invoked with Args... for arguments, then it will have some
      // return value, and we try to create a pointer to it. Note the use of
      // decay_t. This will remove any references from the return value, which
      // would cause SFINAE to fail, since C++ does not allow pointers to
      // references
      template <typename U>
      static std::true_type check(
        U &&,
        std::decay_t<decltype(std::declval<U>()(std::declval<Args>()...))> * =
          nullptr) {
        return {};
      }

      // Catchall which handles the cases where U is not callable with Args
      // arguments
      template <typename U>
      static std::false_type check(...) {
        return {};
      }

      public:
      static constexpr decltype(check<Fn>(std::declval<Fn>())) value() {
        return {};
      }
    };
  }  // namespace detail

  template <typename Fn, typename... Args>
  struct is_invocable
    : decltype(detail::is_invocable_helper<Fn, Args...>::value()) {};

  // @todo: It might be a good idea to move these to a separate file
  // @todo: should these be using the std naming convention?

  namespace __impl_variadics_type_traits {
    // General container type which represents a parameter pack
    template <typename...>
    struct pack {};
    template <template <typename...> class Variadic, typename Pack>
    struct FromPack;

    template <template <typename...> class Variadic, typename Pack>
    using FromPackType = typename FromPack<Variadic, Pack>::type;

    template <template <typename...> class Variadic, typename... U>
    struct FromPack<Variadic, pack<U...>> {
      using type = Variadic<U...>;
    };

    template <bool, typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct VariadicContainsSwitch;

    template <typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct VariadicContains;

    // Base case where pack<U...> == pack<>
    template <typename Needle, template <typename, typename> class Cmp>
    struct VariadicContains<Needle, __impl_variadics_type_traits::pack<>, Cmp>
      : std::false_type {};

    // Base case where Variadic<A...> == Variadic<>
    template <typename Needle, typename U0, typename... U,
              template <typename, typename> class Cmp>
    struct VariadicContains<Needle,
                            __impl_variadics_type_traits::pack<U0, U...>, Cmp>
      : __impl_variadics_type_traits::VariadicContainsSwitch<
          Cmp<Needle, U0>::value, Needle,
          __impl_variadics_type_traits::pack<U...>, Cmp> {};

    template <typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct VariadicContainsSwitch<true, Needle, Haystack, Cmp>
      : std::true_type {};

    template <typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct VariadicContainsSwitch<false, Needle, Haystack, Cmp>
      : VariadicContains<Needle, Haystack, Cmp> {};

  }  // namespace __impl_variadics_type_traits

  // -- VariadicContains --
  // A utility for checking if any template class X<A0, A1, ..., An> has one of
  // its template parameters such that Ak == Needle

  template <typename Needle, typename Haystack,
            template <typename, typename> class Cmp = std::is_same>
  struct VariadicContains;

  // Base case where Variadic<A...> == Variadic<>
  template <typename Needle, template <typename...> class Variadic,
            typename... U, template <typename, typename> class Cmp>
  struct VariadicContains<Needle, Variadic<U...>, Cmp>
    : __impl_variadics_type_traits::VariadicContains<
        Needle, __impl_variadics_type_traits::pack<U...>, Cmp> {};

  template <typename Needle, typename Haystack,
            template <typename, typename> class... Cmp>
  constexpr bool VariadicContainsValue{
    VariadicContains<Needle, Haystack, Cmp...>::value};

  // -- VariadicConcat --
  template <typename... Variadics>
  struct VariadicConcat;
  template <typename... Variadics>
  using VariadicConcatType = typename VariadicConcat<Variadics...>::type;

  namespace __impl_variadics_type_traits {

    template <template <typename...> class Variadic, typename Pack>
    struct VariadicConcat;

    template <template <typename...> class Variadic, class... U>
    struct VariadicConcat<Variadic, pack<pack<U...>>> {
      using type = pack<U...>;
    };

    template <template <typename...> class Variadic, class... A, class... B,
              class... Rest>
    struct VariadicConcat<Variadic, pack<pack<A...>, Variadic<B...>, Rest...>> {
      using type =
        typename VariadicConcat<Variadic,
                                pack<pack<A..., B...>, Rest...>>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic, class... A, class... Rest>
  struct VariadicConcat<Variadic<A...>, Rest...> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic,
      typename __impl_variadics_type_traits::VariadicConcat<
        Variadic, __impl_variadics_type_traits::pack<
                    __impl_variadics_type_traits::pack<A...>, Rest...>>::type>;
  };

  // -- VariadicRemoveDuplicates --
  template <typename Variadic, template <typename> class Filter>
  struct VariadicFilter;

  template <typename Variadic, template <typename> class Filter>
  using VariadicFilterType = typename VariadicFilter<Variadic, Filter>::type;

  namespace __impl_variadics_type_traits {

    template <typename Filtered, typename Unfiltered,
              template <typename> class Filter>
    struct VariadicFilter;

    template <bool, typename Filtered, typename Unfiltered,
              template <typename> class Filter>
    struct VariadicFilterSwitch;

    template <typename Filtered, typename... Unfiltered,
              template <typename> class Filter>
    struct VariadicFilter<Filtered, pack<Unfiltered...>, Filter> {
      using type = Filtered;
    };

    template <typename... Filtered, typename R0, typename... Rest,
              template <typename> class Filter>
    struct VariadicFilter<pack<Filtered...>, pack<R0, Rest...>, Filter>
      : VariadicFilterSwitch<Filter<R0>::value, pack<Filtered...>,
                             pack<R0, Rest...>, Filter> {};

    template <typename... Filtered, typename R0, typename... Rest,
              template <typename> class Filter>
    struct VariadicFilterSwitch<false, pack<Filtered...>, pack<R0, Rest...>,
                                Filter> {
      using type =
        typename VariadicFilter<pack<Filtered...>, pack<Rest...>, Filter>::type;
    };

    template <typename... Filtered, typename R0, typename... Rest,
              template <typename> class Filter>
    struct VariadicFilterSwitch<true, pack<Filtered...>, pack<R0, Rest...>,
                                Filter> {
      using type = typename VariadicFilter<pack<Filtered..., R0>, pack<Rest...>,
                                           Filter>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic, typename... U,
            template <typename> class Filter>
  struct VariadicFilter<Variadic<U...>, Filter> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, typename __impl_variadics_type_traits::VariadicFilter<
                  __impl_variadics_type_traits::pack<>,
                  __impl_variadics_type_traits::pack<U...>, Filter>::type>;
  };

  // -- VariadicRemoveDuplicates --
  template <typename Variadic,
            template <typename, typename> class Cmp = std::is_same>
  struct VariadicRemoveDuplicates;

  template <typename Variadic, template <typename, typename> class... Cmp>
  using VariadicRemoveDuplicatesType =
    typename VariadicRemoveDuplicates<Variadic, Cmp...>::type;

  namespace __impl_variadics_type_traits {

    template <typename Unique, typename Remaining,
              template <typename, typename> class Cmp>
    struct VariadicRemoveDuplicates;

    template <bool, typename Unique, typename Remaining,
              template <typename, typename> class Cmp>
    struct VariadicRemoveDuplicatesSwitch;

    template <typename Unique, typename... Rest,
              template <typename, typename> class Cmp>
    struct VariadicRemoveDuplicates<Unique, pack<Rest...>, Cmp> {
      using type = Unique;
    };

    template <typename... Unique, typename R0, typename... Rest,
              template <typename, typename> class Cmp>
    struct VariadicRemoveDuplicates<pack<Unique...>, pack<R0, Rest...>, Cmp>
      : VariadicRemoveDuplicatesSwitch<
          VariadicContainsValue<R0, pack<Unique...>, Cmp>, pack<Unique...>,
          pack<R0, Rest...>, Cmp> {};

    template <typename... Unique, typename R0, typename... Rest,
              template <typename, typename> class Cmp>
    struct VariadicRemoveDuplicatesSwitch<false, pack<Unique...>,
                                          pack<R0, Rest...>, Cmp> {
      using type = typename VariadicRemoveDuplicates<pack<Unique..., R0>,
                                                     pack<Rest...>, Cmp>::type;
    };

    template <typename... Unique, typename R0, typename... Rest,
              template <typename, typename> class Cmp>
    struct VariadicRemoveDuplicatesSwitch<true, pack<Unique...>,
                                          pack<R0, Rest...>, Cmp> {
      using type = typename VariadicRemoveDuplicates<pack<Unique...>,
                                                     pack<Rest...>, Cmp>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic, typename... U,
            template <typename, typename> class Cmp>
  struct VariadicRemoveDuplicates<Variadic<U...>, Cmp> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, typename __impl_variadics_type_traits::VariadicRemoveDuplicates<
                  __impl_variadics_type_traits::pack<>,
                  __impl_variadics_type_traits::pack<U...>, Cmp>::type>;
  };

  // -- VariadicUnion --
  // Given X<A0, A1, ... An> and X<B0, B1, ... Bm>, this gives X<Union of
  // A and B>. Note that this will also remove any duplicates in A and B.
  // Also, the ordering of the elements in the union is undefined.
  template <typename A, typename B,
            template <typename, typename> class Cmp = std::is_same>
  struct VariadicUnion;

  template <typename A, typename B, template <typename, typename> class... Cmp>
  using VariadicUnionType = typename VariadicUnion<A, B, Cmp...>::type;

  template <template <typename...> class Variadic, typename... A, typename... B,
            template <typename, typename> class Cmp>
  struct VariadicUnion<Variadic<A...>, Variadic<B...>, Cmp> {
    // @todo: There are more efficient ways to do this
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, VariadicRemoveDuplicatesType<
                  VariadicConcatType<__impl_variadics_type_traits::pack<A...>,
                                     __impl_variadics_type_traits::pack<B...>>,
                  Cmp>>;
  };

  // -- VariadicIntersection --
  // Given X<A0, A1, ... An> and X<B0, B1, ... Bm>, this gives X<Intersection of
  // A and B>
  template <typename A, typename B,
            template <typename, typename> class Cmp = std::is_same>
  struct VariadicIntersection;

  template <typename A, typename B, template <typename, typename> class... Cmp>
  using VariadicIntersectionType =
    typename VariadicIntersection<A, B, Cmp...>::type;

  template <template <typename...> class Variadic, typename... A, typename... B,
            template <typename, typename> class Cmp>
  struct VariadicIntersection<Variadic<A...>, Variadic<B...>, Cmp> {
    template <typename T>
    using InA =
      VariadicContains<T, __impl_variadics_type_traits::pack<A...>, Cmp>;
    template <typename T>
    using InB =
      VariadicContains<T, __impl_variadics_type_traits::pack<B...>, Cmp>;

    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, VariadicFilterType<
                  VariadicFilterType<
                    VariadicRemoveDuplicatesType<
                      __impl_variadics_type_traits::pack<A..., B...>, Cmp>,
                    InA>,
                  InB>>;
  };

}  // namespace emp

#endif
