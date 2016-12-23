//  This file is part of Empirical, https://github.com/devosoft/Empirical
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
//    emp::Action<int>([pop](int ud){ if (id%1000==0) pop.Kill(0.99); }, "kill_most");
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
#include "../debug/debug.h"
#include "FunctionSet.h"
#include "string_utils.h"
#include "../meta/TypePack.h"

namespace emp {

  // Pre-declare classes
  template <typename... ARGS> class Signal;
  class SignalManager;
  namespace internal {
    class Action_Base;
    class Signal_Base;
  }

  // A LinkKey is a quick way to keep track of a specific link to modify or remove it.
  // For now, it just maps to an integer value.
  class LinkKey {
  private:
    uint64_t id;
  public:
    LinkKey(uint64_t _id) : id(_id) { ; }
    LinkKey(const LinkKey &) = default;
    LinkKey & operator=(const LinkKey &) = default;
    LinkKey() = delete;
    ~LinkKey() { ; }

    bool operator==(const LinkKey& in) const { return id == in.id; }
    bool operator!=(const LinkKey& in) const { return id != in.id; }
    bool operator<(const LinkKey& in)  const { return id < in.id; }
    bool operator>(const LinkKey& in)  const { return id > in.id; }
    bool operator<=(const LinkKey& in) const { return id <= in.id; }
    bool operator>=(const LinkKey& in) const { return id >= in.id; }

    uint64_t GetID() const { return id; }
    bool IsActive() const { return id > 0; }

    // These functions require additional classes and are defined later in this file...
    internal::Signal_Base * GetSignal();
    void Delete();
    void Replace(internal::Action_Base & new_action);
    template <typename... ARGS> void Replace(const std::function<void(ARGS...)> & fun);
  };

  // Hidden implementation details are in an internal namespace.
  namespace internal {
    // BASE CLASS for all Signals
    class Signal_Base {
    protected:
      std::string name;                        // What is the unique name of this signal?
      std::map<LinkKey, size_t> link_key_map;  // Map unique link keys to link index for actions.
      const size_t num_args;                   // How many arguments does this signal provide?

      // Signal_Base should only be constructable from derrived classes.
      Signal_Base(const std::string & n, size_t a) : name(n), num_args(a) {
        Depricated("signal.h", "Instead use reimplementation in control/ folder.");
      }
    public:
      virtual ~Signal_Base() { ; }

      // Don't allow Signals to be copied or moved.
      Signal_Base(const Signal_Base &) = delete;
      Signal_Base(Signal_Base &&) = delete;
      Signal_Base & operator=(const Signal_Base &) = delete;
      Signal_Base & operator=(Signal_Base &&) = delete;

      const std::string & GetName() const { return name; }
      size_t GetNumArgs() const { return num_args; }

      // NOTE: If a Trigger is called on a base class, convert the signal assuming that the args
      // map to the correct types (defined below with a dynamic cast to ensure correctness)
      template <typename... ARGS> void BaseTrigger(ARGS... args);

      // Add a previously-defined action by name and return a unique key for the pairing.
      LinkKey AddAction(const std::string & name);

      // Actions without arguments or a return type can be associated with any signal.
      virtual LinkKey AddAction(const std::function<void()> & in_fun) = 0;

      // Add an action using an Action object.
      virtual LinkKey AddAction(internal::Action_Base &) = 0;
    };

    // BASE CLASS for all Actions
    class Action_Base {
    protected:
      std::string name;          // Unique name for this action.
      const size_t num_params;   // How many paramaters does this action have?

      Action_Base(const std::string & n, size_t p) : name(n), num_params(p) { ; }
    public:
      virtual ~Action_Base() { ; }

      // Don't allow Actions to be copied.
      Action_Base(const Action_Base &) = delete;
      Action_Base(Action_Base &&) = delete;
      Action_Base & operator=(const Action_Base &) = delete;
      Action_Base & operator=(Action_Base &&) = delete;
    };
   }

  // The SignalManager creates signals and handles all proper associations, but does nothing
  // once signals and actions are attached.  As such, it does not need to be overly optimized.

  class SignalManager {
  private:
    std::map<std::string, internal::Signal_Base *> signals;       // Map names to signals
    std::map<std::string, internal::Action_Base *> actions;       // Map names to actions
    std::map<size_t, internal::Signal_Base*> link_key_to_signal;  // Map keys to signals

    size_t next_link_key;     // Each signal-action link should have a unique key id
    size_t next_name_id;      // Used in order to build unique signal names

    SignalManager() : next_link_key(0), next_name_id(0) { ; }
    SignalManager(const SignalManager &) = delete;
    SignalManager(SignalManager &&) = delete;
  public:
    void Register(const std::string & name, internal::Signal_Base * s) {
      emp_assert(name != "" && "Cannot register a signal without a name!");
      emp_assert(signals.find(name) == signals.end() &&
                 "Cannot register two signals by the same name.", name);
      signals[name] = s;
    }
    void Register(const std::string & name, internal::Action_Base * a) {
      emp_assert(name != "" && "Cannot register an action without a name!");
      emp_assert(actions.find(name) == actions.end() &&
                 "Cannot register two actions by the same name.", name);
      actions[name] = a;
    }

    inline void PrintSignalNames(size_t indent=0) {
      for (const auto & s : signals) {
        for (size_t i = 0; i < indent; i++) std::cout << " ";
        std::cout << s.first << std::endl;
      }
    }

    inline void PrintActionNames(size_t indent=0) {
      for (const auto & s : actions) {
        for (size_t i = 0; i < indent; i++) std::cout << " ";
        std::cout << s.first << std::endl;
      }
    }


    // Generate a unique signal name to prevent duplicates.
    inline std::string GenerateSignalName(const std::string & prefix) {
      return emp::to_string(prefix, '.', next_name_id++);
    }


    // We want a singleton for the SignalManager, so the following static function provides it.
    static SignalManager & Get() {
      static SignalManager s;
      return s;
    }


    // If a name is passed in for the signal, convert it to a Signal_Base pointer.
    // Pass through anything for actions and return a unique key for the pairing.
    template <typename A>
    LinkKey LinkSignal(const std::string & s_name, A && a) {
      emp_assert(signals.find(s_name) != signals.end());
      return LinkSignal(signals[s_name], std::forward<A>(a));  // Lookup signal name and forward.
    }

    // If a name is passed in for the action, convert it to an Action_Base pointer.
    // (signal names were handled in the previous overloaded version of this function)
    LinkKey LinkSignal(internal::Signal_Base * s, const std::string & a_name) {
      emp_assert(actions.find(a_name) != actions.end());
      return s->AddAction(*(actions[a_name]));  // Lookup action name and add it to the signal.
    }

    // We now know we have base classes for both signals and actions.  Convert them to
    // the derrived versions!
    LinkKey LinkSignal(internal::Signal_Base * s, internal::Action_Base * a) {
      return s->AddAction(*a);
    }

    // One final pair of cases: if we were provided with a function object; pass it through!
    // (Defined near bottom)
    template <typename... ARGS>
    LinkKey LinkSignal(internal::Signal_Base * s, const std::function<void(ARGS...)> & fun);

    // Add an action that takes no arguments.
    LinkKey LinkSignal(internal::Signal_Base * s, const std::function<void()> & in_fun) {
      return s->AddAction(in_fun);
    }

    // Helper function to generate a unique key to be able to look up a link again.
    LinkKey RegisterLink(internal::Signal_Base * s) {
      next_link_key++;
      link_key_to_signal[next_link_key] = s;
      return next_link_key;
    }

    // Signals and actions should be searchable by name.
    internal::Signal_Base * FindSignal(const std::string & name) {
      if (signals.find(name) == signals.end()) return nullptr;
      return signals[name];
    }
    internal::Action_Base * FindAction(const std::string & name) {
      if (actions.find(name) == actions.end()) return nullptr;
      return actions[name];
    }
  };


  // Definition for method to trigger a signal object from the BASE CLASS, declared above.
  template <typename... ARGS>

  inline void internal::Signal_Base::BaseTrigger(ARGS... args) {
    // Make sure this base class is really of the correct derrived type (but do so in an
    // assert since triggers may be called frequently and should be fast!)
    emp_assert(dynamic_cast< Signal<ARGS...> * >(this));
    ((Signal<ARGS...> *) this)->Trigger(args...);
  }

  // Method to add an action by name to a Signal object
  inline LinkKey internal::Signal_Base::AddAction(const std::string & name)
  {
    return SignalManager::Get().LinkSignal(this, name);
  }

  // Derived Action classes add a specific function call associated with the action.
  template <typename... ARGS>
  class Action : public internal::Action_Base {
  public:
    std::function<void(ARGS...)> fun;

    Action(const std::function<void(ARGS...)> & f, const std::string & name="")
      : internal::Action_Base(name,sizeof...(ARGS)), fun(f) {
      if (name != "") SignalManager::Get().Register(name, this);
    }
  };


  // Derived signals add a set of functions associated with each action.
  template <typename... ARGS>
  class Signal : public internal::Signal_Base {
  private:
    FunctionSet<void, ARGS...> actions;
    static constexpr size_t arg_count = sizeof...(ARGS);
  public:
    Signal(const std::string & name="") : internal::Signal_Base(name,sizeof...(ARGS)) {
      if (name != "") SignalManager::Get().Register(name, this);
    }
    ~Signal() { ; }

    size_t GetNumActions() const { return actions.GetSize(); }

    inline void Trigger(ARGS... args) { actions.Run(args...); }

    // Add an action that takes the proper arguments.
    inline LinkKey AddAction(const std::function<void(ARGS...)> & in_fun) {
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    // Add an action that takes too few arguments... but provide specific padding info.
    template <typename... FUN_ARGS, typename... EXTRA_ARGS>
    LinkKey AddAction(const std::function<void(FUN_ARGS...)> & in_fun,
                      TypePack<EXTRA_ARGS...> extra)
    {
      // If we made it here, we know the extra arguments that we need to throw away when
      // calling this function.  Call it correctly.
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = actions.size();
      std::function<void(ARGS...)> full_fun =
        [in_fun](FUN_ARGS... args, EXTRA_ARGS...){ in_fun(std::forward<FUN_ARGS>(args)...); };
      actions.Add(full_fun);
      return link_id;
    }

    // Add an action that takes too few arguments... we need to figure out how to pad it out.
    template <typename... FUN_ARGS>
    LinkKey AddAction(const std::function<void(FUN_ARGS...)> & in_fun) {
      // Trim off the args we have... any extras should be accepted, but ignored.
      using extra_type = typename TypePack<ARGS...>::template popN<sizeof...(FUN_ARGS)>;
      return AddAction(in_fun, extra_type());
    }


    // Add an action that takes no arguments.
    inline LinkKey AddAction(const std::function<void()> & in_fun) override {
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = actions.size();
      actions.Add( [in_fun](ARGS...){ in_fun(); } );
      return link_id;
    }

    // Add an action from an Action object using Action_Base.
    inline LinkKey AddAction(internal::Action_Base & a_base) override {
      Action<ARGS...> * a = dynamic_cast< Action<ARGS...>* >(&a_base);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->fun);
    }

    // Add an action object directly.
    inline LinkKey AddAction(Action<ARGS...> & a) { return AddAction(a.fun); }
  };

  // A specialized version of Signal for functions with no arguments.
  template <>
  class Signal<> : public internal::Signal_Base {
  private:
    FunctionSet<void> actions;
  public:
    Signal(const std::string & name="") : internal::Signal_Base(name,0) {
      if (name != "") SignalManager::Get().Register(name, this);
    }
    ~Signal() { ; }

    inline size_t GetNumActions() const { return actions.GetSize(); }

    inline void Trigger() { actions.Run(); }

    // Add an action that takes the proper arguments.
    inline LinkKey AddAction(const std::function<void()> & in_fun) override {
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    inline LinkKey AddAction(internal::Action_Base & a_base) override {
      Action<> * a = dynamic_cast< Action<>* >(&a_base);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->fun);
    }
  };

  // Definition of link signal (declared above) that requires Action template to be defined.
  template <typename... ARGS>
  LinkKey SignalManager::LinkSignal(internal::Signal_Base * s,
                                    const std::function<void(ARGS...)> & fun) {
    emp::Action<ARGS...> action(fun);
    return s->AddAction(action);
  }


  // -- Global functions that interact with the SignalManager --

  template <typename S, typename A>
  LinkKey LinkSignal(S && s, A && a) {
    return SignalManager::Get().LinkSignal(std::forward<S>(s), std::forward<A>(a));
  }

  template <typename... ARGS>
  void TriggerSignal(const std::string & name, ARGS... args) {
    auto * base_signal = SignalManager::Get().FindSignal(name);
    auto * signal = dynamic_cast< Signal<ARGS...>* >(base_signal);
    emp_assert( signal != nullptr && "invalid signal conversion!" );
    signal->Trigger(args...);
  }

  inline void PrintSignalNames(size_t indent=0) { SignalManager::Get().PrintSignalNames(indent); }
  inline void PrintActionNames(size_t indent=0) { SignalManager::Get().PrintActionNames(indent); }
  inline void PrintSignalInfo(size_t indent=0) {
    if (indent) std::cout << std::string(indent, ' ');
    std::cout << "SIGNAL NAMES:" << std::endl;
    SignalManager::Get().PrintSignalNames(indent+2);
    if (indent) std::cout << std::string(indent, ' ');
    std::cout << "ACTION NAMES:" << std::endl;
    SignalManager::Get().PrintActionNames(indent+2);
  }

  inline std::string GenerateSignalName(const std::string & prefix) {
    return SignalManager::Get().GenerateSignalName(prefix);
  }
}

#endif
