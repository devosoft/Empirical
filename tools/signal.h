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
#include "FunctionSet.h"

namespace emp {

  // Pre-declare classes
  template <typename... ARGS> class Signal;
  class SignalManager;
  namespace internal {
    class Action_Base;
    class Signal_Base;
  }

  // A LinkKey is a quick way to re-find a specific link to modify or remove it.
  class LinkKey {
  private:
    int id;
  public:
    LinkKey(int _id) : id(_id) { ; }
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

    int GetID() const { return id; }
    bool IsActive() const { return id > 0; }

    // Define later...
    internal::Signal_Base * GetSignal();
    void Delete();
    void Replace(internal::Action_Base & new_action);
    template <typename... ARGS> void Replace(const std::function<void(ARGS...)> & fun);
  };

  namespace internal {
    // BASE CLASS for Signals
    class Signal_Base {
    protected:
      std::string name;                     // What is the unique name of this signal?
      std::map<LinkKey, int> link_key_map;  // Map unique link keys to link index for actions.
    public:
      Signal_Base(const std::string & n) : name(n) { ; }
      virtual ~Signal_Base() { ; }

      // Don't allow Signals to be copied.
      Signal_Base(const Signal_Base &) = delete;
      Signal_Base & operator=(const Signal_Base &) = delete;

      // NOTE: Trigger must have specialized arguments!  We require extra testing in here.
      template <typename... ARGS> void BaseTrigger(ARGS... args);

      // Add an action by name and return a unique key for the pairing.
      LinkKey AddAction(const std::string & name);

      // Actions without arguments can be associated with any signal.
      virtual LinkKey AddAction(const std::function<void()> & in_fun) = 0;

      // Add an action by Action object.
      virtual LinkKey AddAction(internal::Action_Base *) = 0;
    };

    // BASE CLASS for Actions
    class Action_Base {
    protected:
      std::string name;
    public:
      Action_Base(const std::string & n) : name(n) { ; }
      virtual ~Action_Base() { ; }

      // Don't allow Actions to be copied.
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
    std::map<int, internal::Signal_Base*> link_key_to_signal;
    int next_link_key;

    SignalManager() : next_link_key(0) { ; }
    SignalManager(const SignalManager &) = delete;
  public:
    void Register(const std::string & name, internal::Signal_Base * s) {
      emp_assert(name != "" && "Cannot register a signal without a name!");
      emp_assert(signals.find(name) == signals.end() &&
                 "Cannot register two signals by the same name.");
      signals[name] = s;
    }
    void Register(const std::string & name, internal::Action_Base * a) {
      emp_assert(name != "" && "Cannot register an action without a name!");
      emp_assert(actions.find(name) == actions.end() &&
                 "Cannot register two actions by the same name.");
      actions[name] = a;
    }

    void PrintSignalNames(int indent=0) {
      int unnamed_count = 0;
      for (const auto & s : signals) {
        if (s.first == "") {
          unnamed_count++;
          continue;
        }
        for (int i = 0; i < indent; i++) std::cout << " ";
        std::cout << s.first << std::endl;
      }
      for (int i = 0; i < indent; i++) std::cout << " ";
      std::cout << unnamed_count << " unnamed signals." << std::endl;
    }

    void PrintActionNames(int indent=0) {
      int unnamed_count = 0;
      for (const auto & s : actions) {
        if (s.first == "") {
          unnamed_count++;
          continue;
        }
        for (int i = 0; i < indent; i++) std::cout << " ";
        std::cout << s.first << std::endl;
      }
      for (int i = 0; i < indent; i++) std::cout << " ";
      std::cout << unnamed_count << " unnamed actions." << std::endl;
    }

    static SignalManager & Get() {
      static SignalManager s;
      return s;
    }

    // If a name is passed in for the signal, convert it.  Pass through anything for actions
    // and return a unique key for the pairing.
    template <typename A>
    LinkKey LinkSignal(const std::string & s_name, A && a) {
      emp_assert(signals.find(s_name) != signals.end());
      return LinkSignal(signals[s_name], std::forward<A>(a));
    }

    // If action is a name, convert it.  (signal names were handled in the previous.
    LinkKey LinkSignal(internal::Signal_Base * s, const std::string & a_name) {
      emp_assert(actions.find(a_name) != actions.end());
      return s->AddAction(actions[a_name]);
    }

    // We now know we have base classes for both signals and actions.  Convert them to
    // the derrived versions!
    LinkKey LinkSignal(internal::Signal_Base * s, internal::Action_Base * a) {
      return s->AddAction(a);
    }

    // Every link needs a unique key to be able to look it up again.
    LinkKey RegisterLink(internal::Signal_Base * s) {
      next_link_key++;
      link_key_to_signal[next_link_key] = s;
      return next_link_key;
    }

    // Provide accessors to signals and actions by name.
    internal::Signal_Base * FindSignal(const std::string & name) {
      if (signals.find(name) == signals.end()) return nullptr;
      return signals[name];
    }
    internal::Action_Base * FindAction(const std::string & name) {
      if (actions.find(name) == actions.end()) return nullptr;
      return actions[name];
    }
  };


  // Method to trigger a signal object from the BASE CLASS.
  template <typename... ARGS>
  void internal::Signal_Base::BaseTrigger(ARGS... args) {
    // Make sure this base class is really of the correct derrived time.
    emp_assert(dynamic_cast< Signal<ARGS...> * >(this));
    ((Signal<ARGS...> *) this)->Trigger(args...);
  }

  // Method to add an action by name to a Signal object
  LinkKey internal::Signal_Base::AddAction(const std::string & name)
  {
    return SignalManager::Get().LinkSignal(this, name);
  }


  template <typename... ARGS>
  class Action : public internal::Action_Base {
  public:
    std::function<void(ARGS...)> fun;

    Action(const std::function<void(ARGS...)> & f, const std::string & name="")
      : internal::Action_Base(name), fun(f) {
      if (name != "") SignalManager::Get().Register(name, this);
    }
  };

  template <typename... ARGS>
  class Signal : public internal::Signal_Base {
  private:
    FunctionSet<void, ARGS...> actions;
  public:
    Signal(const std::string & name="") : internal::Signal_Base(name) {
      if (name != "") SignalManager::Get().Register(name, this);
    }
    ~Signal() { ; }

    inline void Trigger(ARGS... args) { actions.Run(args...); }

    // Add an action that takes the proper arguments.
    LinkKey AddAction(const std::function<void(ARGS...)> & in_fun) {
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = (int) actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    // Add an action that takes no arguments.
    LinkKey AddAction(const std::function<void()> & in_fun) override {
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = (int) actions.size();
      actions.Add( [in_fun](ARGS...){in_fun();} );
      return link_id;
    }

    // Add an action object using Action_Base.
    LinkKey AddAction(internal::Action_Base * a_base) override {
      Action<ARGS...> * a = dynamic_cast< Action<ARGS...>* >(a_base);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->fun);
    }

    // Add an action object.
    LinkKey AddAction(Action<ARGS...> & a) { return AddAction(a.fun); }


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
      if (name != "") SignalManager::Get().Register(name, this);
    }
    ~Signal() { ; }

    inline void Trigger() { actions.Run(); }

    // Add an action that takes the proper arguments.
    LinkKey AddAction(const std::function<void()> & in_fun) override {
      const LinkKey link_id = SignalManager::Get().RegisterLink(this);
      link_key_map[link_id] = (int) actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    LinkKey AddAction(internal::Action_Base * a_base) override {
      Action<> * a = dynamic_cast< Action<>* >(a_base);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->fun);
    }
  };


  // Global functions that interact with the SignalManager

  template <typename S, typename A>
  LinkKey LinkSignal(S && s, A && a) {
    return SignalManager::Get().LinkSignal(std::forward<S>(s), std::forward<A>(a));
  }

  template <typename... ARGS>
  void TriggerSignal(const std::string & name, ARGS... args) {
    auto * base_signal = SignalManager::Get().FindSignal(name);
    auto * signal = dynamic_cast< Signal<ARGS...>* >(base_signal);
    signal->Trigger(args...);
  }

  void PrintSignalNames(int indent=0) { SignalManager::Get().PrintSignalNames(indent); }
  void PrintActionNames(int indent=0) { SignalManager::Get().PrintActionNames(indent); }
  void PrintSignalInfo(int indent=0) {
    if (indent) std::cout << std::string(indent, ' ');
    std::cout << "SIGNAL NAMES:" << std::endl;
    SignalManager::Get().PrintSignalNames(indent+2);
    if (indent) std::cout << std::string(indent, ' ');
    std::cout << "ACTION NAMES:" << std::endl;
    SignalManager::Get().PrintActionNames(indent+2);
  }
}

#endif
