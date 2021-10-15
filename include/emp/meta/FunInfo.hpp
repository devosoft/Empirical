/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file FunInfo.hpp
 *  @brief Wrap a function to provide more information about it.
 *  @note Status: ALPHA
 * 
 *  FunInfo will collect information about a provided function and facilitate
 *  manipulations.
 * 
 * 
 *  Developer Notes:
 *  - Need to setup BindAt to choose bind position(s).
 *    For example:  fun_info::BindAt<2,4>(my_fun, 12, "abc")
 */

#ifndef EMP_FUN_INFO_HPP
#define EMP_FUN_INFO_HPP

#include <functional>

#include "TypePack.hpp"

namespace emp {

  // A generic base class that expands anything with operator()
  template <typename T>
  struct FunInfo : public FunInfo< decltype(&T::operator()) > {};

  // Specialization for functions; redirect to function-object specialization.
  template <typename RETURN_T, typename... PARAM_Ts>
  struct FunInfo <RETURN_T(PARAM_Ts...)>
  : public FunInfo< std::function<RETURN_T(PARAM_Ts...)> > {};

  // Specialization for functions; redirect to function-object specialization.
  template <typename RETURN_T, typename... PARAM_Ts>
  struct FunInfo <RETURN_T(*)(PARAM_Ts...)>
  : public FunInfo< std::function<RETURN_T(PARAM_Ts...)> > {};


  // Specialization for function objects with AT LEAST ONE parameter...
  template <typename CLASS_T, typename RETURN_T, typename PARAM1_T, typename... PARAM_Ts>
  struct FunInfo <RETURN_T(CLASS_T::*)(PARAM1_T, PARAM_Ts...) const>
  {
    using return_t = RETURN_T;
    using param_t = TypePack<PARAM1_T, PARAM_Ts...>;

    template <size_t ID>
    using arg_t = typename param_t::template get<ID>;

    static constexpr size_t NumArgs() { return 1 + sizeof...(PARAM_Ts); }

    /// Test if this function can be called with a particular set of arguments.
    template <typename ARG1, typename... ARG_Ts>
    static constexpr bool InvocableWith(ARG1 &&, ARG_Ts &&...) {
      return std::is_invocable<CLASS_T, ARG1, ARG_Ts...>();
    }

    /// Test if this function can be called with a particular set of argument TYPEs.
    template <typename... ARG_Ts>
    static constexpr bool InvocableWith() {
      return std::is_invocable<CLASS_T, ARG_Ts...>();
    }

    template <typename FUN_T, typename CONVERTER_T>
    static auto WrapReturnType(FUN_T fun, CONVERTER_T convert_fun)
    {
      return [fun=fun, c=convert_fun](PARAM1_T && arg1, PARAM_Ts &&... args) {
        return c( fun(std::forward<PARAM1_T>(arg1), std::forward<PARAM_Ts>(args)...) );
      };
    }

    template <typename T>
    static auto BindFirst(CLASS_T fun, T && bound) {
      // If the function needs a reference for the parameter, send the supplied value through.
      if constexpr (std::is_reference<PARAM1_T>()) {
        return [fun, &bound](PARAM_Ts &&... args) {
          return fun(std::forward<T>(bound), std::forward<PARAM_Ts>(args)...);
        };
      }
      // Otherwise, a copy is fine.
      else {
        return [fun, bound](PARAM_Ts &&... args) {
          return fun(std::forward<T>(bound), std::forward<PARAM_Ts>(args)...);
        };
      }
    }

    // template <size_t... POS, typename... Ts>
    // static auto BindAt(CLASS_T fun, Ts &&... bound) {
    //   return [fun, bound](PARAM_Ts... args) { return fun(bound, args...); };
    // }

  };

  // Specialization for function objects with NO parameters...
  template <typename CLASS_T, typename RETURN_T>
  struct FunInfo <RETURN_T(CLASS_T::*)() const>
  {
    using return_t = RETURN_T;
    using param_t = TypePack<>;

    static constexpr size_t NumArgs() { return 0; }

    /// Test if this function can be called with a particular set of arguments.
    template <typename... ARG_Ts>
    static constexpr bool InvocableWith(ARG_Ts...) { return sizeof...(ARG_Ts) == 0; }

    template <typename FUN_T, typename CONVERTER_T>
    static auto WrapReturnType(FUN_T fun, CONVERTER_T convert_fun)
    {
      return [fun=fun, c=convert_fun]() {
        return c(fun());
      };
    }

  };


  // === Stand-alone helper functions ===

  template <typename FUN_T, typename CONVERTER_T>
  static auto WrapReturnType(FUN_T fun, CONVERTER_T convert_fun)
  {
    return FunInfo<FUN_T>::WrapReturnType(fun, convert_fun);
  }

  /// Lock in the first argument of a function.
  template <typename FUN_T, typename T>
  auto BindFirst(FUN_T fun, T && bound) {
    return FunInfo<FUN_T>::BindFirst(fun, std::forward<T>(bound));
  }

}

#endif
