/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file SignalControl.hpp
 *  @brief The SignalControl class manages all of the signals and actions, linking them together
 *  upon request (by name, base class, or derived class).
 *
 *  Developer notes:
 *  * Right now SignalControl can only link to signals that it creates.  Ideally, external signals
 *    should also be able to be added to it.  The question is, how should this be done?
 *    - They could be copied.  But should we copy existing actions?  How do we provide keys?
 *    - They could use a smart pointer style, so only deleted with final version (slow down?)
 */

#ifndef EMP_CONTROL_SIGNALCONTROL_HPP_INCLUDE
#define EMP_CONTROL_SIGNALCONTROL_HPP_INCLUDE

#include <string>
#include <unordered_map>

#include "ActionManager.hpp"
#include "SignalManager.hpp"

namespace emp {

  class SignalControl : public internal::SignalControl_Base {
  private:
    ActionManager action_m;
    SignalManager signal_m;
    // uint32_t next_signal_id;                                  // Give each signal a unique id.
    std::unordered_map<uint32_t, SignalBase *> id_to_signal;     // Map signal ID to associated pointer

  public:
    SignalControl() : action_m(), signal_m(), id_to_signal() {;} // : next_signal_id(1) { ; }
    ~SignalControl() { ; }

    size_t GetNumActions() const { return action_m.GetSize(); }
    size_t GetNumSignals() const { return signal_m.GetSize(); }

    const ActionBase & GetAction(const std::string & name) const { return action_m[name]; }
    const SignalBase & GetSignal(const std::string & name) const { return signal_m[name]; }

    ActionManager & GetActionManager() { return action_m; }
    SignalManager & GetSignalManager() override { return signal_m; }

    template <typename... Ts>
    auto & AddAction(Ts &&... act) { return action_m.Add( std::forward<Ts>(act)... ); }

    template <typename... ARGS>
    auto & AddSignal(const std::string & name="") {
      return signal_m.Add<ARGS...>(name);
    }

    template <typename... ARGS>
    auto & AddSignal(Signal<void(ARGS...)> & signal) {
      return signal_m.Add<ARGS...>(signal);
    }

    // Link a specified signal to a specified function.
    template <typename... ARGS>
    auto Link(SignalBase & s, const std::function<void(ARGS...)> & f) { return s.AddAction(f); }

    // If a name is passed in for the signal, convert it to a SignalBase.
    // Pass through anything for actions and return a unique key for the pairing.
    template <typename A>
    auto Link(const std::string & s, A && a) { return Link(signal_m[s], std::forward<A>(a)); }

    // If a name is passed in for the action, convert it to an ActionBase.
    // (signal names were handled in the previous overload of this function)
    auto Link(SignalBase & s, const std::string & a) { return s.AddAction(action_m[a]); }

    // If we have base classes for both signals and actions.  Convert to derrived versions!
    auto Link(SignalBase & s, ActionBase & a) { return s.AddAction(a); }


    template <typename... ARGS>
    void Trigger(const std::string & name, ARGS &&... args) {
      auto & base_signal = signal_m[name];
      auto * signal = dynamic_cast< Signal<void(ARGS...)>* >(&base_signal);
      emp_assert( signal != nullptr && "invalid signal conversion!" );
      signal->Trigger(std::forward<ARGS>(args)...);
    }


    void NotifyConstruct(SignalBase * sig_ptr) override { signal_m.NotifyConstruct(sig_ptr); }


    void PrintNames(std::ostream & os=std::cout) {
      action_m.PrintNames(os);
      signal_m.PrintNames(os);
    }
  };

}

#endif // #ifndef EMP_CONTROL_SIGNALCONTROL_HPP_INCLUDE
