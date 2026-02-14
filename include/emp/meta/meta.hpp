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
#include <stddef.h>
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
    if constexpr (sizeof...(Ts) > 1) return has_unique_types<Ts...>();
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


 






  /// Group types in a parameter pack to build a vector of a designated type.
  template <typename OBJ_T>
  void BuildObjVector1(emp::vector<OBJ_T> &) {}

  template <typename OBJ_T>
  void BuildObjVector2(emp::vector<OBJ_T> &) {}

  template <typename OBJ_T>
  void BuildObjVector3(emp::vector<OBJ_T> &) {}

  template <typename OBJ_T>
  void BuildObjVector4(emp::vector<OBJ_T> &) {}

  template <typename OBJ_T, typename T1, typename... Ts>
  void BuildObjVector1(emp::vector<OBJ_T> & v, T1 & arg1, Ts &... extras) {
    v.emplace_back(arg1);
    BuildObjVector1(v, extras...);
  }

  template <typename OBJ_T, typename T1, typename T2, typename... Ts>
  void BuildObjVector2(emp::vector<OBJ_T> & v, T1 & arg1, T2 & arg2, Ts &... extras) {
    v.emplace_back(arg1, arg2);
    BuildObjVector2(v, extras...);
  }

  template <typename OBJ_T, typename T1, typename T2, typename T3, typename... Ts>
  void BuildObjVector3(emp::vector<OBJ_T> & v, T1 & arg1, T2 & arg2, T3 & arg3, Ts &... extras) {
    v.emplace_back(arg1, arg2, arg3);
    BuildObjVector3(v, extras...);
  }

  template <typename OBJ_T, typename T1, typename T2, typename T3, typename T4, typename... Ts>
  void BuildObjVector4(emp::vector<OBJ_T> & v,
                       T1 & arg1,
                       T2 & arg2,
                       T3 & arg3,
                       T4 & arg4,
                       Ts &... extras) {
    v.emplace_back(arg1, arg2, arg3, arg4);
    BuildObjVector4(v, extras...);
  }

  template <typename OBJ_T, size_t NUM_ARGS, typename... Ts>
  emp::vector<OBJ_T> BuildObjVector(Ts &... args) {
    emp::vector<OBJ_T> out_v;
    constexpr size_t TOTAL_ARGS = sizeof...(Ts);
    static_assert((TOTAL_ARGS % NUM_ARGS) == 0,
                  "emp::BuildObjVector() : Must have the same number of args for each object.");
    out_v.reserve(TOTAL_ARGS / NUM_ARGS);

    if constexpr (NUM_ARGS == 1) {
      BuildObjVector1<OBJ_T>(out_v, args...);
    } else if constexpr (NUM_ARGS == 2) {
      BuildObjVector2<OBJ_T>(out_v, args...);
    } else if constexpr (NUM_ARGS == 3) {
      BuildObjVector3<OBJ_T>(out_v, args...);
    } else if constexpr (NUM_ARGS == 4) {
      BuildObjVector4<OBJ_T>(out_v, args...);
    }
    static_assert(NUM_ARGS < 5, "BuildObjVector currently has a cap of 4 arguments per object.");

    return out_v;
  }


  // sfinae_decoy<X,Y> will always evaluate to X no matter what Y is.
  // X is type you want it to be; Y is a decoy trigger potential substitution failure.
  template <typename REAL_TYPE, typename EVAL_TYPE>
  using sfinae_decoy = REAL_TYPE;
  template <typename REAL_TYPE, typename EVAL_TYPE>
  using type_decoy = REAL_TYPE;
  template <typename REAL_TYPE, typename EVAL_TYPE>
  using decoy_t = REAL_TYPE;
  template <typename EVAL_TYPE>
  using bool_decoy = bool;
  template <typename EVAL_TYPE>
  using int_decoy = int;

  // Deprecated macros
#define emp_bool_decoy(TEST) emp::sfinae_decoy<bool, decltype(TEST)>
#define emp_int_decoy(TEST) emp::sfinae_decoy<int, decltype(TEST)>




// TruncateCall reduces the number of arguments for calling a function if too many are used.
// @CAO: This should be simplified using TypeSet
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <typename... PARAMS>
    struct tcall_impl {
      template <typename FUN_T, typename... EXTRA>
      static auto call(FUN_T fun, PARAMS... args, EXTRA...) {
        return fun(args...);
      }
    };
  }  // namespace internal
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

  // Truncate the arguments provided, using only the relevant ones for a function call.
  template <typename R, typename... PARAMS, typename... ARGS>
  auto TruncateCall(std::function<R(PARAMS...)> fun, ARGS &&... args) {
    return internal::tcall_impl<PARAMS...>::call(fun, std::forward<ARGS>(args)...);
  }

  // Expand a function to take (and ignore) extra arguments.
  template <typename R, typename... ARGS>
  struct AdaptFunction {
    template <typename... EXTRA_ARGS>
    static auto Expand(const std::function<R(ARGS...)> & fun) {
      return [fun](ARGS... args, EXTRA_ARGS...) { return fun(args...); };
    }
  };

// Change the internal type arguments on a template...
// Adapted from: Sam Varshavchik
// http://stackoverflow.com/questions/36511990/is-it-possible-to-disentangle-a-template-from-its-arguments-in-c
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <typename T, typename... U>
    struct AdaptTemplateHelper {
      using type = T;
    };

    template <template <typename...> class T, typename... V, typename... U>
    struct AdaptTemplateHelper<T<V...>, U...> {
      using type = T<U...>;
    };
  }  // namespace internal
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

  template <typename T, typename... U>
  using AdaptTemplate = typename internal::AdaptTemplateHelper<T, U...>::type;


// Variation of AdaptTemplate that only adapts first template argument.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <typename T, typename U>
    class AdaptTemplateHelper_Arg1 {
    public:
      using type = T;
    };

    template <template <typename...> class T, typename X, typename... V, typename U>
    class AdaptTemplateHelper_Arg1<T<X, V...>, U> {
    public:
      using type = T<U, V...>;
    };
  }  // namespace internal
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

  template <typename T, typename U>
  using AdaptTemplate_Arg1 = typename internal::AdaptTemplateHelper_Arg1<T, U>::type;

  // Some math inside templates...
  template <int I, int... Is>
  struct tIntMath {
    static constexpr int Sum() { return I + tIntMath<Is...>::Sum(); }

    static constexpr int Product() { return I * tIntMath<Is...>::Product(); }
  };

  template <int I>
  struct tIntMath<I> {
    static constexpr int Sum() { return I; }

    static constexpr int Product() { return I; }
  };

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  // This bit of magic is from
  // http://meh.schizofreni.co/programming/magic/2013/01/23/function-pointer-from-lambda.html
  // and is useful for fixing lambda function woes
  template <typename Function>
  struct function_traits : public function_traits<decltype(&Function::operator())> {};

  template <typename ClassType, typename ReturnType, typename... Args>
  struct function_traits<ReturnType (ClassType::*)(Args...) const> {
    typedef ReturnType (*pointer)(Args...);
    typedef std::function<ReturnType(Args...)> function;
  };

  template <typename Function>
  typename function_traits<Function>::pointer to_function_pointer(Function & lambda) {
    return static_cast<typename function_traits<Function>::pointer>(lambda);
  }

  template <typename Function>
  typename function_traits<Function>::function to_function(Function & lambda) {
    return static_cast<typename function_traits<Function>::function>(lambda);
  }

#endif /*DOXYGEN_SHOULD_SKIP_THIS*/  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

  /// Determine the size of a built-in array.
  template <typename T, size_t N>
  constexpr size_t GetSize(T (&)[N]) {
    return N;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_META_META_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: schizofreni Varshavchik tcall
