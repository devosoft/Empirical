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
  };

  template <typename RETURN, typename... PARAMS>
  class Function : public GenericFunction {
  protected:
    std::function<RETURN(PARAMS...)> fun;
  public:
    // Forward all args to std::function constructor...
    template <typename... Ts>
    Function(Ts &&... args) : fun(std::forward<Ts>(args)...) { ; }

    // Forward all args to std::function call.
    template <typename... Ts>
    RETURN operator()(Ts &&... args) { return fun(std::forward<Ts>(args)...); }
  };

}

#endif
