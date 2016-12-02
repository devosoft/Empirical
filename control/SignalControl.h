//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The SignalControl class manages all of the signals and actions, linking them together
//  upon request (by name, base class, or derived class).

#include <string>
#include <unordered_map>

#include "ActionManager.h"
#include "Signal.h"

namespace emp {

  class SignalControl {
  private:
    ActionManager action_m;
    std::unordered_map<std::string, SignalBase *> signals;   // Map names to signals

  public:
  };

}
