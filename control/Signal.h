//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Signal class allows functions to be bundled and triggered enmasse.

#ifndef EMP_CONTROL_SIGNAL
#define EMP_CONTROL_SIGNAL

#include <map>
#include <string>

#include "../tools/FunctionSet.h"

#include "Action.h"

namespace emp {

  // SignalKey tracks a specific function triggered by a signal. For now, its just an integer.
  class SignalKey {
  private:
    uint32_t signal_id;
    uint32_t key_id;

    int Compare(const SignalKey& in) const {
      if (signal_id < in.signal_id) return -1;
      if (signal_id > in.signal_id) return 1;
      if (key_id < in.key_id) return -1;
      if (key_id > in.key_id) return 1;
      return 0;
    }
  public:
    SignalKey(uint32_t _kid=0, uint32_t _sid=0) : signal_id(_sid), key_id(_kid) { ; }
    SignalKey(const SignalKey &) = default;
    SignalKey & operator=(const SignalKey &) = default;
    ~SignalKey() { ; }

    bool operator==(const SignalKey& in) const { return Compare(in) == 0; }
    bool operator!=(const SignalKey& in) const { return Compare(in) != 0; }
    bool operator<(const SignalKey& in)  const { return Compare(in) < 0; }
    bool operator>(const SignalKey& in)  const { return Compare(in) > 0; }
    bool operator<=(const SignalKey& in) const { return Compare(in) <= 0; }
    bool operator>=(const SignalKey& in) const { return Compare(in) >= 0; }

    uint64_t GetID() const { return key_id; }
    uint64_t GetSignalID() const { return signal_id; }
    bool IsActive() const { return key_id > 0; }
  };

  class SignalBase {
  protected:
    std::string name;                      // What is the unique name of this signal?
    std::map<SignalKey, int> link_key_map; // Map unique link keys to link index for actions.

    // Helper Functions
    // @CAO FIX!!!
    SignalKey NextSignalKey() { return SignalKey(0,0); }

    // SignalBase should only be constructable from derrived classes.
    SignalBase(const std::string & n) : name(n) { ; }
  public:
    virtual ~SignalBase() { ; }
    virtual SignalBase * Clone() const = 0;

    const std::string & GetName() const { return name; }
    virtual int GetNumArgs() const = 0;

    // NOTE: If a Trigger is called on a base class, convert the signal assuming that the args
    // map to the correct types (defined below with a dynamic cast to ensure correctness)
    template <typename... ARGS>
    void BaseTrigger(ARGS... args);

    // Actions without arguments or a return type can be associated with any signal.
    template <typename... ARGS>
    SignalKey AddAction(const std::function<void(ARGS...)> & in_fun);

    // Add an action using an Action object.
    virtual SignalKey AddAction(ActionBase &) = 0;
  };

  template <typename... ARGS>
  class Signal : public SignalBase {
  protected:
    using this_t = Signal<ARGS...>;
    FunctionSet<void, ARGS...> actions;
  public:
    Signal(const std::string & name) : SignalBase(name) { ; }
    virtual this_t * Clone() const {
      this_t * new_copy = new this_t(name);
      // @CAO: Make sure to copy over actions into new copy.
      return new_copy;
    }

    int GetNumArgs() const { return sizeof...(ARGS); }

    void Trigger(ARGS... args) { actions.Run(args...); }

    // Add an action that takes the proper arguments.
    SignalKey AddAction(const std::function<void(ARGS...)> & in_fun) {
      const SignalKey link_id = NextSignalKey();
      link_key_map[link_id] = actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    SignalKey AddAction(ActionBase & in_action) {
      Action<ARGS...> * a = dynamic_cast< Action<ARGS...>* >(&in_action);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->GetFun());
    }
  };

  template<typename... ARGS>
  inline void SignalBase::BaseTrigger(ARGS... args) {
    // Make sure this base class is really of the correct derrived type (but do so in an
    // assert since triggers may be called frequently and should be fast!)
    emp_assert(dynamic_cast< Signal<ARGS...> * >(this));
    ((Signal<ARGS...> *) this)->Trigger(args...);
  }

}

#endif
