/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  Action.hpp
 *  @brief Allow functions to be bundled (as Actions) and triggered enmasse.
 *  @note Status: Beta
 *
 *  @todo Setup easier mechanism to control the order in which actions are triggered.
 *  @todo Signals should have default parameters so not all need be supplied when triggered.
 */


#ifndef EMP_CONTROL_SIGNAL
#define EMP_CONTROL_SIGNAL

#include <map>
#include <string>

#include "../meta/TypePack.hpp"
#include "../functional/FunctionSet.hpp"
#include "../datastructs/map_utils.hpp"

#include "Action.hpp"

namespace emp {

  /// SignalKey tracks a specific function triggered by a signal. For now, its just a value pair.
  class SignalKey {
  private:
    uint32_t signal_id;   ///< Which signal is this key associated with?
    uint32_t key_id;      ///< Which key id is this.

    // Internal function to compare two signal kays.
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

    /// Are two signal keys identical?
    bool operator==(const SignalKey& in) const { return Compare(in) == 0; }

    /// Are two signal keys different?
    bool operator!=(const SignalKey& in) const { return Compare(in) != 0; }

    bool operator<(const SignalKey& in)  const { return Compare(in) < 0; }
    bool operator>(const SignalKey& in)  const { return Compare(in) > 0; }
    bool operator<=(const SignalKey& in) const { return Compare(in) <= 0; }
    bool operator>=(const SignalKey& in) const { return Compare(in) >= 0; }

    /// What is the KeyID associated with this signal key.
    uint32_t GetID() const { return key_id; }

    /// What is the ID of the signal that this key is associated with.
    uint32_t GetSignalID() const { return signal_id; }

    /// Is this key currently pointing to a signal action?
    bool IsActive() const { return key_id > 0; }

    /// Set this key to specified values.
    void Set(uint32_t _kid=0, uint32_t _sid=0) { signal_id = _sid; key_id = _kid; }

    /// Clear this key.
    void Clear() { signal_id = 0; key_id = 0; }

    operator bool() { return key_id > 0; }
  };

  // Forward declarations.
  class SignalBase;     // ...for pointers to signals.
  class SignalManager;  // ...for setting up as friend.

  // Mechanisms for Signals to report to a manager.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    struct SignalManager_Base {
      virtual void NotifyConstruct(SignalBase * sig_ptr) = 0;
      virtual void NotifyDestruct(SignalBase * sig_ptr) = 0;
      virtual ~SignalManager_Base() { ; }
    };
    struct SignalControl_Base {
      virtual SignalManager_Base & GetSignalManager() = 0;
      virtual void NotifyConstruct(SignalBase * sig_ptr) = 0;
      virtual ~SignalControl_Base() { ; }
    };
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Base class for all signals.
  class SignalBase {
    friend class SignalManager;  // Allow SignalManager to alter internals of a signal.
  protected:
    using man_t = internal::SignalManager_Base;

    std::string name;                          ///< What is the unique name of this signal?
    uint32_t signal_id;                        ///< What is the unique ID of this signal?
    uint32_t next_link_id;                     ///< What ID shouild the next link have?
    std::map<SignalKey, size_t> link_key_map;  ///< Map unique link keys to link index for actions.
    emp::vector<man_t *> managers;             ///< What manager is handling this signal?
    man_t * prime_manager;                     ///< Which manager leads deletion? (nullptr for self)

    // Helper Functions
    SignalKey NextSignalKey() { return SignalKey(signal_id,++next_link_id); }

    // SignalBase should only be constructable from derrived classes.
    SignalBase(const std::string & n, internal::SignalManager_Base * manager=nullptr)
    : name(n), signal_id(0), next_link_id(0), link_key_map(), managers(), prime_manager(nullptr)
    {
      if (manager) manager->NotifyConstruct(this);
    }
  public:
    SignalBase() = delete;
    SignalBase(const SignalBase &) = delete;
    SignalBase(SignalBase &&) = delete;
    SignalBase & operator=(const SignalBase &) = delete;
    SignalBase & operator=(SignalBase &&) = delete;
    virtual ~SignalBase() {
      // Let all managers other than prime know about destruction (prime must have triggered it.)
      for (auto * m : managers) if (m != prime_manager) m->NotifyDestruct(this);
    }
    virtual SignalBase * Clone() const = 0;

    const std::string & GetName() const { return name; }
    virtual size_t GetNumArgs() const = 0;
    virtual size_t GetNumActions() const = 0;

    // NOTE: If a Trigger is called on a base class, convert the signal assuming that the args
    // map to the correct types (defined below with a dynamic cast to ensure correctness)
    template <typename... ARGS>
    void BaseTrigger(ARGS... args);

    /// Actions without arguments or a return type can be associated with any signal.
    template <typename... ARGS>
    SignalKey AddAction(const std::function<void(ARGS...)> & in_fun);

    /// Add an action using an Action object.
    virtual SignalKey AddAction(ActionBase &) = 0;

    /// Test if an action is compatible with a signal.
    virtual bool TestMatch(ActionBase &) = 0;

    /// Remove an action specified by its key.
    virtual void Remove(SignalKey key) = 0;

    /// Remove all actions from this signal.
    void Clear() {
      // While we still have keys, remove them!
      while (link_key_map.size()) Remove(link_key_map.begin()->first);
    }

    bool Has(SignalKey key) const { return emp::Has(link_key_map, key); }
  };

  /// Generic version of Signals; needs specialization to a function type..
  template <typename... ARGS> class Signal;

  /// Signals with void return.
  template <typename... ARGS>
  class Signal<void(ARGS...)> : public SignalBase {
  protected:
    FunctionSet<void(ARGS...)> actions;  ///< Set of functions (actions) to be triggered with this signal.
  public:
    using fun_t = void(ARGS...);
    using this_t = Signal<fun_t>;

    Signal(const std::string & name="", internal::SignalManager_Base * manager=nullptr)
      : SignalBase(name, manager), actions() { ; }
    Signal(const std::string & name, internal::SignalControl_Base & control)
      : this_t(name, &(control.GetSignalManager())) { ; }
    virtual this_t * Clone() const {
      this_t * new_copy = new this_t(name);
      // @CAO: Make sure to copy over actions into new copy.
      return new_copy;
    }

    size_t GetNumArgs() const { return sizeof...(ARGS); }
    size_t GetNumActions() const { return actions.GetSize(); }

    /// Trigger this signal, providing all needed arguments.
    void Trigger(ARGS... args) { actions.Run(args...); }

    /// Add an action that takes the proper arguments.
    SignalKey AddAction(const std::function<void(ARGS...)> & in_fun) {
      const SignalKey link_id = NextSignalKey();
      link_key_map[link_id] = actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    /// Add a specified action to this signal.
    SignalKey AddAction(ActionBase & in_action) {
      Action<fun_t> * a = dynamic_cast< Action<fun_t>* >(&in_action);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->GetFun());
    }

    bool TestMatch(ActionBase & in_action) {
      return dynamic_cast< Action<fun_t>* >(&in_action);
    }

    /// Add an action that takes too few arguments... but provide specific padding info.
    template <typename... FUN_ARGS, typename... EXTRA_ARGS>
    SignalKey AddAction(const std::function<void(FUN_ARGS...)> & in_fun, TypePack<EXTRA_ARGS...>)
    {
      // If we made it here, we have isolated the extra arguments that we need to throw away to
      // call this function correctly.
      const SignalKey link_id = NextSignalKey();
      link_key_map[link_id] = actions.size();
      std::function<void(ARGS...)> expand_fun =
        [in_fun](FUN_ARGS &&... args, EXTRA_ARGS...){ in_fun(std::forward<FUN_ARGS>(args)...); };
      actions.Add(expand_fun);
      return link_id;
    }

    /// Add an std::function that takes the wrong number of arguments.  For now, we will assume
    /// that there are too few and we need to figure out how to pad it out.
    template <typename... FUN_ARGS>
    SignalKey AddAction(const std::function<void(FUN_ARGS...)> & in_fun) {
      // Identify the extra arguments by removing the ones that we know about.
      using extra_type = typename TypePack<ARGS...>::template popN<sizeof...(FUN_ARGS)>;
      return AddAction(in_fun, extra_type());
    }

    /// Add a regular function that takes the wrong number of arguments.  For now, we will assume
    /// that there are too few and we need to figure out how to pad it out.
    template <typename... FUN_ARGS>
    SignalKey AddAction(void in_fun(FUN_ARGS...)) {
      // Identify the extra arguments by removing the ones that we know about.
      using extra_type = typename TypePack<ARGS...>::template popN<sizeof...(FUN_ARGS)>;
      return AddAction(std::function<void(FUN_ARGS...)>(in_fun), extra_type());
    }

    /// Remove an action from this signal by providing its key.
    void Remove(SignalKey key) {
      // Find the action associate with this key.
      emp_assert(emp::Has(link_key_map, key));
      size_t pos = link_key_map[key];

      // Remove the action
      actions.Remove(pos);
      link_key_map.erase(key);

      // Adjust all of the positions of the actions that came after this one.
      for (auto & x : link_key_map) {
        if (x.second > pos) x.second = x.second - 1;
      }
    }

    /// Retrieve the relative priority associated with a specific
    size_t GetPriority(SignalKey key) {
      emp_assert(emp::Has(link_key_map, key));
      return link_key_map[key];
    }

  };

  // Signals with NON-void return.
  template <typename RETURN, typename... ARGS>
  class Signal<RETURN(ARGS...)> : public SignalBase {
  protected:
    FunctionSet<RETURN(ARGS...)> actions;
  public:
    using fun_t = RETURN(ARGS...);
    using this_t = Signal<fun_t>;

    Signal(const std::string & name="", internal::SignalManager_Base * manager=nullptr)
      : SignalBase(name, manager) { ; }
    Signal(const std::string & name, internal::SignalControl_Base & control)
      : this_t(name, &(control.GetSignalManager())) { ; }
    virtual this_t * Clone() const {
      this_t * new_copy = new this_t(name);
      // @CAO: Make sure to copy over actions into new copy.
      return new_copy;
    }

    size_t GetNumArgs() const { return sizeof...(ARGS); }
    size_t GetNumActions() const { return actions.GetSize(); }

    const emp::vector<RETURN> & Trigger(ARGS... args) { return actions.Run(args...); }

    // Add an action that takes the proper arguments.
    SignalKey AddAction(const std::function<fun_t> & in_fun) {
      const SignalKey link_id = NextSignalKey();
      link_key_map[link_id] = actions.size();
      actions.Add(in_fun);
      return link_id;
    }

    SignalKey AddAction(ActionBase & in_action) {
      Action<fun_t> * a = dynamic_cast< Action<fun_t>* >(&in_action);
      emp_assert( a != nullptr && "action type must match signal type." );
      return AddAction(a->GetFun());
    }

    bool TestMatch(ActionBase & in_action) {
      return dynamic_cast< Action<fun_t>* >(&in_action);
    }

    // Add an action that takes too few arguments... but provide specific padding info.
    template <typename... FUN_ARGS, typename... EXTRA_ARGS>
    SignalKey AddAction(const std::function<RETURN(FUN_ARGS...)> & in_fun, TypePack<EXTRA_ARGS...>)
    {
      // If we made it here, we have isolated the extra arguments that we need to throw away to
      // call this function correctly.
      const SignalKey link_id = NextSignalKey();
      link_key_map[link_id] = actions.size();
      std::function<fun_t> expand_fun =
        [in_fun](FUN_ARGS &&... args, EXTRA_ARGS...){ in_fun(std::forward<FUN_ARGS>(args)...); };
      actions.Add(expand_fun);
      return link_id;
    }

    // Add an std::function that takes the wrong number of arguments.  For now, we will assume
    // that there are too few and we need to figure out how to pad it out.
    template <typename... FUN_ARGS>
    SignalKey AddAction(const std::function<RETURN(FUN_ARGS...)> & in_fun) {
      // Identify the extra arguments by removing the ones that we know about.
      using extra_type = typename TypePack<ARGS...>::template popN<sizeof...(FUN_ARGS)>;
      return AddAction(in_fun, extra_type());
    }

    // Add a regular function that takes the wrong number of arguments.  For now, we will assume
    // that there are too few and we need to figure out how to pad it out.
    template <typename... FUN_ARGS>
    SignalKey AddAction(RETURN in_fun(FUN_ARGS...)) {
      // Identify the extra arguments by removing the ones that we know about.
      using extra_type = typename TypePack<ARGS...>::template popN<sizeof...(FUN_ARGS)>;
      return AddAction(std::function<RETURN(FUN_ARGS...)>(in_fun), extra_type());
    }

    void Remove(SignalKey key) {
      // Find the action associate with this key.
      emp_assert(emp::Has(link_key_map, key));
      size_t pos = link_key_map[key];

      // Remove the action
      actions.Remove(pos);
      link_key_map.erase(key);

      // Adjust all of the positions of the actions that came after this one.
      for (auto & x : link_key_map) {
        if (x.second > pos) x.second = x.second - 1;
      }
    }

    size_t GetPriority(SignalKey key) {
      emp_assert(emp::Has(link_key_map, key));
      return link_key_map[key];
    }

  };

  template<typename... ARGS>
  inline void SignalBase::BaseTrigger(ARGS... args) {
    // Make sure this base class is really of the correct derrived type (but do so in an
    // assert since triggers may be called frequently and should be fast!)
    emp_assert(dynamic_cast< Signal<void(ARGS...)> * >(this));
    ((Signal<void(ARGS...)> *) this)->Trigger(args...);
  }

  template <typename... ARGS>
  inline SignalKey SignalBase::AddAction(const std::function<void(ARGS...)> & in_fun) {
    // @CAO: Assert for now; ideally try to find solution with fewer args.
    emp_assert(dynamic_cast< Signal<void(ARGS...)> * >(this));
    return ((Signal<void(ARGS...)> *) this)->AddAction(in_fun);
  }

}

#endif
