//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the ActionManager class, which collects sets of Actions to be looked up
//  or manipulated later.

#ifndef EMP_CONTROL_ACTION_MANAGER
#define EMP_CONTROL_ACTION_MANAGER

#include <string>
#include <unordered_map>

#include "../tools/string_utils.h"

#include "Action.h"

namespace emp {

  class ActionManager {
  private:
    std::unordered_map<std::string, ActionBase *> action_map;
    int next_id=1;
    std::string prefix = "emp_action_";

  public:
    ActionManager() = default;
    ActionManager(ActionManager &&) = default;     // Normal juggle is okay for move constructor
    ActionManager(const ActionManager & in) : next_id(in.next_id), prefix(in.prefix) {
      // Copy all actions from input manager.
      for (const auto & x : in.action_map) {
        action_map[x.first] = x.second->Clone();
      }
    }
    ~ActionManager() { for (auto & x : action_map) delete x.second; }

    int GetNextID() const { return next_id; }
    size_t GetSize() const { return action_map.size(); }

    ActionBase & operator[](const std::string & name) {
      emp_assert(action_map.find(name) != action_map.end());
      return *(action_map[name]);
    }
    const ActionBase & operator[](const std::string & name) const {
      auto it = action_map.find(name);
      emp_assert(it != action_map.end());
      return *(it->second);
    }

    template <typename RETURN, typename... ARGS>
    auto & Add(const std::function<RETURN(ARGS...)> & in_fun, const std::string & name) {
      // Create the new action, save it, and return it.
      auto * new_action = new Action<RETURN, ARGS...>(in_fun, name);
      action_map[name] = new_action;
      return *new_action;
    }

    template <typename RETURN, typename... ARGS>
    auto & Add(const std::function<RETURN(ARGS...)> & in_fun) {
      std::string name(prefix);
      name += emp::to_string(next_id++);
      return Add(in_fun, name);
    }

    auto & Add(const ActionBase & action) {
      auto * new_action = action.Clone();
      action_map[action.GetName()] = new_action;
      return *new_action;
    }

  };

}

#endif
