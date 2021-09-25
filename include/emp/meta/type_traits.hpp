/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file type_traits.hpp
 *  @brief Extensions on the standard library type traits to handle Empirical classes (such as Ptr).
 */

#ifndef EMP_META_TYPE_TRAITS_HPP_INCLUDE
#define EMP_META_TYPE_TRAITS_HPP_INCLUDE


#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../base/_is_streamable.hpp"
//^ provides is_streamable implementation,
// located in base directory to preserve levelization

#include "meta.hpp"

namespace emp {

  // Predeclarations used below.
  template <typename TYPE> class Ptr;
  #ifdef NDEBUG
  template <typename T, typename... Ts> using vector = std::vector<T, Ts...>;
  #else
  template <typename T, typename... Ts> class vector;
  #endif

  #ifndef DOXYGEN_SHOULD_SKIP_THIS // Doxygen is getting tripped up by this
  // adapted from https://stackoverflow.com/a/29634934
  namespace detail {

    // using statements allow Argument-Dependent Lookup (ADL)
    // with custom begin/end
    // see https://en.cppreference.com/w/cpp/language/adl
    using std::begin;
    using std::end;

    template <typename T>
    auto is_iterable_impl(int)
    -> decltype (
      // begin/end and operator !=
      begin(std::declval<T&>()) != end(std::declval<T&>()),
      // Handle evil operator ,
      void(),
      // operator ++
      ++std::declval<decltype(begin(std::declval<T&>()))&>(),
      // operator*
      void(*begin(std::declval<T&>())),
      std::true_type{}
    );

    template <typename T>
    std::false_type is_iterable_impl(...);
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Determine if a type is iterable.
  template <typename T>
  using IsIterable = decltype(detail::is_iterable_impl<T>(0));

  // Determine if a type has a ToString() member function.
  template <typename T, typename=void> struct HasToString : std::false_type { };
  template<typename T>
  struct HasToString<emp::decoy_t<T, decltype(std::declval<T>().ToString())>> : std::true_type{};

  // Determine if a type has a ToDouble() member function.
  template <typename T, typename=void> struct HasToDouble : std::false_type { };
  template<typename T>
  struct HasToDouble<emp::decoy_t<T, decltype(std::declval<T>().ToDouble())>> : std::true_type{};

  // Determine if a type has a FromString() member function.
  template <typename T, typename=void> struct HasFromString : std::false_type { };
  template<typename T>
  struct HasFromString<emp::decoy_t<T, decltype(std::declval<T>().FromString(""))>> : std::true_type{};

  // Determine if a type has a FromDouble() member function.
  template <typename T, typename=void> struct HasFromDouble : std::false_type { };
  template<typename T>
  struct HasFromDouble<emp::decoy_t<T, decltype(std::declval<T>().FromDouble(0.0))>> : std::true_type{};

  /// Determine if a type passed in is an std::function type (vs a lambda or a raw function)
  template <typename> struct is_std_function : std::false_type { };
  template <typename... Ts> struct is_std_function<std::function<Ts...>> : std::true_type { };

  /// Convert std::function to base function type.
  template <typename T> struct remove_std_function_type { using type = T; };
  template <typename T> struct remove_std_function_type<std::function<T>> { using type = T; };
  template <typename T> using remove_std_function_t = typename remove_std_function_type<T>::type;

  // Collect the reference type for any container.
  template <typename T> struct element_type { using type = T; };
  template <template <typename...> class TMPL, typename T> struct element_type<TMPL<T>>  { using type = T; };
  template <typename T> using element_t = typename element_type<T>::type;
  // template<typename T> using element_type_t = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

  /// Determine if we have an emp::vector.
  template <typename> struct is_emp_vector : std::false_type { };
  template <typename T, typename... Ts>
  struct is_emp_vector<emp::vector<T, Ts...>> : std::true_type { };


  // Customized type traits; for the moment, make sure that emp::Ptr is handled correctly.
  template <typename> struct is_ptr_type : public std::false_type { };
  template <typename T> struct is_ptr_type<T*> : public std::true_type { };
  template <typename T> struct is_ptr_type<T* const> : public std::true_type { };
  template <typename T> struct is_ptr_type<Ptr<T>> : public std::true_type { };
  template <typename T>
  constexpr bool is_ptr_type_v(const T&) { return is_ptr_type<T>::value; }
  template <typename T> using is_pointer = is_ptr_type<T>;

  template <typename T> struct remove_ptr_type         { using type = T; };
  template <typename T> struct remove_ptr_type<T*>     { using type = T; };
  template <typename T> struct remove_ptr_type<T* const> { using type = const T; };
  template <typename T> struct remove_ptr_type<Ptr<T>> { using type = T; };
  template <typename T>
  using remove_ptr_type_t = typename remove_ptr_type<T>::type;
  template <typename T> using remove_pointer_t = remove_ptr_type_t<T>;
  // @CAO: Make sure we are dealing with const and volatile pointers correctly.

  /// Convert a value to a non-pointer if and only if it is one.
  template <typename T>
  auto & remove_ptr_value(T & value) {
    if constexpr (is_ptr_type<T>::value) return *value;
    else return value;
  }

  /// Figure out which type is an unsigned integer with a specified number of bits.
  template <size_t BIT_COUNT, typename DEFAULT=void> struct uint_bit_count {
    using type = DEFAULT;
  };
  template <> struct uint_bit_count<8> { using type = uint8_t; };
  template <> struct uint_bit_count<16> { using type = uint16_t; };
  template <> struct uint_bit_count<32> { using type = uint32_t; };
  template <> struct uint_bit_count<64> { using type = uint64_t; };

  template <size_t BIT_COUNT, typename DEFAULT=void>
  using uint_bit_count_t = typename uint_bit_count<BIT_COUNT, DEFAULT>::type;

  /// Figure out which type is an integer with a specified number of bits.
  template <size_t BIT_COUNT, typename DEFAULT=void> struct int_bit_count {
    using type = DEFAULT;
  };
  template <> struct int_bit_count<8> { using type = int8_t; };
  template <> struct int_bit_count<16> { using type = int16_t; };
  template <> struct int_bit_count<32> { using type = int32_t; };
  template <> struct int_bit_count<64> { using type = int64_t; };

  template <size_t BIT_COUNT, typename DEFAULT=void>
  using int_bit_count_t = typename int_bit_count<BIT_COUNT, DEFAULT>::type;

  /// Match the constness of another type.
  template <typename T, typename MATCH_T>
  struct match_const { using type = std::remove_const_t<T>; };

  template <typename T, typename MATCH_T>
  struct match_const<T,const MATCH_T> { using type = std::add_const_t<T>; };

  template <typename T, typename MATCH_T> using match_const_t
    = typename match_const<T,MATCH_T>::type;

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

    template <template <typename...> class Variadic, typename Pack>
    struct ToPack;

    template <template <typename...> class Variadic, typename Pack>
    using ToPackType = typename ToPack<Variadic, Pack>::type;

    template <template <typename...> class Variadic, typename... U>
    struct ToPack<Variadic, Variadic<U...>> {
      using type = pack<U...>;
    };

    template <bool, typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct variadic_contains_switch;

    template <typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct variadic_contains;

    // Base case where pack<U...> == pack<>
    template <typename Needle, template <typename, typename> class Cmp>
    struct variadic_contains<Needle, __impl_variadics_type_traits::pack<>, Cmp>
      : std::false_type {};

    // Base case where Variadic<A...> == Variadic<>
    template <typename Needle, typename U0, typename... U,
              template <typename, typename> class Cmp>
    struct variadic_contains<Needle,
                             __impl_variadics_type_traits::pack<U0, U...>, Cmp>
      : __impl_variadics_type_traits::variadic_contains_switch<
          Cmp<Needle, U0>::value, Needle,
          __impl_variadics_type_traits::pack<U...>, Cmp> {};

    template <typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct variadic_contains_switch<true, Needle, Haystack, Cmp>
      : std::true_type {};

    template <typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct variadic_contains_switch<false, Needle, Haystack, Cmp>
      : variadic_contains<Needle, Haystack, Cmp> {};

  }  // namespace __impl_variadics_type_traits

  // -- variadic_contains --
  // A utility for checking if any template class X<A0, A1, ..., An> has one of
  // its template parameters such that Ak == Needle

  template <typename Needle, typename Haystack,
            template <typename, typename> class Cmp = std::is_same>
  struct variadic_contains;

  // Base case where Variadic<A...> == Variadic<>
  template <typename Needle, template <typename...> class Variadic,
            typename... U, template <typename, typename> class Cmp>
  struct variadic_contains<Needle, Variadic<U...>, Cmp>
    : __impl_variadics_type_traits::variadic_contains<
        Needle, __impl_variadics_type_traits::pack<U...>, Cmp> {};

  template <typename Needle, typename Haystack,
            template <typename, typename> class... Cmp>
  constexpr bool variadic_contains_v{
    variadic_contains<Needle, Haystack, Cmp...>::value};

  // -- variadic_concat --
  template <typename... Variadics>
  struct variadic_concat;
  template <typename... Variadics>
  using variadic_concat_t = typename variadic_concat<Variadics...>::type;

  namespace __impl_variadics_type_traits {

    template <template <typename...> class Variadic, typename Pack>
    struct variadic_concat;

    template <template <typename...> class Variadic, class... U>
    struct variadic_concat<Variadic, pack<pack<U...>>> {
      using type = pack<U...>;
    };

    template <template <typename...> class Variadic, class... A, class... B,
              class... Rest>
    struct variadic_concat<Variadic,
                           pack<pack<A...>, Variadic<B...>, Rest...>> {
      using type =
        typename variadic_concat<Variadic,
                                 pack<pack<A..., B...>, Rest...>>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic, class... A, class... Rest>
  struct variadic_concat<Variadic<A...>, Rest...> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic,
      typename __impl_variadics_type_traits::variadic_concat<
        Variadic, __impl_variadics_type_traits::pack<
                    __impl_variadics_type_traits::pack<A...>, Rest...>>::type>;
  };

  // -- variadic_remove_duplicates --
  template <typename Variadic, template <typename> class Filter>
  struct variadic_filter;

  template <typename Variadic, template <typename> class Filter>
  using variadic_filter_t = typename variadic_filter<Variadic, Filter>::type;

  namespace __impl_variadics_type_traits {

    template <typename Filtered, typename Unfiltered,
              template <typename> class Filter>
    struct variadic_filter;

    template <bool, typename Filtered, typename Unfiltered,
              template <typename> class Filter>
    struct variadic_filter_switch;

    template <typename Filtered, typename... Unfiltered,
              template <typename> class Filter>
    struct variadic_filter<Filtered, pack<Unfiltered...>, Filter> {
      using type = Filtered;
    };

    template <typename... Filtered, typename R0, typename... Rest,
              template <typename> class Filter>
    struct variadic_filter<pack<Filtered...>, pack<R0, Rest...>, Filter>
      : variadic_filter_switch<Filter<R0>::value, pack<Filtered...>,
                               pack<R0, Rest...>, Filter> {};

    template <typename... Filtered, typename R0, typename... Rest,
              template <typename> class Filter>
    struct variadic_filter_switch<false, pack<Filtered...>, pack<R0, Rest...>,
                                  Filter> {
      using type = typename variadic_filter<pack<Filtered...>, pack<Rest...>,
                                            Filter>::type;
    };

    template <typename... Filtered, typename R0, typename... Rest,
              template <typename> class Filter>
    struct variadic_filter_switch<true, pack<Filtered...>, pack<R0, Rest...>,
                                  Filter> {
      using type = typename variadic_filter<pack<Filtered..., R0>,
                                            pack<Rest...>, Filter>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic, typename... U,
            template <typename> class Filter>
  struct variadic_filter<Variadic<U...>, Filter> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, typename __impl_variadics_type_traits::variadic_filter<
                  __impl_variadics_type_traits::pack<>,
                  __impl_variadics_type_traits::pack<U...>, Filter>::type>;
  };

  // -- variadic_remove_duplicates --
  template <typename Variadic,
            template <typename, typename> class Cmp = std::is_same>
  struct variadic_remove_duplicates;

  template <typename Variadic, template <typename, typename> class... Cmp>
  using variadic_remove_duplicates_t =
    typename variadic_remove_duplicates<Variadic, Cmp...>::type;

  namespace __impl_variadics_type_traits {

    template <typename Unique, typename Remaining,
              template <typename, typename> class Cmp>
    struct variadic_remove_duplicates;

    template <bool, typename Unique, typename Remaining,
              template <typename, typename> class Cmp>
    struct variadic_remove_duplicates_switch;

    template <typename Unique, typename... Rest,
              template <typename, typename> class Cmp>
    struct variadic_remove_duplicates<Unique, pack<Rest...>, Cmp> {
      using type = Unique;
    };

    template <typename... Unique, typename R0, typename... Rest,
              template <typename, typename> class Cmp>
    struct variadic_remove_duplicates<pack<Unique...>, pack<R0, Rest...>, Cmp>
      : variadic_remove_duplicates_switch<
          variadic_contains_v<R0, pack<Unique...>, Cmp>, pack<Unique...>,
          pack<R0, Rest...>, Cmp> {};

    template <typename... Unique, typename R0, typename... Rest,
              template <typename, typename> class Cmp>
    struct variadic_remove_duplicates_switch<false, pack<Unique...>,
                                             pack<R0, Rest...>, Cmp> {
      using type =
        typename variadic_remove_duplicates<pack<Unique..., R0>, pack<Rest...>,
                                            Cmp>::type;
    };

    template <typename... Unique, typename R0, typename... Rest,
              template <typename, typename> class Cmp>
    struct variadic_remove_duplicates_switch<true, pack<Unique...>,
                                             pack<R0, Rest...>, Cmp> {
      using type =
        typename variadic_remove_duplicates<pack<Unique...>, pack<Rest...>,
                                            Cmp>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic, typename... U,
            template <typename, typename> class Cmp>
  struct variadic_remove_duplicates<Variadic<U...>, Cmp> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic,
      typename __impl_variadics_type_traits::variadic_remove_duplicates<
        __impl_variadics_type_traits::pack<>,
        __impl_variadics_type_traits::pack<U...>, Cmp>::type>;
  };

  // -- variadic_union --
  // Given X<A0, A1, ... An> and X<B0, B1, ... Bm>, this gives X<Union of
  // A and B>. Note that this will also remove any duplicates in A and B.
  // Also, the ordering of the elements in the union is undefined.
  template <template <typename, typename> class Cmp, typename...>
  struct variadic_union;

  template <template <typename, typename> class Cmp, typename... U>
  using variadic_union_t = typename variadic_union<Cmp, U...>::type;

  namespace __impl_variadics_type_traits {
    template <template <typename, typename> class Cmp, typename...>
    struct variadic_union;

    template <template <typename, typename> class Cmp, typename U>
    struct variadic_union<Cmp, U> {
      using type = U;
    };

    template <template <typename, typename> class Cmp, typename U0, typename U1,
              typename... U>
    struct variadic_union<Cmp, U0, U1, U...> {
      private:
      using first_t = variadic_remove_duplicates_t<U0, Cmp>;
      using second_t = variadic_remove_duplicates_t<U1, Cmp>;

      template <typename T>
      struct not_in_first
        : std::integral_constant<bool, !variadic_contains_v<T, first_t, Cmp>> {
      };

      using filtered_second_t = variadic_filter_t<second_t, not_in_first>;

      using head_t = variadic_concat_t<first_t, filtered_second_t>;

      public:
      // @todo: There are more efficient ways to do this
      using type = typename variadic_union<Cmp, head_t, U...>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic,
            template <typename, typename> class Cmp, typename... U>
  struct variadic_union<Cmp, Variadic<U...>> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, variadic_remove_duplicates_t<
                  __impl_variadics_type_traits::pack<U...>, Cmp>>;
  };

  template <template <typename...> class Variadic,
            template <typename, typename> class Cmp, typename... U0,
            typename U1, typename... U>
  struct variadic_union<Cmp, Variadic<U0...>, U1, U...> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic,
      typename __impl_variadics_type_traits::variadic_union<
        Cmp, __impl_variadics_type_traits::pack<U0...>,
        __impl_variadics_type_traits::ToPackType<Variadic, U1>,
        __impl_variadics_type_traits::ToPackType<Variadic, U>...>::type>;
  };

  // -- variadic_intersection --
  // Given X<A0, A1, ... An> and X<B0, B1, ... Bm>, this gives X<Intersection
  // of A and B>
  template <template <typename, typename> class Cmp, typename...>
  struct variadic_intersection;

  template <template <typename, typename> class Cmp, typename... U>
  using variadic_intersection_t =
    typename variadic_intersection<Cmp, U...>::type;

  namespace __impl_variadics_type_traits {

    template <template <typename, typename> class Cmp, typename...>
    struct variadic_intersection;
    template <template <typename, typename> class Cmp, typename U>
    struct variadic_intersection<Cmp, U> {
      using type = U;
    };

    template <template <typename, typename> class Cmp, typename U0, typename U1,
              typename... U>
    struct variadic_intersection<Cmp, U0, U1, U...> {
      private:
      using first_t = variadic_remove_duplicates_t<U0, Cmp>;
      using second_t = variadic_remove_duplicates_t<U1, Cmp>;

      template <typename T>
      struct in_second
        : std::integral_constant<bool, variadic_contains_v<T, second_t, Cmp>> {
      };

      // Remove everything from first_t that is not also in second_t
      using intersection_t = variadic_filter_t<first_t, in_second>;

      public:
      // Remove that everything in intersection_t that is not also in U...
      using type =
        typename variadic_intersection<Cmp, intersection_t, U...>::type;
    };
  }  // namespace __impl_variadics_type_traits

  template <template <typename...> class Variadic,
            template <typename, typename> class Cmp, typename... U>
  struct variadic_intersection<Cmp, Variadic<U...>> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic, variadic_remove_duplicates_t<
                  __impl_variadics_type_traits::pack<U...>, Cmp>>;
  };

  template <template <typename...> class Variadic,
            template <typename, typename> class Cmp, typename... U0,
            typename U1, typename... U>
  struct variadic_intersection<Cmp, Variadic<U0...>, U1, U...> {
    using type = __impl_variadics_type_traits::FromPackType<
      Variadic,
      typename __impl_variadics_type_traits::variadic_intersection<
        Cmp, __impl_variadics_type_traits::pack<U0...>,
        __impl_variadics_type_traits::ToPackType<Variadic, U1>,
        __impl_variadics_type_traits::ToPackType<Variadic, U>...>::type>;
  };

  // variadic_index_of

  template <typename Needle, typename Haystack,
            template <typename, typename> class Cmp = std::is_same>
  struct variadic_index_of;

  template <typename Needle, typename Haystack,
            template <typename, typename> class... Cmp>
  static constexpr auto variadic_index_of_v = {
    variadic_index_of<Needle, Haystack, Cmp...>::value};

  namespace __impl_variadics_type_traits {

    template <size_t I, typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct variadic_index_of;

    template <bool, size_t, typename Needle, typename Haystack,
              template <typename, typename> class Cmp>
    struct variadic_index_ofSwitch;

    template <size_t I, typename Needle, typename... U,
              template <typename, typename> class Cmp>
    struct variadic_index_ofSwitch<true, I, Needle, pack<U...>, Cmp>
      : std::integral_constant<size_t, I> {};

    template <size_t I, typename Needle, typename... U,
              template <typename, typename> class Cmp>
    struct variadic_index_ofSwitch<false, I, Needle, pack<U...>, Cmp>
      : variadic_index_of<I + 1, Needle, pack<U...>, Cmp> {};

    template <size_t I, typename Needle, typename U0, typename... U,
              template <typename, typename> class Cmp>
    struct variadic_index_of<I, Needle, pack<U0, U...>, Cmp>
      : variadic_index_ofSwitch<Cmp<Needle, U0>::value, I, Needle, pack<U...>,
                                Cmp> {};

  }  // namespace __impl_variadics_type_traits

  template <typename Needle, template <typename...> class Haystack,
            typename... U, template <typename, typename> class Cmp>
  struct variadic_index_of<Needle, Haystack<U...>, Cmp>
    : __impl_variadics_type_traits::variadic_index_of<
        0, Needle, __impl_variadics_type_traits::pack<U...>, Cmp> {};

}  // namespace emp

#endif // #ifndef EMP_META_TYPE_TRAITS_HPP_INCLUDE
