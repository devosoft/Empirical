//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Signal class allows functions to be bundled and triggered enmasse.

#include <map>
#include <string>

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

    // These functions require additional classes and are defined later in this file...
    // internal::Signal_Base * GetSignal();
    // void Delete();
    // void Replace(internal::Action_Base & new_action);
    // template <typename... ARGS> void Replace(const std::function<void(ARGS...)> & fun);
  };

  class SignalBase {
  protected:
    std::string name;                      // What is the unique name of this signal?
    std::map<SignalKey, int> link_key_map; // Map unique link keys to link index for actions.
    const int num_args;                    // How many arguments does this signal provide?

    // SignalBase should only be constructable from derrived classes.
    SignalBase(const std::string & n, int a) : name(n), num_args(a) { ; }
  public:
    virtual ~SignalBase() { ; }
  };

  template <typename... ARGS>
  class Signal : public SignalBase {
  };

}
