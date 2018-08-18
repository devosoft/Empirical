/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  GenericFunction.h
 *  @brief Based on std::function, but with a common base class.
 *  @note Status: ALPHA
 *
 *  @todo Need to setup Call on emp::GenericFunction to just take a function signature as a
 *    template argument, rather than listing all types.
 */


#ifndef EMP_GENERIC_FUNCTION_H
#define EMP_GENERIC_FUNCTION_H

#include <functional>
#include "../base/assert.h"

namespace emp {

  ///  The emp::Function templated class behaves almost identically to std::function, but can be
  ///  reduced to the emp::GenericFunction base class which is NOT templated.
  ///
  ///  An emp::GenericFunction object can be converted back into the derived type with the
  ///  .Convert<return(args...)>() member function.

  class GenericFunction {
  protected:
  public:
    virtual ~GenericFunction() { ; }

    /// A generic form of the function call operator; use arg types to determine derived form.
    template <typename RETURN, typename... Ts> auto Call(Ts &&... args);

    /// Test if a function call will succeed before trying it.
    template <typename RETURN, typename... Ts> bool CallOK(Ts &&...);

    /// Test if a function call will succeed before trying it, based only on types.
    template <typename RETURN, typename... Ts> bool CallTypeOK();

    /// A generic form of the function call operator; use arg types to determine derived form.
    template <typename RETURN, typename... Ts> auto operator()(Ts &&... args) {
      return Call<RETURN, Ts...>( std::forward<Ts>(args)... );
    }

    /// Convert this GenericFunction into a derived emp::Function
    template <typename T> auto Convert();

    /// Determine if this GenericFunction can be converted into a derived emp::Function
    template <typename T> bool ConvertOK();
  };

  // Undefined base type for Function, to create an error if a function type is not passed in.
  template <typename... Ts> class Function;

  // Specialized form for proper function types.
  template <typename RETURN, typename... PARAMS>
  class Function<RETURN(PARAMS...)> : public GenericFunction {
  protected:
    using fun_t = std::function<RETURN(PARAMS...)>;
    fun_t fun;  ///< The std::function to be called.
  public:
    /// Forward all args to std::function constructor...
    template <typename... Ts>
    Function(Ts &&... args) : fun(std::forward<Ts>(args)...) { ; }

    /// Forward all args to std::function call.
    template <typename... Ts>
    RETURN Call(Ts &&... args) { return fun(std::forward<Ts>(args)...); }

    /// Forward all args to std::function call.
    template <typename... Ts>
    RETURN operator()(Ts &&... args) { return fun(std::forward<Ts>(args)...); }

    /// Get the std::function to be called.
    const fun_t & GetFunction() const { return fun; }
  };

  template <typename RETURN, typename... Ts>
  auto GenericFunction::Call(Ts &&... args) {
    using fun_t = Function<RETURN(Ts...)>;

    emp_assert(dynamic_cast<fun_t *>(this));  // Make sure this Call cast is legal.

    fun_t * fun = (fun_t *) this;
    return fun->Call( std::forward<Ts>(args)... );
  }

  template <typename RETURN, typename... Ts>
  bool GenericFunction::CallOK(Ts &&...) {
    using fun_t = Function<RETURN(Ts...)>;
    return dynamic_cast<fun_t *>(this);    
  }

  template <typename RETURN, typename... Ts>
  bool GenericFunction::CallTypeOK() {
    using fun_t = Function<RETURN(Ts...)>;
    return dynamic_cast<fun_t *>(this);    
  }

  template <typename T> auto GenericFunction::Convert() {
    emp_assert(dynamic_cast<Function<T> *>(this));
    return (Function<T> *) this;
  }

  template <typename T> bool GenericFunction::ConvertOK() {
    return dynamic_cast<Function<T> *>(this);
  }

}

#endif
