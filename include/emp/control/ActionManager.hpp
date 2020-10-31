/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  Action.hpp
 *  @brief ActionManager collects sets of Actions to be looked up or manipulated later.
 *  @note Status: Beta
 */

#ifndef EMP_CONTROL_ACTION_MANAGER
#define EMP_CONTROL_ACTION_MANAGER

#include <string>
#include <unordered_map>

#include "../tools/string_utils.hpp"
#include "Action.hpp"

namespace emp {

  class ActionManager {
  private:
    std::unordered_map<std::string, ActionBase *> action_map;  ///< A set of all actions handled by manager.
    int next_id=1;                                             ///< Unique ID for next new function.
    std::string prefix = "emp_action_";                        ///< Prefix for function names to keep unique.

  public:
    ActionManager() : action_map() { ; }
    ActionManager(ActionManager &&) = default;     // Normal juggle is okay for move constructor
    ActionManager(const ActionManager & in) : action_map(), next_id(in.next_id), prefix(in.prefix) {
      // Copy all actions from input manager.
      for (const auto & x : in.action_map) {
        action_map[x.first] = x.second->Clone();
      }
    }
    ~ActionManager() { for (auto & x : action_map) delete x.second; }

    /// Get the ID to be used for the next new function.
    int GetNextID() const { return next_id; }

    /// How many actions are in this manager?
    size_t GetSize() const { return action_map.size(); }

    /// Look up an action with the specified name
    ActionBase & operator[](const std::string & name) {
      emp_assert(action_map.find(name) != action_map.end());
      return *(action_map[name]);
    }

    /// Look up an action with the specified name (const version)
    const ActionBase & operator[](const std::string & name) const {
      auto it = action_map.find(name);
      emp_assert(it != action_map.end());
      return *(it->second);
    }

    /// Add a functon to this manager with a pre-specified name.
    template <typename RETURN, typename... ARGS>
    auto & Add(const std::function<RETURN(ARGS...)> & in_fun, const std::string & name) {
      // Create the new action, save it, and return it.
      auto * new_action = new Action<RETURN(ARGS...)>(in_fun, name);
      action_map[name] = new_action;
      return *new_action;
    }

    /// Add a function to this manager with an auto-generated name.
    template <typename RETURN, typename... ARGS>
    auto & Add(const std::function<RETURN(ARGS...)> & in_fun) {
      std::string name(prefix);
      name += emp::to_string(next_id++);
      return Add(in_fun, name);
    }

    /// Add an action to this manager.
    auto & Add(const ActionBase & action) {
      auto * new_action = action.Clone();
      action_map[action.GetName()] = new_action;
      return *new_action;
    }

    /// Print out the name of all actions maintained by this manager.
    void PrintNames(std::ostream & os=std::cout) {
      os << action_map.size() << " actions found:\n";
      for (auto & x : action_map) os << "  " << x.first << std::endl;
    }
  };

}

#endif
