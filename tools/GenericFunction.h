//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Based on std::function, but with a common base class.
//
//  Developer notes:
//  * Deal with function return values.

#ifndef EMP_GENERIC_FUNCTION_H
#define EMP_GENERIC_FUNCTION_H

#include <functional>

namespace emp {

  class GenericFunction {
  protected:
  public:
    virtual ~GenericFunction() { ; }

    // A generic form of the function call operator; use arg types to determine derived form.
    template <typename RETURN, typename... Ts> auto Call(Ts &&... args);
    template <typename RETURN, typename... Ts> auto operator()(Ts &&... args) {
      return Call<RETURN, Ts...>( std::forward<Ts>(args)... );
    }
  };

  // Undefined bast type for Function
  template <typename... Ts> class Function;

  // Specialized form for proper function types.
  template <typename RETURN, typename... PARAMS>
  class Function<RETURN(PARAMS...)> : public GenericFunction {
  protected:
    std::function<RETURN(PARAMS...)> fun;
  public:
    // Forward all args to std::function constructor...
    template <typename... Ts>
    Function(Ts &&... args) : fun(std::forward<Ts>(args)...) { ; }

    // Forward all args to std::function call.
    template <typename... Ts>
    RETURN Call(Ts &&... args) { return fun(std::forward<Ts>(args)...); }
    template <typename... Ts>
    RETURN operator()(Ts &&... args) { return fun(std::forward<Ts>(args)...); }
  };

  template <typename RETURN, typename... Ts>
  auto GenericFunction::Call(Ts &&... args) {
    using fun_t = Function<RETURN(Ts...)>;
    fun_t * fun = (fun_t *) this;
    return fun->Call( std::forward<Ts>(args)... );
  }
}

#endif
