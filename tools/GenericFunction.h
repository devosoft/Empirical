//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Based on std::function, but with a common base class.

#ifndef EMP_GENERIC_FUNCTION_H
#define EMP_GENERIC_FUNCTION_H

#include <functional>

namespace emp {

  class GenericFunction {
  };

  template <typename... Ts>
  class Function : public GenericFunction {
  };

}

#endif
