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
//  Actions can be just std::function or lambdas, or can be built as an emp::Action object.
//
//    emp::Action([pop](int ud){ if (id%1000==0) pop.Kill(0.99); }, "kill_most");
//
//  Actions can be linked to symbols by name or by object:
//
//    mutation_signal.AddAction( RecordMutation );   // Link signal object to a function.
//    update_signal.AddAction( "kill_most" );        // Link signal object to an action name.
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

#include <functional>
#include <map>
#include <string>

#include "assert.h"
#include "FunctionSet.h"

namespace emp {

  class SignalManager;

  namespace internal {
    class Signal_Base {
    protected:
      std::string name;
    public:
      Signal_Base(const std::string & n) : name(n) { ; }
      Signal_Base(const Signal_Base &) = delete;
      Signal_Base & operator=(const Signal_Base &) = delete;
    };
    
    class Action_Base {
    protected:
      std::string name;
    public:
      Action_Base(const std::string & n) : name(n) { ; }
      Action_Base(const Action_Base &) = delete;
      Action_Base & operator=(const Action_Base &) = delete;
    };
  }
  
  // The SignalManager creates signals and handles all proper associations, but
  // is not involved once a run gets started.  As such, it does not need to be
  // overly optimized.
  
  class SignalManager {
  private:
    std::map<std::string, internal::Signal_Base *> signals;
    std::map<std::string, internal::Action_Base *> actions;

    SignalManager() = default;
    SignalManager(const SignalManager &) = delete;
  public:
    void Register(const std::string & name, internal::Signal_Base * s) {
      emp_assert(signals.find(name) == signals.end() &&
                 "Cannot register two signals by the same name.");
      signals[name] = s;
    }
    void Register(const std::string & name, internal::Action_Base * a) {
      emp_assert(actions.find(name) == actions.end() &&
                 "Cannot register two actions by the same name.");
      actions[name] = a;
    }

    static SignalManager & Get() {
      static SignalManager s;
      return s;
    }
  };
  

  template <typename... ARGS>
  class Signal : public internal::Signal_Base {
  private:
    FunctionSet<void, ARGS...> actions;
  public:
    Signal(const std::string & name="") : internal::Signal_Base(name) {
      SignalManager::Get().Register(name, this);
    }
    ~Signal() { ; }
    
    void Trigger(ARGS... args) { actions.Run(args...); }

    // Add an action that takes the proper arguments.
    void AddAction(const std::function<void(ARGS...)> & in_fun) { actions.Add(in_fun); }

    // Add an action that takes no arguments.
    void AddAction(const std::function<void()> & in_fun) {
      actions.Add( [in_fun](ARGS...){in_fun();} );
    }

    // @CAO... if we want chain base clasess (or do other clever meta-programming), we
    // could allow addition of actions that leave off sub-sets of actions....
  };

  // A specialized function for no arguments.
  template <>
  class Signal<> : public internal::Signal_Base {
  private:
    FunctionSet<void> actions;
  public:
    Signal(const std::string & name="") : internal::Signal_Base(name) {
      SignalManager::Get().Register(name, this);
    }
    ~Signal() { ; }
    
    void Trigger() { actions.Run(); }

    // Add an action that takes the proper arguments.
    void AddAction(const std::function<void()> & in_fun) { actions.Add(in_fun); }
  };

  template <typename... ARGS>
  class Action : public internal::Action_Base {
  public:
    std::function<void(ARGS...)> fun;

    Action(std::function<void(ARGS...)> & f, const std::string & name="") : fun(f) {
      if (name != "") SignalManager::Get().Register(name, this);
    }
  };
  
}

#endif

