//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the ActionManager class, which collects sets of Actions to be looked up
//  or manipulated later.

#include <map>
#include <string>

#include "Action.h"

namespace emp {

  class ActionManager {
  private:
    std::map<std::string, ActionBase *> action_map;

  public:
    ActionManager() = default;
    ActionManager(const ActionManager &) = delete; // No copy constructor; can't duplicate Action*
    ActionManager(ActionManager &&) = default;     // Normal juggle is okay for move constructor
    ~ActionManager() { for (auto & x : action_map) delete x.second; }
  };

}
