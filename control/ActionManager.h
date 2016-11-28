//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the ActionManager class, which collects sets of Actions to be looked up
//  or manipulated later.

#include <map>
#include <string>

#include "../tools/string_utils.h"

#include "Action.h"

namespace emp {

  class ActionManager {
  private:
    std::map<std::string, ActionBase *> action_map;
    int next_id=0;

  public:
    ActionManager() = default;
    ActionManager(const ActionManager &) = delete; // No copy constructor; can't duplicate Action*
    ActionManager(ActionManager &&) = default;     // Normal juggle is okay for move constructor
    ~ActionManager() { for (auto & x : action_map) delete x.second; }

    template <typename RETURN, typename... ARGS>
    auto & Insert(const std::function<RETURN(ARGS...)> & in_fun, const std::string & name) {
      // Create the new action, save it, and return it.
      auto * new_action = new Action<RETURN, ARGS...>(in_fun, name);
      action_map[name] = in_fun;
      return *new_action;
    }

    template <typename RETURN, typename... ARGS>
    auto & Insert(const std::function<RETURN(ARGS...)> & in_fun) {
      std::string name("emp_action_");
      name += emp::to_string(next_id++);
      return Insert(in_fun, name);
    }

  };

}
