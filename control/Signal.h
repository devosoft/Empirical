//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Signal class allows functions to be bundled and triggered enmasse.

#include <string>

namespace emp {

  class SignalBase {
  private:
    std::string name;
  public:
  };

  template <typename... ARGS>
  class Signal : public SignalBase {
  };

}
