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
    std::string prefix = "emp_action_";

  public:
    ActionManager() = default;
    ActionManager(const ActionManager &) = delete; // No copy constructor; can't duplicate Action*
    ActionManager(ActionManager &&) = default;     // Normal juggle is okay for move constructor
    ~ActionManager() { for (auto & x : action_map) delete x.second; }

    int GetNextID() const { return next_id; }

    ActionBase & Get(const std::string & name) {
      emp_assert(action_map.find(name) != action_map.end());
      return *(action_map[name]);
    }

    template <typename RETURN, typename... ARGS>
    auto & Add(const std::function<RETURN(ARGS...)> & in_fun, const std::string & name) {
      // Create the new action, save it, and return it.
      auto * new_action = new Action<RETURN, ARGS...>(in_fun, name);
      action_map[name] = in_fun;
      return *new_action;
    }

    template <typename RETURN, typename... ARGS>
    auto & Add(const std::function<RETURN(ARGS...)> & in_fun) {
      std::string name(prefix);
      name += emp::to_string(next_id++);
      return Add(in_fun, name);
    }

  };

}
