//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The SignalControl class manages all of the signals and actions, linking them together
//  upon request (by name, base class, or derived class).

#ifndef EMP_SIGNAL_CONTROL
#define EMP_SIGNAL_CONTROL

#include <string>
#include <unordered_map>

#include "ActionManager.h"
#include "SignalManager.h"

namespace emp {

  class SignalControl {
  private:
    ActionManager action_m;
    SignalManager signal_m;
    uint32_t next_signal_id;                                 // Give each signal a unique id.
    std::unordered_map<uint32_t, SignalBase *> id_to_signal; // Map signal ID to associated pointer

  public:
    SignalControl() : next_signal_id(1) { ; }
    ~SignalControl() { ; }

    size_t GetNumActions() const { return action_m.GetSize(); }
    size_t GetNumSignals() const { return signal_m.GetSize(); }

    const ActionBase & GetAction(const std::string & name) const { return action_m[name]; }
    const SignalBase & GetSignal(const std::string & name) const { return signal_m[name]; }

    template <typename RETURN, typename... ARGS>
    auto & AddAction(const std::function<RETURN(ARGS...)> & fun, const std::string & name) {
      return action_m.Add(fun, name);
    }
    template <typename RETURN, typename... ARGS>
    auto & AddAction(const std::function<RETURN(ARGS...)> & fun) {
      return action_m.Add(fun);
    }
  };

}

#endif
