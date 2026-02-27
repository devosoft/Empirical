/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/meta/meta.hpp
 * @brief A bunch of C++ Template Meta-programming tricks.
 *
 * Developer notes:
 * * Right now test_type<> returns false if a template can't resolve, but if it's true it checks
 *   if a value field is present; if so that determines success.  The reason for this choice was
 *   to make sure that true_type and false_type are handled correctly (with built-in type_traits)
 */

#pragma once

#ifndef INCLUDE_EMP_META_META_HPP_GUARD
#define INCLUDE_EMP_META_META_HPP_GUARD

#include <functional>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../base/vector.hpp"

namespace emp {

  /// Take any number of arguments and do nothing (force evaluation of args when passed by value).
  template <typename... Ts>
  constexpr void DoNothing(Ts &&...) noexcept { }

  /// Force evaluation of constructor arguments; useful for pack-expansion side effects.
  struct run_and_ignore {
    template <typename... Ts>
    constexpr explicit run_and_ignore(Ts &&...) noexcept {}
  };

  /// A struct declaration with no definition to show a type name in a compile time error.
  template <typename...>
  struct ShowType;

  /// A false type that does NOT resolve in unexecuted if-constexpr branches.
  /// By Brian Bi; from: https://stackoverflow.com/questions/69501472/best-way-to-trigger-a-compile-time-error-if-no-if-constexprs-succeed
  template <class T>
  struct dependent_false : std::false_type {};


  // === Pack indexing helpers ===

  /// Pick a specific position from a type pack.
  template <std::size_t I, typename... Ts>
  using pack_id = std::tuple_element_t<I, std::tuple<Ts...>>;

  /// First N types from a pack, as convenience aliases
  template <typename... Ts> using first_type  = pack_id<0, Ts...>;
  template <typename... Ts> using second_type = pack_id<1, Ts...>;
  template <typename... Ts> using third_type  = pack_id<2, Ts...>;
  template <typename... Ts> using fourth_type = pack_id<3, Ts...>;

  template <typename... Ts> using last_type = pack_id<sizeof...(Ts)-1, Ts...>;


  // === Pack membership / counting / uniqueness ===

  template <typename Test, typename... Ts>
  inline constexpr bool has_type_v = (std::same_as<Test, Ts> || ...);

  template <typename Test, typename... Ts>
  constexpr bool has_type() noexcept { return has_type_v<Test, Ts...>; }

  template <typename Test, typename... Ts>
  inline constexpr std::size_t count_type_v = (std::size_t{0} + ... + (std::same_as<Test, Ts> ? 1u : 0u));

  template <typename Test, typename... Ts>
  constexpr std::size_t count_type() noexcept { return count_type_v<Test, Ts...>; }

  template <typename T1, typename... Ts>
  constexpr bool has_unique_types() noexcept {
    if constexpr (count_type<T1, Ts...>() > 0) return false; // T1 was found later!
    if constexpr (sizeof...(Ts) > 0) return has_unique_types<Ts...>();
    return true;
  }


  // === Pack indexing ===

  namespace internal {
    template <typename Test, typename... Ts>
    consteval int get_type_index_impl() noexcept {
      int idx = 0;
      bool found = false;

      // consteval loop over the pack with a fold:
      ((found = found || std::same_as<Test, Ts>, idx += found ? 0 : 1), ...);

      return found ? idx : -1;
    }
  }  // namespace internal

  template <typename Test, typename... Ts>
  consteval int get_type_index() noexcept {
    if constexpr (sizeof...(Ts) == 0) return -1;
    else return internal::get_type_index_impl<Test, Ts...>();
  }


  //  ===  Detection / "test_type" utilities  ===

  // - If TEST<T...> is ill-formed: test fails (false).
  // - If TEST<T...> is well-formed:
  //     - If TEST<T...>::value exists: result is bool(TEST<T...>::value)
  //     - Otherwise: existence alone counts as success (true)

  template <typename T>
  concept has_value_member = requires { T::value; };

  template <template <typename...> class TEST, typename... Args>
  concept template_well_formed = requires { typename TEST<Args...>; };

  template <template <typename...> class TEST, typename... Args>
  consteval bool test_type_exist() noexcept {
    return template_well_formed<TEST, Args...>;
  }

  template <template <typename...> class TEST, typename... Args>
  consteval bool test_type_value() noexcept
    requires template_well_formed<TEST, Args...> && has_value_member<TEST<Args...>>
  {
    return static_cast<bool>(TEST<Args...>::value);
  }

  template <template <typename...> class TEST, typename... Args>
  consteval bool test_type() noexcept {
    if constexpr (!template_well_formed<TEST, Args...>) return false;
    else if constexpr (has_value_member< TEST<Args...> >) {
      return static_cast<bool>( TEST<Args...> ::value);
    }
    else return true;
  }

  /// Determine the size of a built-in array.
  template <typename T, std::size_t N>
  constexpr std::size_t GetSize(T (&)[N]) {
    return N;
  }


  // === Decoy templates for SFINAE (should phase out in favor of concepts and if constexpr!) ===

  // sfinae_decoy<X,Y> will always evaluate to X no matter what Y is.
  // X is type you want it to be; Y is a decoy trigger potential substitution failure.
  template <typename REAL_TYPE, typename EVAL_TYPE>
  using sfinae_decoy = REAL_TYPE;
  template <typename REAL_TYPE, typename EVAL_TYPE>
  using decoy_t = REAL_TYPE;
  template <typename EVAL_TYPE>
  using bool_decoy = bool;
  template <typename EVAL_TYPE>
  using int_decoy = int;


  //  === Other utilities ===
  
  /// BuildObjVector groups arguments into objects and return emp::vector<OBJ_T>

  namespace internal {
    template <typename OBJ_T, std::size_t NUM_ARGS, std::size_t BASE, typename Tuple, std::size_t... Is>
    void emplace_from_tuple(emp::vector<OBJ_T> & out, Tuple && tup, std::index_sequence<Is...>) {
      out.emplace_back(std::get<BASE + Is>(std::forward<Tuple>(tup))...);
    }

    template <typename OBJ_T, std::size_t NUM_ARGS, typename Tuple, std::size_t... ChunkIs>
    void build_obj_vector_from_tuple(emp::vector<OBJ_T> & out, Tuple && tup, std::index_sequence<ChunkIs...>) {
      // For each chunk k, BASE = k * NUM_ARGS
      (emplace_from_tuple<OBJ_T, NUM_ARGS, ChunkIs * NUM_ARGS>(
        out, std::forward<Tuple>(tup), std::make_index_sequence<NUM_ARGS>{}
      ), ...);
    }
  }  // namespace internal

  template <typename OBJ_T, std::size_t NUM_ARGS, typename... Args>
  emp::vector<OBJ_T> BuildObjVector(Args &&... args) {
    static_assert(NUM_ARGS >= 1);
    constexpr std::size_t TOTAL_ARGS = sizeof...(Args);
    static_assert((TOTAL_ARGS % NUM_ARGS) == 0,
                  "emp::BuildObjVector(): Must have the same number of args for each object.");

    auto tup = std::forward_as_tuple(std::forward<Args>(args)...);

    emp::vector<OBJ_T> out;
    out.reserve(TOTAL_ARGS / NUM_ARGS);

    internal::build_obj_vector_from_tuple<OBJ_T, NUM_ARGS>(
      out, tup, std::make_index_sequence<TOTAL_ARGS / NUM_ARGS>{}
    );

    return out;
  }
}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_META_META_HPP_GUARD
