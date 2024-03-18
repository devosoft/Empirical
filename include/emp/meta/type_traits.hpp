/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2024.
 *
 *  @file type_traits.hpp
 *  @brief Extensions on the standard library type traits to handle Empirical classes (such as Ptr).
 */

#ifndef EMP_META_TYPE_TRAITS_HPP_INCLUDE
#define EMP_META_TYPE_TRAITS_HPP_INCLUDE


#include <cstdint>       // uint8_t, uint16_t, etc.
#include <functional>
#include <span>
#include <stddef.h>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../base/concepts.hpp"
#include "meta.hpp"


namespace emp {

  /// Type trait framework for working with whole TYPE PACKS.
  template <template <typename...> typename TRAIT, typename... Ts>
  struct has_trait_any : std::false_type {};
  template <template <typename...> typename TRAIT, typename T, typename... EXTRA_Ts>
  struct has_trait_any<TRAIT, T, EXTRA_Ts...>
      : std::bool_constant<TRAIT<T>::value || has_trait_any<TRAIT, EXTRA_Ts...>::value> {};

  template <template <typename...> typename TRAIT, typename... Ts>
  struct has_trait_all : std::true_type {};
  template <template <typename...> typename TRAIT, typename T, typename... EXTRA_Ts>
  struct has_trait_all<TRAIT, T, EXTRA_Ts...>
      : std::bool_constant<TRAIT<T>::value && has_trait_all<TRAIT, EXTRA_Ts...>::value> {};

  template <template <typename...> typename TRAIT, typename... Ts>
  struct has_trait_none : std::negate<emp::has_trait_any<TRAIT, Ts...>> {};


  template <typename... Ts> using has_any_void = has_trait_any<std::is_void, Ts...>;
  template <typename... Ts> using has_only_void = has_trait_all<std::is_void, Ts...>;
  template <typename... Ts> using has_no_void = has_trait_none<std::is_void, Ts...>;

  template <typename... Ts> using has_any_null_pointer = has_trait_any<std::is_null_pointer, Ts...>;
  template <typename... Ts> using has_only_null_pointer = has_trait_all<std::is_null_pointer, Ts...>;
  template <typename... Ts> using has_no_null_pointer = has_trait_none<std::is_null_pointer, Ts...>;

  template <typename... Ts> using has_any_integral = has_trait_any<std::is_integral, Ts...>;
  template <typename... Ts> using has_only_integral = has_trait_all<std::is_integral, Ts...>;
  template <typename... Ts> using has_no_integral = has_trait_none<std::is_integral, Ts...>;

  template <typename... Ts> using has_any_floating_point = has_trait_any<std::is_floating_point, Ts...>;
  template <typename... Ts> using has_only_floating_point = has_trait_all<std::is_floating_point, Ts...>;
  template <typename... Ts> using has_no_floating_point = has_trait_none<std::is_floating_point, Ts...>;

  template <typename... Ts> using has_any_array = has_trait_any<std::is_array, Ts...>;
  template <typename... Ts> using has_only_array = has_trait_all<std::is_array, Ts...>;
  template <typename... Ts> using has_no_array = has_trait_none<std::is_array, Ts...>;

  template <typename... Ts> using has_any_enum = has_trait_any<std::is_enum, Ts...>;
  template <typename... Ts> using has_only_enum = has_trait_all<std::is_enum, Ts...>;
  template <typename... Ts> using has_no_enum = has_trait_none<std::is_enum, Ts...>;

  template <typename... Ts> using has_any_union = has_trait_any<std::is_union, Ts...>;
  template <typename... Ts> using has_only_union = has_trait_all<std::is_union, Ts...>;
  template <typename... Ts> using has_no_union = has_trait_none<std::is_union, Ts...>;

  template <typename... Ts> using has_any_class = has_trait_any<std::is_class, Ts...>;
  template <typename... Ts> using has_only_class = has_trait_all<std::is_class, Ts...>;
  template <typename... Ts> using has_no_class = has_trait_none<std::is_class, Ts...>;

  template <typename... Ts> using has_any_function = has_trait_any<std::is_function, Ts...>;
  template <typename... Ts> using has_only_function = has_trait_all<std::is_function, Ts...>;
  template <typename... Ts> using has_no_function = has_trait_none<std::is_function, Ts...>;

  template <typename... Ts> using has_any_pointer = has_trait_any<std::is_pointer, Ts...>;
  template <typename... Ts> using has_only_pointer = has_trait_all<std::is_pointer, Ts...>;
  template <typename... Ts> using has_no_pointer = has_trait_none<std::is_pointer, Ts...>;

  template <typename... Ts> using has_any_lvalue_reference = has_trait_any<std::is_lvalue_reference, Ts...>;
  template <typename... Ts> using has_only_lvalue_reference = has_trait_all<std::is_lvalue_reference, Ts...>;
  template <typename... Ts> using has_no_lvalue_reference = has_trait_none<std::is_lvalue_reference, Ts...>;

  template <typename... Ts> using has_any_rvalue_reference = has_trait_any<std::is_rvalue_reference, Ts...>;
  template <typename... Ts> using has_only_rvalue_reference = has_trait_all<std::is_rvalue_reference, Ts...>;
  template <typename... Ts> using has_no_rvalue_reference = has_trait_none<std::is_rvalue_reference, Ts...>;

  template <typename... Ts> using has_any_member_object_pointer = has_trait_any<std::is_member_object_pointer, Ts...>;
  template <typename... Ts> using has_only_member_object_pointer = has_trait_all<std::is_member_object_pointer, Ts...>;
  template <typename... Ts> using has_no_member_object_pointer = has_trait_none<std::is_member_object_pointer, Ts...>;

  template <typename... Ts> using has_any_member_function_pointer = has_trait_any<std::is_member_function_pointer, Ts...>;
  template <typename... Ts> using has_only_member_function_pointer = has_trait_all<std::is_member_function_pointer, Ts...>;
  template <typename... Ts> using has_no_member_function_pointer = has_trait_none<std::is_member_function_pointer, Ts...>;


  template <typename... Ts> using has_any_fundamental = has_trait_any<std::is_fundamental, Ts...>;
  template <typename... Ts> using has_only_fundamental = has_trait_all<std::is_fundamental, Ts...>;
  template <typename... Ts> using has_no_fundamental = has_trait_none<std::is_fundamental, Ts...>;

  template <typename... Ts> using has_any_arithmetic = has_trait_any<std::is_arithmetic, Ts...>;
  template <typename... Ts> using has_only_arithmetic = has_trait_all<std::is_arithmetic, Ts...>;
  template <typename... Ts> using has_no_arithmetic = has_trait_none<std::is_arithmetic, Ts...>;

  template <typename... Ts> using has_any_scalar = has_trait_any<std::is_scalar, Ts...>;
  template <typename... Ts> using has_only_scalar = has_trait_all<std::is_scalar, Ts...>;
  template <typename... Ts> using has_no_scalar = has_trait_none<std::is_scalar, Ts...>;

  template <typename... Ts> using has_any_object = has_trait_any<std::is_object, Ts...>;
  template <typename... Ts> using has_only_object = has_trait_all<std::is_object, Ts...>;
  template <typename... Ts> using has_no_object = has_trait_none<std::is_object, Ts...>;

  template <typename... Ts> using has_any_compound = has_trait_any<std::is_compound, Ts...>;
  template <typename... Ts> using has_only_compound = has_trait_all<std::is_compound, Ts...>;
  template <typename... Ts> using has_no_compound = has_trait_none<std::is_compound, Ts...>;

  template <typename... Ts> using has_any_reference = has_trait_any<std::is_reference, Ts...>;
  template <typename... Ts> using has_only_reference = has_trait_all<std::is_reference, Ts...>;
  template <typename... Ts> using has_no_reference = has_trait_none<std::is_reference, Ts...>;

  template <typename... Ts> using has_any_member_pointer = has_trait_any<std::is_member_pointer, Ts...>;
  template <typename... Ts> using has_only_member_pointer = has_trait_all<std::is_member_pointer, Ts...>;
  template <typename... Ts> using has_no_member_pointer = has_trait_none<std::is_member_pointer, Ts...>;
  
  
  template <typename... Ts> using has_any_const = has_trait_any<std::is_const, Ts...>;
  template <typename... Ts> using has_only_const = has_trait_all<std::is_const, Ts...>;
  template <typename... Ts> using has_no_const = has_trait_none<std::is_const, Ts...>;

  template <typename... Ts> using has_any_volatile = has_trait_any<std::is_volatile, Ts...>;
  template <typename... Ts> using has_only_volatile = has_trait_all<std::is_volatile, Ts...>;
  template <typename... Ts> using has_no_volatile = has_trait_none<std::is_volatile, Ts...>;

  template <typename... Ts> using has_any_trivial = has_trait_any<std::is_trivial, Ts...>;
  template <typename... Ts> using has_only_trivial = has_trait_all<std::is_trivial, Ts...>;
  template <typename... Ts> using has_no_trivial = has_trait_none<std::is_trivial, Ts...>;

  template <typename... Ts> using has_any_trivially_copyable = has_trait_any<std::is_trivially_copyable, Ts...>;
  template <typename... Ts> using has_only_trivially_copyable = has_trait_all<std::is_trivially_copyable, Ts...>;
  template <typename... Ts> using has_no_trivially_copyable = has_trait_none<std::is_trivially_copyable, Ts...>;

  template <typename... Ts> using has_any_standard_layout = has_trait_any<std::is_standard_layout, Ts...>;
  template <typename... Ts> using has_only_standard_layout = has_trait_all<std::is_standard_layout, Ts...>;
  template <typename... Ts> using has_no_standard_layout = has_trait_none<std::is_standard_layout, Ts...>;

  template <typename... Ts> using has_any_empty = has_trait_any<std::is_empty, Ts...>;
  template <typename... Ts> using has_only_empty = has_trait_all<std::is_empty, Ts...>;
  template <typename... Ts> using has_no_empty = has_trait_none<std::is_empty, Ts...>;

  template <typename... Ts> using has_any_polymorphic = has_trait_any<std::is_polymorphic, Ts...>;
  template <typename... Ts> using has_only_polymorphic = has_trait_all<std::is_polymorphic, Ts...>;
  template <typename... Ts> using has_no_polymorphic = has_trait_none<std::is_polymorphic, Ts...>;

  template <typename... Ts> using has_any_abstract = has_trait_any<std::is_abstract, Ts...>;
  template <typename... Ts> using has_only_abstract = has_trait_all<std::is_abstract, Ts...>;
  template <typename... Ts> using has_no_abstract = has_trait_none<std::is_abstract, Ts...>;

  template <typename... Ts> using has_any_final = has_trait_any<std::is_final, Ts...>;
  template <typename... Ts> using has_only_final = has_trait_all<std::is_final, Ts...>;
  template <typename... Ts> using has_no_final = has_trait_none<std::is_final, Ts...>;

  template <typename... Ts> using has_any_aggregate = has_trait_any<std::is_aggregate, Ts...>;
  template <typename... Ts> using has_only_aggregate = has_trait_all<std::is_aggregate, Ts...>;
  template <typename... Ts> using has_no_aggregate = has_trait_none<std::is_aggregate, Ts...>;

  template <typename... Ts> using has_any_signed = has_trait_any<std::is_signed, Ts...>;
  template <typename... Ts> using has_only_signed = has_trait_all<std::is_signed, Ts...>;
  template <typename... Ts> using has_no_signed = has_trait_none<std::is_signed, Ts...>;

  template <typename... Ts> using has_any_unsigned = has_trait_any<std::is_unsigned, Ts...>;
  template <typename... Ts> using has_only_unsigned = has_trait_all<std::is_unsigned, Ts...>;
  template <typename... Ts> using has_no_unsigned = has_trait_none<std::is_unsigned, Ts...>;

  template <typename... Ts> using has_any_bounded_array = has_trait_any<std::is_bounded_array, Ts...>;
  template <typename... Ts> using has_only_bounded_array = has_trait_all<std::is_bounded_array, Ts...>;
  template <typename... Ts> using has_no_bounded_array = has_trait_none<std::is_bounded_array, Ts...>;

  template <typename... Ts> using has_any_unbounded_array = has_trait_any<std::is_unbounded_array, Ts...>;
  template <typename... Ts> using has_only_unbounded_array = has_trait_all<std::is_unbounded_array, Ts...>;
  template <typename... Ts> using has_no_unbounded_array = has_trait_none<std::is_unbounded_array, Ts...>;


  /// Convert std::function to base function type.
  template <typename T> struct remove_std_function_type { using type = T; };
  template <typename T> struct remove_std_function_type<std::function<T>> { using type = T; };
  template <typename T> using remove_std_function_t = typename remove_std_function_type<T>::type;

  // Collect the reference type for any standard container.
  template <typename T> struct element_type { using type = T; };
  template <template <typename...> class TMPL, typename T> struct element_type<TMPL<T>>  { using type = T; };
  template <typename T> using element_t = typename element_type<T>::type;

  // Customized type traits; for the moment, make sure that emp::Ptr is handled correctly.
  template <typename> struct is_ptr_type : public std::false_type { };
  template <typename T> struct is_ptr_type<T*> : public std::true_type { };
  template <typename T> struct is_ptr_type<T* const> : public std::true_type { };
  template <typename T> struct is_ptr_type<Ptr<T>> : public std::true_type { };
  template <typename T> constexpr bool is_ptr_type_v() { return is_ptr_type<T>::value; }
  template <typename T> constexpr bool is_ptr_type_v(const T&) { return is_ptr_type<T>::value; }
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
