/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file meta.hpp
 *  @brief A bunch of C++ Template Meta-programming tricks.
 *
 *  Developer notes:
 *  * Right now test_type<> returns false if a template can't resolve, but if it's true it checks
 *    if a value field is present; if so that determines success.  The reason for this choice was
 *    to make sure that true_type and false_type are handled correctly (with built-in type_tratis)
 */

#ifndef EMP_META_META_HPP_INCLUDE
#define EMP_META_META_HPP_INCLUDE

#include <functional>
#include <tuple>
#include <utility>

#include "../base/vector.hpp"

namespace emp {

  /// A function that will take any number of argument and do nothing with them.
  template <typename... Ts> void DoNothing(Ts...) { ; }

  /// Effectively create a function (via constructor) where all args are computed, then ignored.
  struct run_and_ignore { template <typename... T> run_and_ignore(T&&...) {} };

  /// Trim off a specific type position from a pack.
  template <typename T1, typename... Ts> using first_type = T1;
  template <typename T1, typename T2, typename... Ts> using second_type = T2;
  template <typename T1, typename T2, typename T3, typename... Ts> using third_type = T3;

  /// Create a placeholder template to substitute for a real type.
  template <int> struct PlaceholderType;

  /// Group types in a parameter pack to build a vector of a designated type.
  template <typename OBJ_T> void BuildObjVector1(emp::vector<OBJ_T> &) { }
  template <typename OBJ_T> void BuildObjVector2(emp::vector<OBJ_T> &) { }
  template <typename OBJ_T> void BuildObjVector3(emp::vector<OBJ_T> &) { }
  template <typename OBJ_T> void BuildObjVector4(emp::vector<OBJ_T> &) { }

  template <typename OBJ_T, typename T1, typename... Ts>
  void BuildObjVector1(emp::vector<OBJ_T>& v, T1& arg1, Ts&... extras)
  { v.emplace_back( arg1 ); BuildObjVector1(v, extras...); }

  template <typename OBJ_T, typename T1, typename T2, typename... Ts>
  void BuildObjVector2(emp::vector<OBJ_T>& v, T1& arg1, T2& arg2, Ts&... extras)
  { v.emplace_back( arg1, arg2 ); BuildObjVector2(v, extras...); }

  template <typename OBJ_T, typename T1, typename T2, typename T3, typename... Ts>
  void BuildObjVector3(emp::vector<OBJ_T>& v, T1& arg1, T2& arg2, T3& arg3, Ts&... extras)
  { v.emplace_back( arg1, arg2, arg3 ); BuildObjVector3(v, extras...); }

  template <typename OBJ_T, typename T1, typename T2, typename T3, typename T4, typename... Ts>
  void BuildObjVector4(emp::vector<OBJ_T>& v, T1& arg1, T2& arg2, T3& arg3, T4& arg4, Ts&... extras)
  { v.emplace_back( arg1, arg2, arg3, arg4 ); BuildObjVector4(v, extras...); }

  template <typename OBJ_T, size_t NUM_ARGS, typename... Ts>
  emp::vector<OBJ_T> BuildObjVector(Ts &... args) {
    emp::vector<OBJ_T> out_v;
    constexpr size_t TOTAL_ARGS = sizeof...(Ts);
    static_assert((TOTAL_ARGS % NUM_ARGS) == 0,
                  "emp::BuildObjVector() : Must have the same number of args for each object.");
    out_v.reserve(TOTAL_ARGS / NUM_ARGS);

    if constexpr (NUM_ARGS == 1) BuildObjVector1<OBJ_T>(out_v, args...);
    else if constexpr (NUM_ARGS == 2) BuildObjVector2<OBJ_T>(out_v, args...);
    else if constexpr (NUM_ARGS == 3) BuildObjVector3<OBJ_T>(out_v, args...);
    else if constexpr (NUM_ARGS == 4) BuildObjVector4<OBJ_T>(out_v, args...);
    static_assert(NUM_ARGS < 5, "BuildObjVector currently has a cap of 4 arguments per object.");

    return out_v;
  }

  // Index into a template parameter pack to grab a specific type.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <size_t ID, typename T, typename... Ts>
    struct pack_id_impl { using type = typename pack_id_impl<ID-1,Ts...>::type; };

    template <typename T, typename... Ts>
    struct pack_id_impl<0,T,Ts...> { using type = T; };
  }

  template <size_t ID, typename... Ts>
  using pack_id = typename internal::pack_id_impl<ID,Ts...>::type;
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  // Trim off the last type from a pack.
  template <typename... Ts> using last_type = pack_id<sizeof...(Ts)-1,Ts...>;

  // Trick to call a function using each entry in a parameter pack.
#define EMP_EXPAND_PPACK(PPACK) ::emp::run_and_ignore{ 0, ((PPACK), void(), 0)... }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // Check to see if a specified type is part of a set of types.
  template <typename TEST> constexpr bool has_type() { return false; }
#endif
  template <typename TEST, typename FIRST, typename... OTHERS>
  constexpr bool has_type() { return std::is_same<TEST, FIRST>() || has_type<TEST,OTHERS...>(); }

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // Count how many times a specified type appears in a set of types.
  template <typename TEST> constexpr size_t count_type() { return 0; }
#endif
  template <typename TEST, typename FIRST, typename... OTHERS>
  constexpr size_t count_type() { return count_type<TEST,OTHERS...>() + (std::is_same<TEST, FIRST>()?1:0); }

  // Return the index of a test type in a set of types.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <typename TEST_T> constexpr int get_type_index_impl() { return -1; } // Not found!
    template <typename TEST_T, typename T1, typename... Ts>
    constexpr int get_type_index_impl() {
      if (std::is_same<TEST_T, T1>()) return 0;                     // Found here!
      constexpr int next_id = get_type_index_impl<TEST_T,Ts...>();  // Keep looking...
      if (next_id < 0) return -1;                                   // Not found!
      return next_id + 1;                                           // Found later!
    }
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS
  template <typename TEST_T, typename... Ts>
  constexpr int get_type_index() { return internal::get_type_index_impl<TEST_T, Ts...>(); }


  // These functions can be used to test if a type-set has all unique types or not.

  // Base cases...
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  template <typename TYPE1> constexpr bool has_unique_first_type() { return true; }
  template <typename TYPE1> constexpr bool has_unique_types() { return true; }
  #endif

  template <typename TYPE1, typename TYPE2, typename... TYPE_LIST>
  constexpr bool has_unique_first_type() {
    return (!std::is_same<TYPE1, TYPE2>()) && emp::has_unique_first_type<TYPE1, TYPE_LIST...>();
  }

  template <typename TYPE1, typename TYPE2, typename... TYPE_LIST>
  constexpr bool has_unique_types() {
    return has_unique_first_type<TYPE1, TYPE2, TYPE_LIST...>()  // Check first against all others...
      && has_unique_types<TYPE2, TYPE_LIST...>();               // Recurse through other types.
  }


  // sfinae_decoy<X,Y> will always evaluate to X no matter what Y is.
  // X is type you want it to be; Y is a decoy trigger potential substituion failue.
  template <typename REAL_TYPE, typename EVAL_TYPE> using sfinae_decoy = REAL_TYPE;
  template <typename REAL_TYPE, typename EVAL_TYPE> using type_decoy = REAL_TYPE;
  template <typename REAL_TYPE, typename EVAL_TYPE> using decoy_t = REAL_TYPE;
  template <typename EVAL_TYPE>                     using bool_decoy = bool;
  template <typename EVAL_TYPE>                     using int_decoy = int;

  // Deprecated macros
#define emp_bool_decoy(TEST) emp::sfinae_decoy<bool, decltype(TEST)>
#define emp_int_decoy(TEST) emp::sfinae_decoy<int, decltype(TEST)>


  // constexpr bool test_type<TEST,T>() returns true if T passes the TEST, false otherwise.
  //
  // TEST is a template.  TEST will fail if TEST<T> fails to resolve (substitution failure) -OR-
  // if TEST<T> does resolve, but TEST<T>::value == false.  Otherwise the test passes.
  //
  // Two helper functions exist to test each part: test_type_exist and test_type_value.

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <template <typename...> class FILTER, typename T>
    constexpr bool tt_exist_impl(bool_decoy<FILTER<T>> ) { return true; }
    template <template <typename...> class FILTER, typename T>
    constexpr bool tt_exist_impl(...) { return false; }
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  template <template <typename...> class TEST, typename T>
  constexpr bool test_type_exist() { return internal::tt_exist_impl<TEST, T>(true); }

  template <template <typename...> class TEST, typename T>
  constexpr bool test_type_value() { return TEST<T>::value; }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    // If a test does have a value field, that value determines success.
    template <typename RESULT, bool value_exist>
    struct test_type_v_impl { constexpr static bool Test() { return RESULT::value; } };
    // If a test does not have a value field, the fact it resolved at all indicates success.
    template <typename RESULT>
    struct test_type_v_impl<RESULT,0> { constexpr static bool Test() { return true; } };

    template <typename T> using value_member = decltype(T::value);
    // If TEST<T> *does* resolve, check the value field to determine test success.
    template <template <typename...> class TEST, typename T, bool exist>
    struct test_type_e_impl {
      constexpr static bool Test() {
        using result_t = TEST<T>;
        constexpr bool has_value = test_type_exist<value_member, result_t>();
        return test_type_v_impl<result_t,has_value>::Test();
      }
    };
    // If TEST<T> does *not* resolve, test fails, so return false.
    template <template <typename...> class TEST, typename T>
    struct test_type_e_impl<TEST,T,0> { constexpr static bool Test() { return false; } };
  }
  #endif // DOXYGEN_SHOULD_SKIP THIS

  // Function to actually perform a universal test.
  template <template <typename...> class TEST, typename T>
  constexpr bool test_type() {
    return internal::test_type_e_impl<TEST,T,test_type_exist<TEST,T>()>::Test();
  }


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
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

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
      return [fun](ARGS... args, EXTRA_ARGS...){ return fun(args...); };
    }
  };


  // Change the internal type arguments on a template...
  // Adapted from: Sam Varshavchik
  // http://stackoverflow.com/questions/36511990/is-it-possible-to-disentangle-a-template-from-its-arguments-in-c
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template<typename T, typename ...U> struct AdaptTemplateHelper {
      using type = T;
    };

    template<template <typename...> class T, typename... V, typename... U>
    struct AdaptTemplateHelper<T<V...>, U...> {
      using type = T<U...>;
    };
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  template<typename T, typename... U>
  using AdaptTemplate = typename internal::AdaptTemplateHelper<T, U...>::type;


  // Variation of AdaptTemplate that only adapts first template argument.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
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
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  template<typename T, typename U>
  using AdaptTemplate_Arg1 = typename internal::AdaptTemplateHelper_Arg1<T, U>::type;


  // Some math inside templates...
  template <int I, int... Is> struct tIntMath {
    static constexpr int Sum() { return I + tIntMath<Is...>::Sum(); }
    static constexpr int Product() { return I * tIntMath<Is...>::Product(); }
  };

  template <int I> struct tIntMath<I> {
    static constexpr int Sum() { return I; }
    static constexpr int Product() { return I; }
  };

  //This bit of magic is from
  //http://meh.schizofreni.co/programming/magic/2013/01/23/function-pointer-from-lambda.html
  //and is useful for fixing lambda function woes
  template <typename Function>
  struct function_traits
    : public function_traits<decltype(&Function::operator())>
  {};

  template <typename ClassType, typename ReturnType, typename... Args>
  struct function_traits<ReturnType(ClassType::*)(Args...) const>
  {
    typedef ReturnType (*pointer)(Args...);
    typedef std::function<ReturnType(Args...)> function;
  };

  template <typename Function>
  typename function_traits<Function>::pointer
  to_function_pointer (Function& lambda)
  {
    return static_cast<typename function_traits<Function>::pointer>(lambda);
  }

  template <typename Function>
  typename function_traits<Function>::function
  to_function (Function& lambda)
  {
    return static_cast<typename function_traits<Function>::function>(lambda);
  }

  /// Determine the size of a built-in array.
  template <typename T, size_t N>
  constexpr size_t GetSize(T (&)[N]) { return N; }

}

#endif // #ifndef EMP_META_META_HPP_INCLUDE
