//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Signal class allows functions to be bundled and triggered enmasse.

#include <map>
#include <string>

#include "Action.h"

namespace emp {

  // SignalKey tracks a specific function triggered by a signal. For now, its just an integer.
  class SignalKey {
  private:
    uint64_t id;
  public:
    SignalKey(uint64_t _id=0) : id(_id) { ; }
    SignalKey(const SignalKey &) = default;
    SignalKey & operator=(const SignalKey &) = default;
    ~SignalKey() { ; }

    bool operator==(const SignalKey& in) const { return id == in.id; }
    bool operator!=(const SignalKey& in) const { return id != in.id; }
    bool operator<(const SignalKey& in)  const { return id < in.id; }
    bool operator>(const SignalKey& in)  const { return id > in.id; }
    bool operator<=(const SignalKey& in) const { return id <= in.id; }
    bool operator>=(const SignalKey& in) const { return id >= in.id; }

    uint64_t GetID() const { return id; }
    bool IsActive() const { return id > 0; }
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
    int GetNumArgs() const { return sizeof...(ARGS); }
  };

}
