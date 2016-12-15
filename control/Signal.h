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

    // SignalBase should only be constructable from derrived classes.
    SignalBase(const std::string & n) : name(n) { ; }
  public:
    virtual ~SignalBase() { ; }

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
  public:
    Signal(const std::string & name) : SignalBase(name) { ; }
    
    int GetNumArgs() const { return sizeof...(ARGS); }
  };

}

#endif
