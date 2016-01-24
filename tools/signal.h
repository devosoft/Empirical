//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides a mechanism to link SIGNALS to ACTIONS.
//
//  Create signals using emp::Signal templates; specify arg types sent when triggered:
//
//    emp::Signal<Genome,int> mutation_signal("mutation");
//    emp::Signal<int> update_signal("update");
//
//  Actions are just std::function or lambdas.  An action can be named with emp::NameAction:
//
//    emp::NameAction("kill_most", [pop](int ud){ if (id % 10000 == 0) pop.Kill(0.99); });
//
//  Actions can be linked to symbols by name or by object:
//
//    mutation_signal.Link( RecordMutation );        // Link signal object to a function.
//    update_signal.Link( "kill_most" );             // Link signal object to an action name.
//    emp::LinkSignal("update", OutputUpdateInfo );  // Link signal name to a function.
//    emp::LinkSignal("update", "kill_most");        // Link signal name to an action name.
//
//  When a signal is triggered, it has to be supplied with the appropriate arguments.
//
//    mutation_signal.Trigger(genome, position);
//    update_signal.Trigger(ud_count);
//
//  Technically you can trigger a signal by name, but this is slower since we have to look it up.
//
//    emp::TriggerSignal<int>("update", ud_count);
//


#ifndef EMP_SIGNAL_H
#define EMP_SIGNAL_H

#include <map>
#include <string>

#include "FunctionSet.h"

namespace emp {

  class Signal_Base {
  };

  template <typename... ARGS>
  class Signal : public Signal_Base {
  private:
    FunctionSet<void, ARGS...> actions;
  public:
    Signal() { ; }
    ~Signal() { ; }
    
    Trigger(ARGS... args) { actions.Run(args...); }

    // Add an action that takes the proper arguments.
    AddAction(const std::function<void(ARGS...)> & in_fun) { actions.Add(in_fun) }

    // Add an action that takes no arguments.
    AddAction(const std::function<void()> & in_fun) { actions.Add( [in_fun](ARGS...){in_fun();} ) }

    // @CAO... if we want chain base clasess (or do other clever meta-programming), we
    // could allow addition of actions that leave off sub-sets of actions....
  };

  class Action_Base {
  };
  
  template <typename... ARGS>
  class Action : public Action_Base {
  };
  
  // The SignalManager creates signals and handles all proper associations, but
  // is not involved once a run gets started.  As such, it does not need to be
  // overly optimized.
  
  class SignalManager {
    std::map<std::string, Signal_Base *> triggers;
    std::map<std::string, Action_Base *> actions;
  };
  
}

#endif

