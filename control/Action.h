//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the Action class, which provides a simple mechanism to abstract
//  functions from their underlying type and provide run-time names.
//
//  Actions can be a bit heavyweight, but can easily be converted to more lightweight
//  std:function objects.

#include <functional>
#include <string>

namespace emp {

  class ActionBase {
  private:
    std::string name;
  public:
    ActionBase(const std::string & in_name) : name(in_name) { ; }
    virtual ~ActionBase() { ; }

    const std::string & GetName() { return name; }
    virtual int GetArgCount() const = 0;
  };

  template <int ARG_COUNT>
  class ActionSize : public ActionBase {
  public:
    ActionSize(const std::string & in_name) : ActionBase(in_name) { ; }
    
    int GetArgCount() const { return ARG_COUNT; }
  };

  template <typename RETURN, typename... ARGS>
  class Action : public ActionSize<sizeof...(ARGS)> {
  private:
    std::function<RETURN(ARGS...)> fun;
    using parent_t = ActionSize<sizeof...(ARGS)>;
  public:
    Action(const std::function<RETURN(ARGS...)> & in_fun, const std::string & in_name)
      : parent_t(in_name), fun(in_fun) { ; }    
    
    RETURN Call(ARGS... args) {
      return fun(std::forward<ARGS>(args)...);
    }
      
    using return_t = RETURN;   // Make return type easily accessible.
  };
  
}
