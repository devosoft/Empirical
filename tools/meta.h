//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A bunch of C++ Template Meta-programming tricks.

#ifndef EMP_META_H
#define EMP_META_H


namespace emp {

  // Effectively create a function (via constructor) where all args are computed, then ignored.
  struct run_and_ignore { template <typename... T> run_and_ignore(T&&...) {} };

  // Trick to call a function using each entry in a parameter pack.
#define EMP_EXPAND_PPACK(PPACK) ::emp::run_and_ignore{ 0, ((PPACK), void(), 0)... }

  // The following functions take a test type and a list of types and return the index that
  // matches the test type in question.
  template <typename TEST_TYPE>
  constexpr int get_type_index() {
    // @CAO We don't have a type that matches, so ideally trigger a compile time error.
    // Given we need this to be constexpr, we can't easily put even a static assert here
    // until we require C++14.
    // static_assert(false && "trying to find index of non-existant type");
    return -1000000;
  }
  template <typename TEST_TYPE, typename FIRST_TYPE, typename... TYPE_LIST>
  constexpr int get_type_index() {
    return (std::is_same<TEST_TYPE, FIRST_TYPE>()) ? 0 : (get_type_index<TEST_TYPE,TYPE_LIST...>() + 1);
  }


  // These functions can be used to test if a type-set has all unique types or not.

  // Base cases...
  template <typename TYPE1> constexpr bool has_unique_first_type() { return true; }
  template <typename TYPE1> constexpr bool has_unique_types() { return true; }

  template <typename TYPE1, typename TYPE2, typename... TYPE_LIST>
  constexpr bool has_unique_first_type() {
    return (!std::is_same<TYPE1, TYPE2>()) && emp::has_unique_first_type<TYPE1, TYPE_LIST...>();
  }

  template <typename TYPE1, typename TYPE2, typename... TYPE_LIST>
  constexpr bool has_unique_types() {
    return has_unique_first_type<TYPE1, TYPE2, TYPE_LIST...>()  // Check first against all others...
      && has_unique_types<TYPE2, TYPE_LIST...>();               // Recurse through other types.
  }


  // Apply a tuple as arguments to a function!

  // implementation details, users never invoke these directly
  // Based on Kerrek SB in http://stackoverflow.com/questions/10766112/c11-i-can-go-from-multiple-args-to-tuple-but-can-i-go-from-tuple-to-multiple

  namespace internal {
    template <typename FUN_TYPE, typename TUPLE_TYPE, bool is_done, int TOTAL, int... N>
    struct apply_impl {
      static void apply(FUN_TYPE fun, TUPLE_TYPE && t) {
        apply_impl<FUN_TYPE, TUPLE_TYPE, TOTAL == 1 + sizeof...(N), TOTAL, N..., sizeof...(N)>::apply(fun, std::forward<TUPLE_TYPE>(t));
      }
    };

    template <typename FUN_TYPE, typename TUPLE_TYPE, int TOTAL, int... N>
    struct apply_impl<FUN_TYPE, TUPLE_TYPE, true, TOTAL, N...> {
      static void apply(FUN_TYPE fun, TUPLE_TYPE && t) {
        fun(std::get<N>(std::forward<TUPLE_TYPE>(t))...);
      }
    };
  }

  // user invokes this
  template <typename FUN_TYPE, typename TUPLE_TYPE>
  void ApplyTuple(FUN_TYPE fun, TUPLE_TYPE && tuple) {
    typedef typename std::decay<TUPLE_TYPE>::type TUPLE_DECAY_TYPE;
    internal::apply_impl<FUN_TYPE, TUPLE_TYPE, 0 == std::tuple_size<TUPLE_DECAY_TYPE>::value, std::tuple_size<TUPLE_DECAY_TYPE>::value>::apply(fun, std::forward<TUPLE_TYPE>(tuple));
  }


  // The following template takes two parameters; the real type you want it to be and a decoy
  // type that should just be evaluated for use in SFINAE.
  // To use: typename sfinae_decoy<X,Y>::type
  // This will always evaluate to X no matter what Y is.
  template <typename REAL_TYPE, typename EVAL_TYPE>
  struct sfinae_decoy { using type = REAL_TYPE; };

  // Most commonly we will use a decoy to determine if a member exists, but be treated as a
  // bool value.

#define emp_bool_decoy(TEST) typename emp::sfinae_decoy<bool, decltype(TEST)>::type
#define emp_int_decoy(TEST) typename emp::sfinae_decoy<int, decltype(TEST)>::type

  // Change the internal type arguments on a template...
  // From: Sam Varshavchik
  // http://stackoverflow.com/questions/36511990/is-it-possible-to-disentangle-a-template-from-its-arguments-in-c
  namespace internal {
    template<typename T, typename ...U> class AdaptTemplateHelper {
    public:
      using type = T;
    };

    template<template <typename...> class T, typename... V, typename... U>
    class AdaptTemplateHelper<T<V...>, U...> {
    public:
      using type = T<U...>;
    };
  }

  template<typename T, typename... U>
  using AdaptTemplate=typename internal::AdaptTemplateHelper<T, U...>::type;


  // Variation of AdaptTemplate that only adapts first template argument.
  namespace internal {
    template<typename T, typename U> class AdaptTemplateHelper_Arg1 {
    public:
      using type = T;
    };

    template<template <typename...> class T, typename X, typename ...V, typename U>
    class AdaptTemplateHelper_Arg1<T<X, V...>, U> {
    public:
      using type = T<U, V...>;
    };
  }

  template<typename T, typename U>
  using AdaptTemplate_Arg1=typename internal::AdaptTemplateHelper_Arg1<T, U>::type;

}


#endif
