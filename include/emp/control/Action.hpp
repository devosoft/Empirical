/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  Action.hpp
 *  @brief A mechanism to abstract functions from their underlying type and provide run-time names.
 *  @note Status: Beta
 *
 *  @todo Create an ActionDefaults class that can take fewer args than expected and fill in rest.
 *  @todo Allow for named arguments to facilite intepreted functions.
 */

#ifndef EMP_CONTROL_ACTION
#define EMP_CONTROL_ACTION

#include <functional>
#include <string>

namespace emp {

  /// BaseActions abstract functions and allow for signals to be setup at runtime; they can be
  /// called with types specified in the call.
  ///
  /// Actions can be a bit heavyweight, but can easily be converted to more lightweight
  /// std:function objects.

  class ActionBase {
  protected:
    std::string name;  ///< A unique name for this action so it can be called at runtime.

    ActionBase(const std::string & in_name) : name(in_name) { ; }

    // Protected: ActionBase should not be copied directly, only through derived class.
    ActionBase(const ActionBase &) = default;
    ActionBase(ActionBase &&) = default;
    ActionBase & operator=(const ActionBase &) = default;
    ActionBase & operator=(ActionBase &&) = default;
  public:
    virtual ~ActionBase() { ; }

    /// Get the name of this action.
    const std::string & GetName() const { return name; }

    /// Get number of arguments this action takes.
    virtual size_t GetArgCount() const = 0;

    /// Clone() will produce a pointer to a full copy of an Action, going through derived version.
    virtual ActionBase * Clone() const = 0;
  };

  /// ActionSize is a second layer of abstract actions that know the number of arguments used at compile
  /// time to facilitate easy type-checking.
  template <size_t ARG_COUNT>
  class ActionSize : public ActionBase {
  protected:
    ActionSize(const std::string & in_name) : ActionBase(in_name) { ; }
  public:
    size_t GetArgCount() const { return ARG_COUNT; }
  };

  /// The declaration for Action has any template types; the only definined specilizations require
  /// a function type to be specified (with void and non-void return type variants.)
  template <typename... ARGS> class Action;

  /// This Action class specialization takes a function with a void return tyime and builds it off
  /// of the action base classes.
  template <typename... ARGS>
  class Action<void(ARGS...)> : public ActionSize<sizeof...(ARGS)> {
  protected:
    std::function<void(ARGS...)> fun;  ///< The specific function associated with this action.
  public:
    using this_t = Action<void(ARGS...)>;
    using parent_t = ActionSize<sizeof...(ARGS)>;

    Action(const std::function<void(ARGS...)> & in_fun, const std::string & in_name="")
      : parent_t(in_name), fun(in_fun) { ; }
    template <typename RETURN>
    Action(const std::function<RETURN(ARGS...)> & in_fun, const std::string & in_name="")
      : parent_t(in_name)
      , fun([in_fun](ARGS &&... args){in_fun(std::forward<ARGS>(args)...);}) { ; }
    Action(const this_t &) = default;
    Action(this_t &&) = default;

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;

    const std::function<void(ARGS...)> & GetFun() const { return fun; };

    /// Call the function associated with this action.
    void Call(ARGS &&... args) { return fun(std::forward<ARGS>(args)...); }

    /// Build a copy of this Action.
    this_t * Clone() const { return new this_t(*this); }
  };


  /// This Action class specialization takes a function with any non-void return tyime and builds it
  /// off of the action base classes.
  template <typename RETURN, typename... ARGS>
  class Action<RETURN(ARGS...)> : public ActionSize<sizeof...(ARGS)> {
  protected:
    std::function<RETURN(ARGS...)> fun;  ///< The specific function associated with this action.
  public:
    using fun_t = RETURN(ARGS...);
    using this_t = Action<fun_t>;
    using parent_t = ActionSize<sizeof...(ARGS)>;

    Action(const std::function<RETURN(ARGS...)> & in_fun, const std::string & in_name="")
      : parent_t(in_name), fun(in_fun) { ; }
    Action(const this_t &) = default;
    Action(this_t &&) = default;

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;

    const std::function<fun_t> & GetFun() const { return fun; };

    /// Call the function associated with this action.
    RETURN Call(ARGS &&... args) { return fun(std::forward<ARGS>(args)...); }

    /// Build a copy of this Action.
    this_t * Clone() const { return new this_t(*this); }
  };

  /// Build an action object using this function.
  template <typename RETURN, typename... ARGS>
  auto make_action(const std::function<RETURN(ARGS...)> & in_fun, const std::string & name="") {
    return Action<RETURN(ARGS...)>(in_fun, name);
  }
}

#endif
